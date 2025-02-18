#pragma once

#define MATE_THRESHOLD 200000000

#include <limits>
#include <optional>

#include "board.h"
#include "../util/arrayvec.h"

struct SearchResult {
    int evaluation;
    Move bestMove;

    SearchResult(int m_evaluation, Move m_bestMove) : evaluation(m_evaluation), bestMove(m_bestMove) {
    }
};

namespace Search {
    inline constexpr Move NULL_MOVE = Move();

    inline constexpr uint64_t WHITE_PASSED_PAWN_MASKS[64] = {
        0x303030303030300ULL,
        0x707070707070700ULL,
        0xe0e0e0e0e0e0e00ULL,
        0x1c1c1c1c1c1c1c00ULL,
        0x3838383838383800ULL,
        0x7070707070707000ULL,
        0xe0e0e0e0e0e0e000ULL,
        0xc0c0c0c0c0c0c000ULL,
        0x303030303030000ULL,
        0x707070707070000ULL,
        0xe0e0e0e0e0e0000ULL,
        0x1c1c1c1c1c1c0000ULL,
        0x3838383838380000ULL,
        0x7070707070700000ULL,
        0xe0e0e0e0e0e00000ULL,
        0xc0c0c0c0c0c00000ULL,
        0x303030303000000ULL,
        0x707070707000000ULL,
        0xe0e0e0e0e000000ULL,
        0x1c1c1c1c1c000000ULL,
        0x3838383838000000ULL,
        0x7070707070000000ULL,
        0xe0e0e0e0e0000000ULL,
        0xc0c0c0c0c0000000ULL,
        0x303030300000000ULL,
        0x707070700000000ULL,
        0xe0e0e0e00000000ULL,
        0x1c1c1c1c00000000ULL,
        0x3838383800000000ULL,
        0x7070707000000000ULL,
        0xe0e0e0e000000000ULL,
        0xc0c0c0c000000000ULL,
        0x303030000000000ULL,
        0x707070000000000ULL,
        0xe0e0e0000000000ULL,
        0x1c1c1c0000000000ULL,
        0x3838380000000000ULL,
        0x7070700000000000ULL,
        0xe0e0e00000000000ULL,
        0xc0c0c00000000000ULL,
        0x303000000000000ULL,
        0x707000000000000ULL,
        0xe0e000000000000ULL,
        0x1c1c000000000000ULL,
        0x3838000000000000ULL,
        0x7070000000000000ULL,
        0xe0e0000000000000ULL,
        0xc0c0000000000000ULL,
        0x300000000000000ULL,
        0x700000000000000ULL,
        0xe00000000000000ULL,
        0x1c00000000000000ULL,
        0x3800000000000000ULL,
        0x7000000000000000ULL,
        0xe000000000000000ULL,
        0xc000000000000000ULL,
        0x303030303030303ULL,
        0x707070707070707ULL,
        0xe0e0e0e0e0e0e0eULL,
        0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL,
        0x7070707070707070ULL,
        0xe0e0e0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0c0c0ULL
    };

    inline constexpr uint64_t BLACK_PASSED_PAWN_MASKS[64] = {
        0x3ULL,
        0x7ULL,
        0xeULL,
        0x1cULL,
        0x38ULL,
        0x70ULL,
        0xe0ULL,
        0xc0ULL,
        0x303ULL,
        0x707ULL,
        0xe0eULL,
        0x1c1cULL,
        0x3838ULL,
        0x7070ULL,
        0xe0e0ULL,
        0xc0c0ULL,
        0x30303ULL,
        0x70707ULL,
        0xe0e0eULL,
        0x1c1c1cULL,
        0x383838ULL,
        0x707070ULL,
        0xe0e0e0ULL,
        0xc0c0c0ULL,
        0x3030303ULL,
        0x7070707ULL,
        0xe0e0e0eULL,
        0x1c1c1c1cULL,
        0x38383838ULL,
        0x70707070ULL,
        0xe0e0e0e0ULL,
        0xc0c0c0c0ULL,
        0x303030303ULL,
        0x707070707ULL,
        0xe0e0e0e0eULL,
        0x1c1c1c1c1cULL,
        0x3838383838ULL,
        0x7070707070ULL,
        0xe0e0e0e0e0ULL,
        0xc0c0c0c0c0ULL,
        0x30303030303ULL,
        0x70707070707ULL,
        0xe0e0e0e0e0eULL,
        0x1c1c1c1c1c1cULL,
        0x383838383838ULL,
        0x707070707070ULL,
        0xe0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0ULL,
        0x3030303030303ULL,
        0x7070707070707ULL,
        0xe0e0e0e0e0e0eULL,
        0x1c1c1c1c1c1c1cULL,
        0x38383838383838ULL,
        0x70707070707070ULL,
        0xe0e0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0c0ULL,
        0x303030303030303ULL,
        0x707070707070707ULL,
        0xe0e0e0e0e0e0e0eULL,
        0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL,
        0x7070707070707070ULL,
        0xe0e0e0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0c0c0ULL,
    };

    inline constexpr int PIECE_VALUES[12] = {
        100,
        300,
        300,
        910,
        100000,
        500,
        -100,
        -300,
        -300,
        -910,
        -100000,
        -500
    };

    inline constexpr int NEGATIVE_INFINITY = std::numeric_limits<int>::min() + 1;
    inline constexpr int POSITIVE_INFINITY = std::numeric_limits<int>::max() - 1;

    inline constexpr int TRANSPOSITION_TABLE_BIAS = 10000000;
    inline constexpr int LOSING_CAPTURE_BIAS = 2000000;
    inline constexpr int WINNING_CAPTURE_BIAS = 8000000;
    inline constexpr int PROMOTE_BIAS = 6000000;

    inline int currentEval = 0;
    inline int currentDepth = 0;
    inline bool lastSearchTurnIsWhite = true;

    volatile inline bool searchCancelled = false;

    inline Move bestMove = NULL_MOVE;

    void orderMoves(ArrayVec<Move, 218> &moveVector, Move ttMove);

    void startIterativeSearch(long time);

    SearchResult search(int rootDepth, int depth, int alpha, int beta);

    SearchResult search(int depth);

    int evaluate();

    int quiesce(int alpha, int beta);

    int getPieceValue(uint8_t piece);

    double getEndGameBias();

    bool isNullMove(Move move);

    int evaluatePassedPawn(uint8_t squareIndex, uint8_t piece);

    int evaluateKingDistance(uint8_t squareIndex, uint8_t otherKingIndex, uint8_t piece, int materialDelta);
}
