#pragma once

#include <string>

#include "board.h"
#include <vector>

namespace StandardAlgebraicNotation {
    int stringToSquare(const std::string &square);

    std::string boardToSan(const Move &move);

    bool requiresDisambiguation(const Move &move);

    std::string disambiguation(const Move &move);

    std::string toUci(const Move &move);

    std::string squareToString(int square);

    char getSanPieceChar(int piece);

    char getSanPiecePromotion(int piece);

    std::string squareToFile(int square);

    std::vector<std::string> split(const std::string &str, char delimiter);
}
