#include "search.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <thread>
#include <valarray>

#include "movegen.h"
#include "openingbook.h"
#include "piecesquaretable.h"
#include "san.h"
#include "transpositiontable.h"
#include "zobrist.h"
#include "../ui/window.h"

#include <cstring>

void Search::orderMoves(ArrayVec<Move, 218> &moveVector, Move ttMove, int depth) {
    auto getMoveScore = [&](const Move &move) -> int {
        int score = 0;

        if (move == ttMove) {
            score += TRANSPOSITION_TABLE_BIAS; // Give TT move a big boost
        }

        if (move == transpositionTable.killerMoves[depth][0] || move == transpositionTable.killerMoves[depth][1]) {
            score += KILLER_MOVE_BIAS;
        }

        if (move.capture != NONE) {
            int materialDelta = getPieceValue(move.capture) - getPieceValue(move.pieceFrom);
            score += (materialDelta >= 0 ? WINNING_CAPTURE_BIAS : LOSING_CAPTURE_BIAS) + materialDelta;
        }

        if (move.promotion != -1) {
            score += PROMOTE_BIAS;
        }

        return score;
    };

    std::sort(moveVector.buffer.begin(), moveVector.buffer.begin() + moveVector.elements,
              [&](const Move &a, const Move &b) {
                  return getMoveScore(a) > getMoveScore(b);
              });
}


void Search::startIterativeSearch(Board &board, long time) {
    std::memset(&times, 0, sizeof(times));

    transpositionTable.cutoffs = 0;
    transpositionTable.collisions = 0;
    searchCancelled = false;
    lastSearchTurnIsWhite = board.whiteToMove;

    long currentTimeMillis = getMillisSinceEpoch();
    // Lookup in opening book
    Move move = OpeningBook::fetchNextBookMove(board);
    if (!isNullMove(move)) {
        bestMove = move;
        currentEval = 0;
        std::cout << "1:" << std::to_string(currentEval) << ":" << std::to_string(bestMove.from) << "," <<
                std::to_string(bestMove.to) << ":" << StandardAlgebraicNotation::boardToSan(board, bestMove) <<
                std::endl;
        return;
    }

    std::thread timerThread([time]() {
        long currentTime = 0;
        while (!searchCancelled && (currentTime += 100) <= time) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        searchCancelled = true;
        std::cout << "Cancelled" << std::endl;
    });

    for (currentDepth = 2; currentDepth < 256; currentDepth++) {
        SearchResult result = search(board, currentDepth);

        if (!searchCancelled) {
            times[currentDepth] = getMillisSinceEpoch() - currentTimeMillis;
            currentEval = result.evaluation;
            bestMove = result.bestMove;
            if (abs(currentEval) > MATE_THRESHOLD) {
                searchCancelled = true;
            }
            std::cout << currentDepth << ":" << std::to_string(currentEval) << ":" << std::to_string(bestMove.from) <<
                    "," << std::to_string(bestMove.to) << ":" << StandardAlgebraicNotation::boardToSan(board, bestMove)
                    <<
                    std::endl;
        } else {
            currentDepth--;
            break;
        }
    }

    if (timerThread.joinable())
        timerThread.join();
}

SearchResult Search::search(Board &board, int rootDepth, int depth, int alpha, int beta, bool wasNullSearch) {
    if (searchCancelled)
        return {0, NULL_MOVE};
    if (depth == 0)
        return {quiesce(board, alpha, beta), NULL_MOVE};

    if (rootDepth > 1) {
        if (board.isDrawn())
            return {0, NULL_MOVE};
        alpha = std::max(alpha, NEGATIVE_INFINITY + rootDepth);
        beta = std::min(beta, POSITIVE_INFINITY - rootDepth);
        if (alpha >= beta)
            return {alpha, NULL_MOVE};
    }

    bool inPrincipalVariation = beta > alpha + 1;

    TranspositionEntry entry = transpositionTable.getEntry(board.currentZobristKey);
    if (entry.zobristKey == board.currentZobristKey && entry.depthSearched >= depth && !searchCancelled && !inPrincipalVariation) {
        int correctedScore = transpositionTable.correctScoreForRetrieval(entry.score, rootDepth);
        if (entry.nodeType == EXACT_BOUND) {
            transpositionTable.cutoffs++;
            return {correctedScore, entry.bestMove};
        }
        if (entry.nodeType == UPPER_BOUND && entry.score <= alpha) {
            transpositionTable.cutoffs++;
            return {correctedScore, entry.bestMove};
        }
        if (entry.nodeType == LOWER_BOUND && entry.score >= beta) {
            transpositionTable.cutoffs++;
            return {correctedScore, entry.bestMove};
        }
    }

    int nodeType = EXACT_BOUND;
    bool movesAvailable = false;
    int maximumScore = NEGATIVE_INFINITY;
    Move bestMove = NULL_MOVE;

    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard(board);
    orderMoves(moves, entry.bestMove, depth); // Order moves for better pruning

    bool firstMove = true;
    int moved = 0;
    for (int i = 0; i < moves.elements; i++) {
        if (searchCancelled)
            return {0, NULL_MOVE};
        Move move = moves.buffer[i];

        if (!board.move(move))
            continue;

        movesAvailable = true;
        int negatedScore = negatedPrincipalVariationSearch(board, move, firstMove, moved, rootDepth, depth, alpha, beta, false);
        board.undoMove(move);
        moved++;

        // Alpha-beta update
        if (negatedScore > maximumScore) {
            maximumScore = negatedScore;
            bestMove = move;
            nodeType = UPPER_BOUND;
        }

        alpha = std::max(alpha, negatedScore);
        if (beta <= alpha) {
            transpositionTable.storeKillerMove(move, depth);

            nodeType = LOWER_BOUND; // Beta cutoff
            break;
        }
    }

    if (!movesAvailable) {
        maximumScore = Movegen::isKingInDanger(board, board.whiteToMove) ? NEGATIVE_INFINITY + rootDepth : 0;
    }

    if (!searchCancelled && !isNullMove(bestMove)) {
        transpositionTable.addEntry(board.currentZobristKey, bestMove, rootDepth, depth, maximumScore, nodeType);
    }
    return {maximumScore, bestMove};
}

