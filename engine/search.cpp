#include "search.h"

#include <algorithm>
#include <array>
#include "movegen.h"

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



int Search::search(int depth) {
    bestMove = Move(0, 0);
    return search(depth, depth, NEGATIVE_INFINITY, POSITIVE_INFINITY);
}

int Search::search(int rootDepth, int depth, int alpha, int beta) {
    if (depth == 0) {
        return evaluate();
    }

    std::array<std::optional<Move>, 216> moves = Movegen::generateAllLegalMovesOnBoard();
    orderMoves(moves);
    bool noLegalMoves = true;

    for (int i = 0; i < 216; i++) {
        std::optional<Move> optionalMove = moves[i];

        if (!optionalMove.has_value()) {
            break;
        }

        Move move = optionalMove.value();
        if (Board::move(move)) {
            noLegalMoves = false;
            int score = -search(rootDepth, depth - 1, -beta, -alpha);
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
    for (int i = 0; i < 12; i++) {
        if (i >= 6) {
            totalValue -= __builtin_popcountll(Board::BITBOARDS[i]) * getPieceValue(i);
        }else {
            totalValue += __builtin_popcountll(Board::BITBOARDS[i]) * getPieceValue(i);
        }
    }
    return totalValue * (Board::whiteToMove ? 1 : -1);
}

int Search::getPieceValue(uint8_t piece) {
    return PIECE_VALUES[piece % 6];
}

