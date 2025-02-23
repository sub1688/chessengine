#pragma once

#include <string>
#include <vector>

class Board;
struct Move;

namespace StandardAlgebraicNotation {
    int stringToSquare(const std::string &square);

    std::string boardToSan(Board& board, const Move &move);

    bool requiresDisambiguation(Board& board, const Move &move);

    std::string disambiguation(Board& board, const Move &move);

    std::string toUci(const Move &move);

    std::string squareToString(int square);

    char getSanPieceChar(int piece);

    char getSanPiecePromotion(int piece);

    std::string squareToFile(int square);

    std::vector<std::string> split(const std::string &str, char delimiter);
}
