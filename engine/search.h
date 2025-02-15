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

    inline int searchedNodes = 0;
    inline int currentEval = 0;
    inline int currentDepth = 0;
    inline bool lastSearchTurnIsWhite = true;

    volatile inline bool searchCancelled = false;

    inline Move bestMove = NULL_MOVE;

    void orderMoves(ArrayVec<Move, 218>& moveVector, Move ttMove);

    void startIterativeSearch(long time);

    SearchResult search(int rootDepth, int depth, int alpha, int beta);

    SearchResult search(int depth);

    int evaluate();

    int quiesce(int alpha, int beta);

    int getPieceValue(uint8_t piece);

    bool isInEndgame();

    bool isNullMove(Move move);
}
