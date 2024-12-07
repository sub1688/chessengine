#include "board.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>

namespace Movegen {
    bool isKingInDanger(bool white);
}

uint8_t Board::getPiece(int index) {
    uint64_t mask = 1ULL << index;

    for (uint8_t i = 0; i < 12; i++) {
        if (mask & BITBOARDS[i])
            return i;
    }

    return NONE;
}

void Board::setPiece(int index, uint8_t piece) {
    assert(index >= 0 && index < 64);
    assert(piece <= 12); // 12 represents NONE

    uint64_t mask = 1ULL << index;
    for (int i = 0; i < 12; i++) {
        BITBOARDS[i] &= ~mask;
    }
    if (piece < 12) {
        BITBOARDS[piece] |= mask;
    }
}

bool Board::move(Move m) {
    setPiece(m.to, m.pieceFrom);
    setPiece(m.from, NONE);

    if (m.enPassantTarget != -1) {
        setPiece(m.enPassantTarget, NONE);
    }

    updateOccupancy();
    if (Movegen::isKingInDanger(whiteToMove)) {
        whiteToMove = !whiteToMove;
        moveNumber++;
        undoMove(m);
        return false;
    }

    moveNumber++;

    // en passant
    if (abs(m.to - m.from) == 16 && (m.pieceFrom == WHITE_PAWN || m.pieceFrom == BLACK_PAWN)) {
        epMasks[moveNumber] = 1ULL << (whiteToMove ? m.from + 8 : m.from - 8);
    }else {
        epMasks[moveNumber] = 0ULL;
    }

    whiteToMove = !whiteToMove;
    return true;
}

void Board::undoMove(Move m) {
    setPiece(m.from, m.pieceFrom);
    if (m.enPassantTarget != -1) {
        setPiece(m.enPassantTarget, m.capture);
        setPiece(m.to, NONE);
    }else {
        setPiece(m.to, m.capture);
    }

    // en passant
    moveNumber--;

    updateOccupancy();
    whiteToMove = !whiteToMove;
}

void Board::updateOccupancy() {
    BITBOARD_WHITE_OCCUPANCY = BITBOARDS[WHITE_PAWN] | BITBOARDS[WHITE_KNIGHT] | BITBOARDS[WHITE_BISHOP] |
                               BITBOARDS[WHITE_QUEEN] | BITBOARDS[WHITE_KING] | BITBOARDS[WHITE_ROOK];
    BITBOARD_BLACK_OCCUPANCY = BITBOARDS[BLACK_PAWN] | BITBOARDS[BLACK_KNIGHT] | BITBOARDS[BLACK_BISHOP] |
                               BITBOARDS[BLACK_QUEEN] | BITBOARDS[BLACK_KING] | BITBOARDS[BLACK_ROOK];
    BITBOARD_OCCUPANCY = BITBOARD_WHITE_OCCUPANCY | BITBOARD_BLACK_OCCUPANCY;
}


void Board::setStartingPosition() {
    BITBOARDS[WHITE_PAWN] = 0x000000000000FF00ULL; // Rank 2
    BITBOARDS[WHITE_ROOK] = 0x0000000000000081ULL; // Corners of rank 1
    BITBOARDS[WHITE_KNIGHT] = 0x0000000000000042ULL; // Knights on rank 1
    BITBOARDS[WHITE_BISHOP] = 0x0000000000000024ULL; // Bishops on rank 1
    BITBOARDS[WHITE_QUEEN] = 0x0000000000000008ULL; // Queen on d1
    BITBOARDS[WHITE_KING] = 0x0000000000000010ULL; // King on e1
    BITBOARDS[BLACK_PAWN] = 0x00FF000000000000ULL; // Rank 7
    BITBOARDS[BLACK_ROOK] = 0x8100000000000000ULL; // Corners of rank 8
    BITBOARDS[BLACK_KNIGHT] = 0x4200000000000000ULL; // Knights on rank 8
    BITBOARDS[BLACK_BISHOP] = 0x2400000000000000ULL; // Bishops on rank 8
    BITBOARDS[BLACK_QUEEN] = 0x0800000000000000ULL; // Queen on d8
    BITBOARDS[BLACK_KING] = 0x1000000000000000ULL; // King on e8

    updateOccupancy();
}

void Board::printBoard() {
    const std::string pieces[] = {
        "P", "N", "B", "Q", "K", "R",
        "p", "n", "b", "q", "k", "r",
        ".",
    };

    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int index = rank * 8 + file;
            uint8_t piece = getPiece(index);
            std::cout << pieces[piece] << "  ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void Board::importFEN(const std::string &fen) {
    // Clear all bitboards
    for (int i = 0; i < 12; i++) {
        BITBOARDS[i] = 0ULL;
    }

    size_t index = 0;
    int rank = 7;
    int file = 0;

    // Parse the board description (first part of FEN)
    while (fen[index] != ' ') {
        char c = fen[index];
        if (c == '/') {
            // Move to the next rank
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            // Empty squares
            file += c - '0';
        } else {
            // Map the character to the corresponding bitboard
            int square = rank * 8 + file;
            uint64_t mask = 1ULL << square;

            switch (c) {
                case 'P': BITBOARDS[WHITE_PAWN] |= mask;
                    break;
                case 'N': BITBOARDS[WHITE_KNIGHT] |= mask;
                    break;
                case 'B': BITBOARDS[WHITE_BISHOP] |= mask;
                    break;
                case 'R': BITBOARDS[WHITE_ROOK] |= mask;
                    break;
                case 'Q': BITBOARDS[WHITE_QUEEN] |= mask;
                    break;
                case 'K': BITBOARDS[WHITE_KING] |= mask;
                    break;

                case 'p': BITBOARDS[BLACK_PAWN] |= mask;
                    break;
                case 'n': BITBOARDS[BLACK_KNIGHT] |= mask;
                    break;
                case 'b': BITBOARDS[BLACK_BISHOP] |= mask;
                    break;
                case 'r': BITBOARDS[BLACK_ROOK] |= mask;
                    break;
                case 'q': BITBOARDS[BLACK_QUEEN] |= mask;
                    break;
                case 'k': BITBOARDS[BLACK_KING] |= mask;
                    break;

                default:
                    throw std::invalid_argument("Invalid FEN character: " + std::string(1, c));
            }
            file++;
        }
        index++;
    }

    // Update occupancy bitboards
    updateOccupancy();
}

