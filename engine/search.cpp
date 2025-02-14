#include "search.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <thread>

#include "movegen.h"
#include "piecesquaretable.h"
#include "transpositiontable.h"

void Search::orderMoves(ArrayVec<Move, 218>& moveVector, Move ttMove) {
    auto getMoveScore = [&](const Move& move) -> int {
        int score = 0;

        if (move.from == ttMove.from && move.to == ttMove.to) {
            score += TRANSPOSITION_TABLE_BIAS; // Give TT move a big boost
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
              [&](const Move& a, const Move& b) {
                  return getMoveScore(a) > getMoveScore(b);
              });
}


void Search::startIterativeSearch(long time) {
    TranspositionTable::cutoffs = 0;
    searchCancelled = false;
    lastSearchTurnIsWhite = Board::whiteToMove;
    std::thread timerThread([time]() {
        long currentTime = 0;
        while (!searchCancelled && (currentTime += 10) <= time) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        searchCancelled = true;
    });

    for (currentDepth = 0; currentDepth < 256; currentDepth++) {
        SearchResult result = search(currentDepth);

        if (!searchCancelled) {
            currentEval = result.evaluation;
            bestMove = result.bestMove;
            if (abs(result.evaluation) >= MATE_THRESHOLD)
                searchCancelled = true;
        }
        else {
            currentDepth--;
            break;
        }
    }

    if (timerThread.joinable())
        timerThread.join();
}

SearchResult Search::search(int rootDepth, int depth, int alpha, int beta) {
    if (searchCancelled)
        return {0, NULL_MOVE};
    if (depth == 0)
        return {quiesce(alpha, beta), NULL_MOVE};

    TranspositionEntry entry = TranspositionTable::getEntry(Board::currentZobristKey);
    int nodeType = UPPER_BOUND;
    if (entry.zobristKey == Board::currentZobristKey && entry.depthSearched >= depth && rootDepth != 0) {
        if (abs(entry.score) > MATE_THRESHOLD) {
            if (entry.score < 0) {
                entry.score += rootDepth * 100;
            }else {
                entry.score -= rootDepth * 100;
            }
        }
        if (entry.nodeType == EXACT_BOUND) {
            TranspositionTable::cutoffs++;
            return {entry.score, entry.bestMove};
        }
        if (entry.nodeType == UPPER_BOUND && entry.score < alpha) {
            TranspositionTable::cutoffs++;
            return {entry.score, entry.bestMove};
        }
        if (entry.nodeType == LOWER_BOUND && entry.score >= beta) {
            TranspositionTable::cutoffs++;
            return {entry.score, entry.bestMove};
        }
    }

    bool movesAvailable = false;
    int maximumScore = NEGATIVE_INFINITY;
    Move bestMove = NULL_MOVE;

    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();
    orderMoves(moves, entry.bestMove);
    for (int i = 0; i < moves.elements; i++) {
        if (searchCancelled)
            return {0, NULL_MOVE};
        Move move = moves.buffer[i];

        if (Board::move(move)) {
            movesAvailable = true;
            SearchResult result = search(rootDepth + 1, depth - 1, -beta, -alpha);
            Board::undoMove(move);

            int negatedScore = -result.evaluation;

            alpha = std::max(alpha, negatedScore);
            if (negatedScore > maximumScore) {
                maximumScore = negatedScore;
                bestMove = move;
                nodeType = EXACT_BOUND;
            }

            if (beta <= alpha) {
                nodeType = LOWER_BOUND;
                break;
            }
        }
    }

    if (Board::isDrawnByRepetition() && beta > alpha)
        maximumScore = 0;

    if (!movesAvailable && beta > alpha) {
        maximumScore = Movegen::isKingInDanger(Board::whiteToMove) ? -NEGATIVE_INFINITY + rootDepth * 100 : 0;
    } // This returns NEGATIVE INFINITY for Checkmate (because what could be worse) and 0 for stalemate (which is a draw)

    if (!searchCancelled)
        TranspositionTable::addEntry(Board::currentZobristKey, bestMove, depth, maximumScore, nodeType);
    return {maximumScore, bestMove};
}

SearchResult Search::search(int depth) {
    return search(0, depth, NEGATIVE_INFINITY, POSITIVE_INFINITY);
}

int Search::quiesce(int alpha, int beta) {
    int standingPat = evaluate();
    if (standingPat >= beta)
        return beta;
    if (alpha < standingPat)
        alpha = standingPat;

    TranspositionEntry entry = TranspositionTable::getEntry(Board::currentZobristKey);

    ArrayVec<Move, 218> captures = Movegen::generateAllCapturesOnBoard();
    orderMoves(captures, entry.zobristKey == Board::currentZobristKey ? entry.bestMove : NULL_MOVE);

    for (int i = 0; i < captures.elements; i++) {
        Move move = captures.buffer.at(i);

        if (Board::move(move)) {
            int score = -quiesce(-beta, -alpha);
            Board::undoMove(move);
            if (score >= beta)
                return beta;
            if (score > alpha)
                alpha = score;
        }
    }
    return alpha;
}

int Search::evaluate() {
    if (Board::isDrawnByRepetition())
        return 0;
    int totalValue = 0;
    bool endgame = isInEndgame();
    uint64_t bitboard = Board::BITBOARD_OCCUPANCY;
    while (bitboard) {
        uint8_t index = Movegen::popLeastSignificantBitAndGetIndex(bitboard);
        uint8_t piece = Board::getPiece(index);
        totalValue += getPieceValue(piece) + (endgame
                                                  ? PieceSquareTable::PIECE_SQUARE_TABLE_ENDGAME[piece][index]
                                                  : PieceSquareTable::PIECE_SQUARE_TABLE[piece][index]) * (
            piece > 5 ? -1 : 1);
    }
    return totalValue * (Board::whiteToMove ? 1 : -1);
}

bool Search::isInEndgame() {
    return __builtin_popcountll(Board::BITBOARD_OCCUPANCY) <= 14;
}

int Search::getPieceValue(uint8_t piece) {
    return PIECE_VALUES[piece];
}

bool Search::isNullMove(Move move) {
    return move.from == move.to;
}
