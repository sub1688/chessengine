#include "search.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <thread>

#include "movegen.h"
#include "piecesquaretable.h"

void Search::orderMoves(std::array<std::optional<Move>, 216> &moves) {
    // Define a lambda function to calculate the score of a move
    auto getMoveScore = [](const Move &move) -> int {
        return getPieceValue(move.capture) * 10 - getPieceValue(move.pieceFrom);
    };

    std::sort(moves.begin(), moves.end(),
              [&](const std::optional<Move> &a, const std::optional<Move> &b) {
                  if (!a.has_value()) return false;
                  if (!b.has_value()) return true;
                  return getMoveScore(a.value()) > getMoveScore(b.value());
              });
}

void Search::startIterativeSearch(long time) {
    searchCancelled = false;
    std::thread timerThread([time]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        searchCancelled = true;
    });

    Move bestMoveThisIteration = Move(0, 0);
    int evalThisIteration = 0;

    for (int i = 1; i <= 64; i++) {
        currentDepth = i;
        currentEval = search(i, bestMoveThisIteration);
        if (!searchCancelled) {
            bestMoveThisIteration = bestMove;
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

int Search::search(int rootDepth, int depth, int alpha, int beta, Move iterativeStart) {
    if (depth == 0) {
        return evaluate();
    }

    std::array<std::optional<Move>, 216> moves = Movegen::generateAllLegalMovesOnBoard();
    orderMoves(moves);
    bool noLegalMoves = true;
    bool consideredIterativeStart = false;

    for (int i = 0; i < 216; i++) {
        if (searchCancelled) {
            return 0;
        }

        std::optional<Move> optionalMove = moves[i];
        Move move = iterativeStart;

        if (!consideredIterativeStart && iterativeStart.from != iterativeStart.to && rootDepth == depth) {
            consideredIterativeStart = true;
            i--;
        }else {
            if (!optionalMove.has_value()) {
                break;
            }
            move = optionalMove.value();
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


