#include "search.h"
#include <array>
#include <bits/ctype_base.h>

#include "movegen.h"

Move Search::getBestMove(int depth) {
    std::array<std::optional<Move>, 216> moves = Movegen::generateAllLegalMovesOnBoard();
    int bestScore = NEGATIVE_INFINITY;
    Move bestMove = Move(0, 0);

    int alpha = NEGATIVE_INFINITY;
    int beta = POSITIVE_INFINITY;

    for (int i = 0; i < 216; i++) {
        std::optional<Move> moveOptional = moves[i];
        if (!moveOptional.has_value()) {
            break;
        }

        Move move = moveOptional.value();

        if (Board::move(move)) {
            searchedNodes++;

            int score = -search(depth - 1, alpha, beta);

            Board::undoMove(move);

            // Update best score
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
                if (score > NEGATIVE_INFINITY) {
                    alpha = score;
                }
            }

            if (score >= beta)
                break;
        }
    }
    return bestMove;
}


int Search::search(int depth) {
    return search(depth, NEGATIVE_INFINITY, POSITIVE_INFINITY);
}

int Search::search(int depth, int alpha, int beta) {
    // TODO: implement quiescence search
    if (depth == 0) {
        return evaluate();
    }

    std::array<std::optional<Move>, 216> moves = Movegen::generateAllLegalMovesOnBoard();
    int bestScore = NEGATIVE_INFINITY;
    bool foundLegalMove = false;

    for (int i = 0; i < 216; i++) {
        std::optional<Move> moveOptional = moves[i];
        if (!moveOptional.has_value()) {
            break;
        }

        Move move = moveOptional.value();

        if (Board::move(move)) {
            searchedNodes++;
            foundLegalMove = true;

            int score = -search(depth - 1, -beta, -alpha);

            Board::undoMove(move);

            // Update best score
            if (score > bestScore) {
                bestScore = score;
                if (score > alpha) {
                    alpha = score;
                }
            }

            if (score >= beta)
                break;
        }
    }

    // Checkmate / Stalemate handling
    if (!foundLegalMove) {
        return Movegen::isKingInDanger(Board::whiteToMove)
            ? NEGATIVE_INFINITY + depth
            : 0;
    }

    return bestScore;
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

