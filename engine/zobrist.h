#pragma once

#include <cstdint>

namespace Zobrist {
    inline uint64_t whiteToMove;
    inline uint64_t enPassantKeys[8];
    inline uint64_t castleRightsKeys[8];
    inline uint64_t pieceSquareKeys[64][12];

    void init();

    uint64_t secureRandomUnsigned64();

    uint64_t calculateZobristKey();
}
