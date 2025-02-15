#pragma once

#include <string>

#include "board.h"

namespace StandardAlgebraicNotation {
    std::string boardToSan(const Move &move);

    bool requiresDisambiguation(const Move &move);

    std::string disambiguation(const Move &move);

    std::string toUci(const Move &move);

    std::string squareToString(int square);

    char getSanPieceChar(int piece);

    char getSanPiecePromotion(int piece);

    std::string squareToFile(int square);
}
