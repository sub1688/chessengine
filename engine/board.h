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

/**
 *  64 bit integer:
 *  |from(5 bits)|to(5 bits)|en passant target(3 bits)|piece from(4 bits)|piece captured(4 bits)|piece promoted(2 bits)|castle flag(1 bit)|
 *  Total: 26 bits
 */
struct Move {
    uint8_t from = 0;
    uint8_t to = 0;
    uint8_t enPassantTarget = 0;
    uint8_t pieceFrom = NONE;
    uint8_t capture = NONE;
    uint8_t promotion = NONE;

    bool castle = false;

    Move() = default;

    Move(uint8_t m_from, uint8_t m_to) : from(m_from), to(m_to) {}

    bool operator==(const Move & move) const {
        return from == move.from && to == move.to && enPassantTarget == move.enPassantTarget && pieceFrom == move.pieceFrom && capture == move.capture && promotion == move.promotion && castle == move.castle;
    };
};

class Board {
public:
    bool whiteToMove = true;

    uint64_t epMasks[1024] = {};
    uint8_t castleRights[1024] = {};
    uint64_t zobristHistory[1024] = {};
    uint8_t fiftyMoveHistory[1024] = {};
    uint32_t moveNumber = 0;

    uint8_t mailbox[64] = {};

    uint64_t BITBOARDS[12] = {};
    uint64_t BITBOARD_OCCUPANCY = 0ULL;
    uint64_t BITBOARD_WHITE_OCCUPANCY = 0ULL;
    uint64_t BITBOARD_BLACK_OCCUPANCY = 0ULL;

    uint64_t currentZobristKey = 0ULL;

    bool move(Move m);

    void undoMove(Move m);

    void undoMove(Move m, bool ignoreZobrist);

    void updateOccupancy();

    void setStartingPosition();


    void printBoard();

    void importFEN(const std::string &fen);

    bool canWhiteCastleKingside(uint32_t moveNumber);

    bool canWhiteCastleQueenside(uint32_t moveNumber);

    bool canBlackCastleKingside(uint32_t moveNumber);

    bool canBlackCastleQueenside(uint32_t moveNumber);

    void setWhiteCastleKingside(uint32_t moveNumber, bool value);

    void setWhiteCastleQueenside(uint32_t moveNumber, bool value);

    void setBlackCastleKingside(uint32_t moveNumber, bool value);

    void setBlackCastleQueenside(uint32_t moveNumber, bool value);

    bool isDrawn();

    [[nodiscard]] uint8_t getPiece(int index) const {
        return mailbox[index];
    }

    inline void setPiece(int index, uint8_t piece);

    std::string generateFEN();
};
