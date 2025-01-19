#include "zobrist.h"
#include <random>
#include <cstdint>
#include "board.h"

uint64_t Zobrist::secureRandomUnsigned64() {
    std::random_device rd; // Non-deterministic random number generator
    uint64_t random_number = (static_cast<uint64_t>(rd()) << 32) | rd();
    return random_number;
}

void Zobrist::init() {
    for (auto &pieceSquareKey: pieceSquareKeys) {
        for (uint64_t &j: pieceSquareKey) {
            j = secureRandomUnsigned64();
        }
    }

    for (uint64_t &castleRightsKey: castleRightsKeys) {
        castleRightsKey = secureRandomUnsigned64();
    }

    for (uint64_t &enPassantKey: enPassantKeys) {
        enPassantKey = secureRandomUnsigned64();
    }

    whiteToMove = secureRandomUnsigned64();
}


uint64_t Zobrist::calculateZobristKey() {
    uint64_t zobristKey = 0ULL;
    for (int i = 0; i < 64; i++) {
        int piece = Board::getPiece(i);

        if (piece == NONE)
            continue;

        zobristKey ^= pieceSquareKeys[i][piece];
    }

    uint64_t epMask = Board::epMasks[Board::moveNumber];
    if (epMask != 0ULL) {
        int index = __builtin_ctzll(epMask);
        int file = index % 8;
        zobristKey ^= enPassantKeys[file];
    }

    zobristKey ^= castleRightsKeys[Board::castleRights[Board::moveNumber]];
    if (Board::whiteToMove) {
        zobristKey ^= whiteToMove;
    }

    return zobristKey;
}

