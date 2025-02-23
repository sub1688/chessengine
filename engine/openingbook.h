#pragma once
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include "board.h"

struct MoveEntry {
    std::string move;
    int probability;

    MoveEntry(std::string move, int probability) : move(std::move(move)), probability(probability) {}
};

namespace OpeningBook {
    static inline std::unordered_map<std::string, std::vector<MoveEntry>> openingBook;

    void loadOpeningBook(std::string filename);

    Move fetchNextBookMove(Board& board);
}
