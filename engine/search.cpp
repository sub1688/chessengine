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

#include <cstring>

void Search::orderMoves(ArrayVec<Move, 218> &moveVector, int rootDepth, ThreadWorkerInfo *threadWorkerInfoPtr, Move ttMove, int depth) {
    auto getMoveScore = [&](const Move &move) -> int {
        int score = 0;

        if (move == ttMove) {
            score += TRANSPOSITION_TABLE_BIAS; // Give TT move a big boost
        }

        if (threadWorkerInfoPtr != nullptr) {
            if (move == threadWorkerInfoPtr->killerMoves[depth][0] || move == threadWorkerInfoPtr->killerMoves[depth][1]) {
                score += KILLER_MOVE_BIAS;
            }
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

    if (rootDepth == 0) {
        if (threadWorkerInfoPtr->threadNumber == 0 || moveVector.elements <= 1) {
            // Main thread uses the standard ordering
            return;
        }

        // For helper threads, rotate the top N moves based on thread ID
        int rotationCount = threadWorkerInfoPtr->threadNumber % std::min(4, static_cast<int>(moveVector.elements));
        if (rotationCount > 0) {
            // Only rotate among the top few moves (the most promising ones)
            int topMovesToConsider = std::min(static_cast<int>(moveVector.elements), 4 + threadWorkerInfoPtr->threadNumber % 3);

            // Shift the top moves by the rotation count
            std::rotate(
                moveVector.buffer.begin(),
                moveVector.buffer.begin() + rotationCount % topMovesToConsider,
                moveVector.buffer.begin() + topMovesToConsider
            );
        }
    }
}

void Search::threadSearch(ThreadWorkerInfo *info) {
    for (info->depthToSearch = 1; info->depthToSearch < 256; info->depthToSearch++) {
        SearchResult result = search(info->board, info, info->depthToSearch);

        if (searchCancelled)
            break;

        if (info->threadNumber == 0) {
            currentEval = result.evaluation;
            currentDepth = info->depthToSearch;
            bestMove = result.bestMove;
            times[currentDepth] = getMillisSinceEpoch() - currentTimeMillis;
            timesFloat[currentDepth] = static_cast<float>(times[currentDepth]);
            depths[currentDepth] = static_cast<float>(currentDepth);
            evaluations[info->board.moveNumber] = static_cast<float>(currentEval) / 100.F * static_cast<float>(lastSearchTurnIsWhite ? 1 : -1);

            std::cout << currentDepth << ":" << std::to_string(currentEval) << ":" << std::to_string(bestMove.from) <<
                    "," << std::to_string(bestMove.to) << ":" << StandardAlgebraicNotation::boardToSan(
                        info->board, bestMove) <<
                    std::endl;
        }
    }
}


void Search::startIterativeSearch(Board &board, long time) {
    std::memset(&times, 0, sizeof(times));
    std::memset(&timesFloat, 0, sizeof(timesFloat));
    std::memset(&depths, 0, sizeof(depths));

    nullPrunes = 0;
    searchCancelled = false;
    searchDuration = time;
    nodesCounted = 0;
    transpositionTable.cutoffs = 0;
    lastSearchTurnIsWhite = board.whiteToMove;
    currentTimeMillis = getMillisSinceEpoch();

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

    std::vector<std::thread> threads;
    threads.reserve(MAX_THREADS);

    std::vector<std::unique_ptr<ThreadWorkerInfo>> threadWorkerInfos;
    threadWorkerInfos.reserve(MAX_THREADS);

    for (int threadNumber = 0; threadNumber < MAX_THREADS; threadNumber++) {
        threadWorkerInfos.emplace_back(std::make_unique<ThreadWorkerInfo>(threadNumber, currentDepth));

        ThreadWorkerInfo *infoPtr = threadWorkerInfos[threadNumber].get();
        memcpy(&infoPtr->board, &board, sizeof(Board));
        threads.emplace_back(threadSearch, infoPtr);
    }

    for (auto &t: threads) {
        if (t.joinable()) t.join();
    }
}

SearchResult Search::search(Board &board, ThreadWorkerInfo *threadWorkerInfoPtr, int rootDepth, int depth, int alpha,
                            int beta, bool wasNullSearch, bool inPrincipalVariation) {
    nodesCounted++;

    // Checkup to see search duration is over
    if ((nodesCounted & 2047) == 0 && getMillisSinceEpoch() - currentTimeMillis >= searchDuration)
        searchCancelled = true;

    if (searchCancelled)
        return {0, NULL_MOVE};

    if (board.isDrawn())
        return {0, NULL_MOVE};

    if (rootDepth > 0) {
        alpha = std::max(alpha, NEGATIVE_INFINITY + rootDepth);
        beta = std::min(beta, POSITIVE_INFINITY - rootDepth - 1);
        if (alpha >= beta)
            return {alpha, NULL_MOVE};
    }

    if (depth <= 0)
        return {quiesce(board, alpha, beta), NULL_MOVE};

    TranspositionEntry entry;
    Move lookupBestMove;
    if (transpositionTable.tableLookup(board.currentZobristKey, entry)) {
        uint64_t moveBits = EXTRACT_BEST_MOVE_BITS(entry.data);
        GET_MOVE_FROM_BITS(moveBits, lookupBestMove);

        int depthSearched = EXTRACT_DEPTH_SEARCHED(entry.data);
        int nodeType = EXTRACT_NODE_TYPE(entry.data);
        int score = EXTRACT_SCORE(entry.data);
        if (depthSearched >= depth && !searchCancelled && !inPrincipalVariation) {
            int correctedScore = transpositionTable.correctScoreForRetrieval(score, rootDepth);
            if (nodeType == EXACT_BOUND) {
                transpositionTable.cutoffs++;
                return {correctedScore, lookupBestMove};
            }
            if (nodeType == UPPER_BOUND && score <= alpha) {
                transpositionTable.cutoffs++;
                return {alpha, lookupBestMove};
            }
            if (nodeType == LOWER_BOUND && score >= beta) {
                transpositionTable.cutoffs++;
                return {beta, lookupBestMove};
            }
        }
    }

    // Null Move Pruning
    if (!inPrincipalVariation && rootDepth && depth >= 3 && !wasNullSearch && canNullMove(board) && !Movegen::isKingInDanger(board, board.whiteToMove)) {
        int reduction = 2 + depth / 4;

        board.nullMove();
        SearchResult result = search(board, threadWorkerInfoPtr, rootDepth + 1, depth - 1 - reduction, -beta, -beta + 1,
                                     true, inPrincipalVariation);
        int negatedScore = -result.evaluation;
        board.undoNullMove();

        if (searchCancelled)
            return {0, NULL_MOVE};

        if (negatedScore >= beta && abs(negatedScore) < MATE_THRESHOLD) {
            nullPrunes++;
            return {negatedScore, NULL_MOVE};
        }
    }

    int nodeType = UPPER_BOUND;
    bool movesAvailable = false;
    Move bestMove = lookupBestMove;

    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard(board);

    orderMoves(moves, rootDepth, threadWorkerInfoPtr, bestMove, depth); // Order moves for better pruning
    bool firstMove = true;
    int moved = 0;
    for (int i = 0; i < moves.elements; i++) {
        if (searchCancelled)
            return {0, NULL_MOVE};
        Move move = moves.buffer[i];

        if (!board.move(move))
            continue;

        movesAvailable = true;
        int negatedScore = negatedPrincipalVariationSearch(board, threadWorkerInfoPtr, move, firstMove, moved,
                                                           rootDepth, depth, alpha, beta,
                                                           wasNullSearch, !rootDepth && firstMove);
        board.undoMove(move);
        moved++;

        // Alpha-beta update
        if (negatedScore > alpha) {
            alpha = negatedScore;
            bestMove = move;
            nodeType = EXACT_BOUND;
        }

        if (alpha >= beta) {
            storeKillerMove(threadWorkerInfoPtr, move, depth);
            transpositionTable.addEntry(board.currentZobristKey, bestMove, rootDepth, depth, beta, LOWER_BOUND);
            return {beta, bestMove};
        }
    }

    if (!movesAvailable) {
        alpha = Movegen::isKingInDanger(board, board.whiteToMove) ? NEGATIVE_INFINITY + rootDepth : 0;
    }

    if (!searchCancelled) {
        transpositionTable.addEntry(board.currentZobristKey, bestMove, rootDepth, depth, alpha, nodeType);
    }
    return {alpha, bestMove};
}

int Search::negatedPrincipalVariationSearch(Board &board, ThreadWorkerInfo *threadWorkerInfoPtr, Move move,
                                            bool &firstMove, int moved, int rootDepth,
                                            int depth, int alpha, int beta, bool wasNullSearch,
                                            bool inPrincipalVariation) {
    int negatedScore;

    if (depth > 3 && move.capture == NONE && move.promotion == NONE && moved > 4) {
        negatedScore = -search(board, threadWorkerInfoPtr, rootDepth + 1, depth - 2, -alpha - 1, -alpha, wasNullSearch,
                               inPrincipalVariation).evaluation;
        if (negatedScore <= alpha) {
            return negatedScore;
        }
    }

    if (firstMove) {
        // Full window search for the first move
        negatedScore = -search(board, threadWorkerInfoPtr, rootDepth + 1, depth - 1, -beta, -alpha, wasNullSearch,
                               inPrincipalVariation).evaluation;
        firstMove = false;
    } else {
        // Principal Variation Search: try a null window search first
        negatedScore = -search(board, threadWorkerInfoPtr, rootDepth + 1, depth - 1, -alpha - 1, -alpha, wasNullSearch,
                               inPrincipalVariation).evaluation;

        // If null window search fails high, do a full research
        if (negatedScore > alpha && negatedScore < beta) {
            negatedScore = -search(board, threadWorkerInfoPtr, rootDepth + 1, depth - 1, -beta, -alpha, wasNullSearch,
                                   true).evaluation;
        }
    }
    return negatedScore;
}

bool Search::canNullMove(Board &board) {
    return __builtin_popcountll(board.majorPieceBitboards(board.whiteToMove));
}

SearchResult Search::search(Board &board, ThreadWorkerInfo *threadWorkerInfoPtr, int depth) {
    return search(board, threadWorkerInfoPtr, 0, depth, NEGATIVE_INFINITY, POSITIVE_INFINITY, false, true);
}

int Search::quiesce(Board &board, int alpha, int beta) {
    int standingPat = evaluate(board);
    if (standingPat >= beta)
        return beta;
    if (alpha < standingPat)
        alpha = standingPat;


    ArrayVec<Move, 218> captures = Movegen::generateAllLegalMovesOnBoard(board, true);
    orderMoves(captures, 1, nullptr, NULL_MOVE, 0);

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

        totalValue += evaluateKingDistance(whiteKing, blackKing, WHITE_KING, materialDelta);
        totalValue -= evaluateKingDistance(blackKing, whiteKing, BLACK_KING, materialDelta);
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

int Search::evaluateKingDistance(uint8_t squareIndex, uint8_t otherKingIndex, uint8_t piece,
                                 int materialDelta) {
    if (piece == WHITE_KING && materialDelta < 0)
        return 0;
    if (piece == BLACK_KING && materialDelta > 0)
        return 0;

    int file1 = squareIndex % 8, rank1 = squareIndex / 8;
    int file2 = otherKingIndex % 8, rank2 = otherKingIndex / 8;

    return (std::abs(file1 - file2) + std::abs(rank1 - rank2)) * -50;
}

void Search::storeKillerMove(ThreadWorkerInfo *threadWorkerInfoPtr, Move move, int depth) {
    if (move.capture != NONE || move.promotion != NONE)
        return;

    if (threadWorkerInfoPtr->killerMoves[depth][0] == move) return;

    /*
     * the previous killer move is shifted is so that the first killer move in the array is always the most recent one, which is likely to be better in more positions
     */
    threadWorkerInfoPtr->killerMoves[depth][1] = threadWorkerInfoPtr->killerMoves[depth][0];
    threadWorkerInfoPtr->killerMoves[depth][0] = move;
}
