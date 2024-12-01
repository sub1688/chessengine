#pragma once
#include <cstdint>
#include <string>

#define NONE 12
#define WHITE_PAWN 0
#define WHITE_KNIGHT 1
#define WHITE_BISHOP 2
#define WHITE_QUEEN 3
#define WHITE_KING 4
#define WHITE_ROOK 5
#define BLACK_PAWN 6
#define BLACK_KNIGHT 7
#define BLACK_BISHOP 8
#define BLACK_QUEEN 9
#define BLACK_KING 10
#define BLACK_ROOK 11

namespace Board {
    inline uint8_t pieceSquareTable[64];

    inline bool whiteToMove = true;

    inline uint64_t BITBOARD_BLACK_ROOK = 0ULL;
    inline uint64_t BITBOARD_BLACK_KNIGHT = 0ULL;
    inline uint64_t BITBOARD_BLACK_BISHOP = 0ULL;
    inline uint64_t BITBOARD_BLACK_QUEEN = 0ULL;
    inline uint64_t BITBOARD_BLACK_KING = 0ULL;
    inline uint64_t BITBOARD_BLACK_PAWN = 0ULL;
    inline uint64_t BITBOARD_WHITE_ROOK = 0ULL;
    inline uint64_t BITBOARD_WHITE_KNIGHT = 0ULL;
    inline uint64_t BITBOARD_WHITE_BISHOP = 0ULL;
    inline uint64_t BITBOARD_WHITE_QUEEN = 0ULL;
    inline uint64_t BITBOARD_WHITE_KING = 0ULL;
    inline uint64_t BITBOARD_WHITE_PAWN = 0ULL;
    inline uint64_t BITBOARD_OCCUPANCY = 0ULL;
    inline uint64_t BITBOARD_WHITE_OCCUPANCY = 0ULL;
    inline uint64_t BITBOARD_BLACK_OCCUPANCY = 0ULL;

    void updateOccupancy();

    void setStartingPosition();

    uint8_t getPiece(int index);

    uint8_t move(int index, int targetIndex);

    void printBoard();

    void importFEN(const std::string &fen);
}
