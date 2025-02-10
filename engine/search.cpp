#include "search.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <thread>

#include "movegen.h"
#include "piecesquaretable.h"

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
}

SearchResult Search::search(int rootDepth, int depth, int alpha, int beta) {

}

SearchResult Search::search(int depth) {
    return search(depth, depth, NEGATIVE_INFINITY, POSITIVE_INFINITY);
}

int Search::quiesce(int alpha, int beta) {
    int standingPat = evaluate();
    if (standingPat >= beta)
        return beta;
    if (alpha < standingPat)
        alpha = standingPat;

    ArrayVec<Move, 218> captures = Movegen::generateAllCapturesOnBoard();
    orderMoves(captures, Move());

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
            }
            else {
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
