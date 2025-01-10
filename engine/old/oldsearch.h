#pragma once

#include <limits>
#include <optional>

#include "../board.h"
#include "../../util/arrayvec.h"

namespace OldSearch {
    //TODO: PIECE SQUARE TABLES
    inline constexpr int PIECE_VALUES[6] = {
        100,
        300,
        300,
        910,
        100000,
        500
    };

    inline constexpr int NEGATIVE_INFINITY = std::numeric_limits<int>::min() + 1;
    inline constexpr int POSITIVE_INFINITY = std::numeric_limits<int>::max() - 1;

    inline int searchedNodes = 0;
    inline int currentEval = 0;
    inline int currentDepth = 0;
    inline bool searchCancelled = false;
    inline bool quiescence = false;

    inline Move bestMove = Move(0, -1);

    void orderMoves(ArrayVec<Move, 218>& moveVector);

    void startIterativeSearch(long time);

    int search(int rootDepth, int depth, int alpha, int beta, Move iterativeStart);

    int search(int depth, Move iterativeStart);

    int evaluate();

    int getPieceValue(uint8_t piece);

    bool isInEndgame();

    int quiesce(int alpha, int beta);
}
