#pragma once

#include <limits>
#include "board.h"

namespace Search {

    inline constexpr int PIECE_VALUES[6] = {
        100,
        300,
        300,
        910,
        100000,
        500
    };

    inline constexpr int NEGATIVE_INFINITY = std::numeric_limits<int>::min();
    inline constexpr int POSITIVE_INFINITY = std::numeric_limits<int>::max();

    inline int searchedNodes = 0;

    inline int currentEval = 0;
    inline Move bestMove = Move(0, 0);

    Move getBestMove(int depth);

    int search(int depth, int alpha, int beta);
    int search(int depth);

    int evaluate();

    int getPieceValue(uint8_t piece);

}
