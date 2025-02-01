#include "search.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <thread>

#include "movegen.h"
#include "piecesquaretable.h"
#include "transpositiontable.h"
#include "zobrist.h"

void Search::orderMoves(ArrayVec<Move, 218> &moveVector) {
    auto getMoveScore = [](const Move &move) -> int {
        int score = 0;
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

void Search::startIterativeSearch(long time, Move &lastMove) {
    TranspositionTable::clear();
    searchCancelled = false;
    lastSearchTurnIsWhite = Board::whiteToMove;
    std::thread timerThread([time]() {
        long currentTime = 0;
        while (!searchCancelled && (currentTime += 10) <= time) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        searchCancelled = true;
    });

    Move bestMoveThisIteration = Move(0, 0);

    int prevEval = 0;
    for (int i = 1; i <= 256; i++) {
        currentEval = search(i);
        currentDepth = i;
        if (!searchCancelled) {
            bestMoveThisIteration = bestMove;
            lastMove = bestMoveThisIteration;
            prevEval = currentEval;
        } else {
            currentEval = prevEval;
            currentDepth--;
            break;
        }
    }

    if (timerThread.joinable()) {
        timerThread.join();
    }

    bestMove = bestMoveThisIteration;
}

int Search::search(int depth) {
    return search(depth, depth, NEGATIVE_INFINITY, POSITIVE_INFINITY);
}

int Search::quiesce(int alpha, int beta) {
    int standingPat = evaluate();
    if (standingPat >= beta)
        return beta;
    if (alpha < standingPat)
        alpha = standingPat;

    ArrayVec<Move, 218> captures = Movegen::generateAllCapturesOnBoard();
    orderMoves(captures);

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

int Search::search(int rootDepth, int depth, int alpha, int beta) {
    if (depth == 0) {
        return quiesce(alpha, beta);
    }

    uint64_t zobristKey = Board::currentZobristKey;
    TranspositionEntry &entry = TranspositionTable::getEntry(zobristKey);

    // Step 1: Look up transposition table
    if (entry.zobristKey == zobristKey && entry.depthSearched >= depth) {
        if (entry.nodeType == EXACT_BOUND) return entry.score;
        if (entry.nodeType == LOWER_BOUND && entry.score >= beta) return beta;
        if (entry.nodeType == UPPER_BOUND && entry.score <= alpha) return alpha;
    }

    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();
    orderMoves(moves);

    bool noLegalMoves = true;
    Move bestMoveLocal = Move(); // Track the best move found in this search
    int originalAlpha = alpha;   // Store original alpha

    for (int i = 0; i < moves.elements; i++) {
        if (searchCancelled) {
            return 0; // Exit early, keeping previous bestMove
        }

        Move move = moves.buffer[i];

        if (Board::move(move)) {
            noLegalMoves = false;
            int score = -search(rootDepth, depth - 1, -beta, -alpha);
            Board::undoMove(move);

            if (score > alpha) {
                alpha = score;
                bestMoveLocal = move;
                if (depth == rootDepth) bestMove = move; // Update bestMove only if not canceled
            }

            if (score >= beta) {
                TranspositionTable::addEntry(zobristKey, move, depth, beta, LOWER_BOUND);
                return beta;
            }
        }
    }

    // Check for checkmate or stalemate
    if (noLegalMoves) {
        if (Movegen::isKingInDanger(Board::whiteToMove)) {
            alpha = NEGATIVE_INFINITY + (rootDepth - depth) * 100; // Checkmate
        } else {
            alpha = 0; // Stalemate
        }
    }

    // If no move was better than alpha, don't overwrite bestMove
    if (alpha > originalAlpha) {
        int nodeType = (alpha <= entry.score) ? UPPER_BOUND : (alpha >= beta ? LOWER_BOUND : EXACT_BOUND);
        TranspositionTable::addEntry(zobristKey, bestMoveLocal, depth, alpha, nodeType);
    }

    return alpha;
}

int Search::evaluate() {
    if (TranspositionTable::getRepetitionEntry(Board::currentZobristKey) >= 3)
        return 0;

    int totalValue = 0;
    bool endgame = isInEndgame();
    for (int i = 0; i < 12; i++) {
        uint64_t bitboard = Board::BITBOARDS[i];
        int pieceValue = getPieceValue(i);
        while (bitboard) {
            int index = Movegen::popLeastSignificantBitAndGetIndex(bitboard);
            if (i >= 6) {
                totalValue -= pieceValue;
                totalValue -= endgame
                                  ? PieceSquareTable::PIECE_SQUARE_TABLE_ENDGAME[i][index]
                                  : PieceSquareTable::PIECE_SQUARE_TABLE[i][index];
            } else {
                totalValue += pieceValue;
                totalValue += endgame
                                  ? PieceSquareTable::PIECE_SQUARE_TABLE_ENDGAME[i][index]
                                  : PieceSquareTable::PIECE_SQUARE_TABLE[i][index];
            }
        }
    }
    totalValue += Board::canWhiteCastleKingside(Board::moveNumber) * 25;
    totalValue += Board::canWhiteCastleQueenside(Board::moveNumber) * 25;
    totalValue -= Board::canBlackCastleKingside(Board::moveNumber) * 25;
    totalValue -= Board::canBlackCastleQueenside(Board::moveNumber) * 25;
    return totalValue * (Board::whiteToMove ? 1 : -1);
}

bool Search::isInEndgame() {
    return __builtin_popcountll(Board::BITBOARD_OCCUPANCY) <= 14;
}

int Search::getPieceValue(uint8_t piece) {
    return PIECE_VALUES[piece % 6];
}


