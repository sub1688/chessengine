#include "search.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <thread>

#include "movegen.h"
#include "piecesquaretable.h"

void Search::orderMoves(ArrayVec<Move, 218>& moveVector) {
    // Define a lambda function to calculate the score of a move
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

void Search::startIterativeSearch(long time, Move& lastMove) {
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
    int evalThisIteration = 0;

    for (int i = 1; i <= 256; i++) {
        currentEval = search(i, bestMoveThisIteration);
        currentDepth = i;
        if (!searchCancelled) {
            bestMoveThisIteration = bestMove;
            lastMove = bestMoveThisIteration;
            evalThisIteration = currentEval;
        }else {
            break;
        }
    }

    if (timerThread.joinable()) {
        timerThread.join();
    }

    currentEval = evalThisIteration;
    bestMove = bestMoveThisIteration;
}

int Search::search(int depth, Move iterativeStart) {
    return search(depth, depth, NEGATIVE_INFINITY, POSITIVE_INFINITY, iterativeStart);
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

int Search::search(int rootDepth, int depth, int alpha, int beta, Move iterativeStart) {
    if (depth == 0) {
        return quiesce(alpha, beta);
    }

    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();
    orderMoves(moves);
    bool noLegalMoves = true;
    bool consideredIterativeStart = false;

    for (int i = 0; i < moves.elements; i++) {
        if (searchCancelled) {
            return 0;
        }

        Move move = iterativeStart;

        if (!consideredIterativeStart && iterativeStart.from != iterativeStart.to && rootDepth == depth) {
            consideredIterativeStart = true;
            i--;
        }else {
            move = moves.buffer[i];
        }

        if (Board::move(move)) {
            noLegalMoves = false;
            int score = -search(rootDepth, depth - 1, -beta, -alpha, iterativeStart);
            Board::undoMove(move);

            if (score > alpha) {
                alpha = score;
                if (depth == rootDepth) {
                    currentEval = alpha;
                    bestMove = move;
                }
            }

            if (score >= beta)
                break;
        }
    }

    // Check for checkmate or stalemate
    if (noLegalMoves) {
        if (Movegen::isKingInDanger(Board::whiteToMove)) {
            return NEGATIVE_INFINITY + (rootDepth - depth); // Checkmate
        }
        return 0; // Stalemate
    }

    return alpha;
}


int Search::evaluate() {
    int totalValue = 0;
    bool endgame = isInEndgame();
    for (int i = 0; i < 12; i++) {
        uint64_t bitboard = Board::BITBOARDS[i];
        int pieceValue = getPieceValue(i);
        while (bitboard) {
            int index = Movegen::popLeastSignificantBitAndGetIndex(bitboard);
            if (i >= 6) {
                totalValue -= pieceValue;
                totalValue -= endgame ? PieceSquareTable::PIECE_SQUARE_TABLE_ENDGAME[i][index] : PieceSquareTable::PIECE_SQUARE_TABLE[i][index];
            }else {
                totalValue += pieceValue;
                totalValue += endgame ? PieceSquareTable::PIECE_SQUARE_TABLE_ENDGAME[i][index] : PieceSquareTable::PIECE_SQUARE_TABLE[i][index];
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


