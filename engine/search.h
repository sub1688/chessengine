#pragma once

#include <limits>
#include <optional>

#include "board.h"

namespace Search {
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
    inline Move bestMove = Move(0, -1);


    void orderMoves(std::array<std::optional<Move>, 216> &moves);

    int search(int rootDepth, int depth, int alpha, int beta);

    int search(int depth);

    int evaluate();

    int getPieceValue(uint8_t piece);
}
