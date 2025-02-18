#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "board.h"

namespace OpeningBook {
    static inline std::unordered_map<std::string, std::vector<Move>> openingBook;

    void loadOpeningBook(std::string filename);

    Move fetchNextBookMove();
}