int Search::negatedPrincipalVariationSearch(Board &board, Move move, bool &firstMove, int moved, int rootDepth, int depth, int alpha, int beta, bool wasNullSearch) {
    int negatedScore;

    if (depth > 3 && move.capture == NONE && move.promotion == NONE && moved > 4)
    {
        negatedScore = -search(board, rootDepth + 1, depth - 2, -alpha - 1, -alpha, wasNullSearch).evaluation;
        if (negatedScore <= alpha)
        {
            return negatedScore;
        }
    }

    if (firstMove) {
        // Full window search for the first move
        negatedScore = -search(board, rootDepth + 1, depth - 1, -beta, -alpha, wasNullSearch).evaluation;
        firstMove = false;
    } else {
        // Late move reductions!!!
        // Principal Variation Search: try a null window search first
        negatedScore = -search(board, rootDepth + 1, depth - 1, -alpha - 1, -alpha, wasNullSearch).evaluation;

        // If null window search fails high, do a full re-search
        if (negatedScore > alpha && negatedScore < beta) {
            negatedScore = -search(board, rootDepth + 1, depth - 1, -beta, -alpha, wasNullSearch).evaluation;
        }
    }
    return negatedScore;
}

bool Search::canNullMove(Board &board) {
    return __builtin_popcountll(board.majorPieceBitboards(board.whiteToMove)) + __builtin_popcountll(board.minorPieceBitboards(board.whiteToMove)) > 0;
}

SearchResult Search::search(Board &board, int depth) {
    return search(board, 0, depth, NEGATIVE_INFINITY, POSITIVE_INFINITY, false);
}

int Search::quiesce(Board &board, int alpha, int beta) {
    int standingPat = evaluate(board);
    if (standingPat >= beta)
        return beta;
    if (alpha < standingPat)
        alpha = standingPat;


    ArrayVec<Move, 218> captures = Movegen::generateAllLegalMovesOnBoard(board, true);
    orderMoves(captures, NULL_MOVE, 0);

    for (int i = 0; i < captures.elements; i++) {
        Move move = captures.buffer.at(i);

        if (board.move(move)) {
            int score = -quiesce(board, -beta, -alpha);
            board.undoMove(move);
            if (score >= beta)
                return beta;
            if (score > alpha)
                alpha = score;
        }
    }
    return alpha;
}

int Search::evaluate(Board &board) {
    if (board.isDrawn())
        return 0;
    int totalValue = 0;

    double endgameBias = getEndGameBias(board);

    uint64_t bitboard = board.BITBOARD_OCCUPANCY;
    int materialDelta = 0;
    while (bitboard) {
        uint8_t index = Movegen::popLeastSignificantBitAndGetIndex(bitboard);
        uint8_t piece = board.getPiece(index);
        materialDelta += getPieceValue(piece);
        totalValue += static_cast<int>(
                    PieceSquareTable::PIECE_SQUARE_TABLE[piece][index] * endgameBias +
                    PieceSquareTable::PIECE_SQUARE_TABLE_ENDGAME[piece][index] * (1 - endgameBias)) *
                (
                    piece > 5 ? -1 : 1);
        totalValue += evaluatePassedPawn(board, index, piece);
    }

    totalValue += materialDelta;

    if (endgameBias < 0.18) {
        uint64_t whiteKing = std::countr_zero(board.BITBOARDS[WHITE_KING]);
        uint64_t blackKing = std::countr_zero(board.BITBOARDS[BLACK_KING]);

        totalValue += evaluateKingDistance(board, whiteKing, blackKing, WHITE_KING, materialDelta);
        totalValue -= evaluateKingDistance(board, blackKing, whiteKing, BLACK_KING, materialDelta);
    }

    return totalValue * (board.whiteToMove ? 1 : -1);
}

double Search::getEndGameBias(Board &board) {
    return static_cast<double>(__builtin_popcountll(board.BITBOARD_OCCUPANCY)) / 32;
}

int Search::getPieceValue(uint8_t piece) {
    return PIECE_VALUES[piece];
}

bool Search::isNullMove(Move move) {
    return move.from == move.to;
}

int Search::evaluatePassedPawn(Board &board, uint8_t squareIndex, uint8_t piece) {
    if (piece == WHITE_PAWN) {
        if (WHITE_PASSED_PAWN_MASKS[squareIndex] & board.BITBOARDS[BLACK_PAWN] == 0) {
            int rank = squareIndex / 8;
            return 40 * rank;
        }
    }
    if (piece == BLACK_PAWN) {
        if (BLACK_PASSED_PAWN_MASKS[squareIndex] & board.BITBOARDS[WHITE_PAWN] == 0) {
            int rank = squareIndex / 8;
            return -40 * (8 - rank);
        }
    }
    return 0;
}

int Search::evaluateKingDistance(Board &board, uint8_t squareIndex, uint8_t otherKingIndex, uint8_t piece,
                                 int materialDelta) {
    if (piece == WHITE_KING && materialDelta < 0)
        return 0;
    if (piece == BLACK_KING && materialDelta > 0)
        return 0;

    int file1 = squareIndex % 8, rank1 = squareIndex / 8;
    int file2 = otherKingIndex % 8, rank2 = otherKingIndex / 8;

    return (std::abs(file1 - file2) + std::abs(rank1 - rank2)) * -20;
}
