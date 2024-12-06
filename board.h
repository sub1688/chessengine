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

struct Move {
    uint8_t from;
    uint8_t to;
    uint8_t pieceFrom = 12;
    uint8_t capture = 12;

    Move(uint8_t m_from, uint8_t m_to) : from(m_from), to(m_to) {}
};

namespace Board {
    inline uint8_t pieceSquareTable[64];

    inline bool whiteToMove = true;

    inline uint64_t BITBOARDS[12];
    inline uint64_t ATTACK_RAYS = 0ULL;
    inline uint64_t PIN_MASK = 0ULL;
    inline uint64_t BITBOARD_OCCUPANCY;
    inline uint64_t BITBOARD_WHITE_OCCUPANCY = 0ULL;
    inline uint64_t BITBOARD_BLACK_OCCUPANCY = 0ULL;

    bool move(Move m);
    void undoMove(Move m);

    void updateOccupancy();

    void setStartingPosition();

    uint8_t getPiece(int index);

    void setPiece(int index, uint8_t piece);

    void printBoard();

    void importFEN(const std::string &fen);
}
