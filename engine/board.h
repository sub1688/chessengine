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

#define PROMOTE_KNIGHT 0
#define PROMOTE_BISHOP 1
#define PROMOTE_ROOK 2
#define PROMOTE_QUEEN 3

struct Move {
    uint8_t from;
    uint8_t to;
    uint8_t pieceFrom = 12;
    uint8_t capture = 12;

    int promotion = -1;
    int enPassantTarget = -1;
    bool castle;

    Move(uint8_t m_from, uint8_t m_to) : from(m_from), to(m_to) {
    }
};

namespace Board {
    inline bool whiteToMove = true;

    // En Passant
    // If the number of moves per game exceeds 1024, we are in deep trouble...
    inline uint64_t epMasks[1024];
    inline uint8_t castleRights[1024];
    inline int moveNumber = 0;

    inline uint64_t BITBOARDS[12];
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

    bool canWhiteCastleKingside(int moveNumber);

    bool canWhiteCastleQueenside(int moveNumber);

    bool canBlackCastleKingside(int moveNumber);

    bool canBlackCastleQueenside(int moveNumber);

    void setWhiteCastleKingside(int moveNumber, bool value);

    void setWhiteCastleQueenside(int moveNumber, bool value);

    void setBlackCastleKingside(int moveNumber, bool value);

    void setBlackCastleQueenside(int moveNumber, bool value);
}
