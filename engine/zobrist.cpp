#include "zobrist.h"
#include <random>
#include <cstdint>
#include "board.h"


void Zobrist::init() {
    int index = 0;
    for (auto &pieceSquareKey: pieceSquareKeys) {
        for (uint64_t &j: pieceSquareKey) {
            j = randoms[index++];
        }
    }

    for (uint64_t &castleRightsKey: castleRightsKeys) {
        castleRightsKey = randoms[index++];
    }

    for (uint64_t &enPassantKey: enPassantKeys) {
        enPassantKey = randoms[index++];
    }

    whiteToMove = randoms[index];
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

