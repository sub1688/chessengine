#include "board.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>


uint8_t Board::getPiece(int index) {
    assert(index >= 0 && index < 64);

    uint64_t mask = 1ULL << index;

    if (BITBOARD_WHITE_PAWN & mask) return WHITE_PAWN;
    if (BITBOARD_WHITE_KNIGHT & mask) return WHITE_KNIGHT;
    if (BITBOARD_WHITE_BISHOP & mask) return WHITE_BISHOP;
    if (BITBOARD_WHITE_ROOK & mask) return WHITE_ROOK;
    if (BITBOARD_WHITE_QUEEN & mask) return WHITE_QUEEN;
    if (BITBOARD_WHITE_KING & mask) return WHITE_KING;

    if (BITBOARD_BLACK_PAWN & mask) return BLACK_PAWN;
    if (BITBOARD_BLACK_KNIGHT & mask) return BLACK_KNIGHT;
    if (BITBOARD_BLACK_BISHOP & mask) return BLACK_BISHOP;
    if (BITBOARD_BLACK_ROOK & mask) return BLACK_ROOK;
    if (BITBOARD_BLACK_QUEEN & mask) return BLACK_QUEEN;
    if (BITBOARD_BLACK_KING & mask) return BLACK_KING;

    return NONE;
}

void Board::updateOccupancy() {
    BITBOARD_OCCUPANCY = BITBOARD_WHITE_PAWN | BITBOARD_BLACK_PAWN | BITBOARD_WHITE_KNIGHT | BITBOARD_BLACK_KNIGHT |
                         BITBOARD_WHITE_BISHOP | BITBOARD_BLACK_BISHOP | BITBOARD_WHITE_QUEEN | BITBOARD_BLACK_QUEEN |
                         BITBOARD_WHITE_KING | BITBOARD_BLACK_KING | BITBOARD_WHITE_ROOK | BITBOARD_BLACK_KING;
    BITBOARD_WHITE_OCCUPANCY = BITBOARD_WHITE_PAWN | BITBOARD_WHITE_KNIGHT | BITBOARD_WHITE_BISHOP |
                               BITBOARD_WHITE_QUEEN | BITBOARD_WHITE_KING | BITBOARD_WHITE_ROOK;
    BITBOARD_BLACK_OCCUPANCY = BITBOARD_BLACK_PAWN | BITBOARD_BLACK_KNIGHT | BITBOARD_BLACK_BISHOP |
                               BITBOARD_BLACK_QUEEN | BITBOARD_BLACK_KING | BITBOARD_BLACK_KING;
}


void Board::setStartingPosition() {
    BITBOARD_WHITE_PAWN = 0x000000000000FF00ULL; // Rank 2
    BITBOARD_WHITE_ROOK = 0x0000000000000081ULL; // Corners of rank 1
    BITBOARD_WHITE_KNIGHT = 0x0000000000000042ULL; // Knights on rank 1
    BITBOARD_WHITE_BISHOP = 0x0000000000000024ULL; // Bishops on rank 1
    BITBOARD_WHITE_QUEEN = 0x0000000000000008ULL; // Queen on d1
    BITBOARD_WHITE_KING = 0x0000000000000010ULL; // King on e1
    BITBOARD_BLACK_PAWN = 0x00FF000000000000ULL; // Rank 7
    BITBOARD_BLACK_ROOK = 0x8100000000000000ULL; // Corners of rank 8
    BITBOARD_BLACK_KNIGHT = 0x4200000000000000ULL; // Knights on rank 8
    BITBOARD_BLACK_BISHOP = 0x2400000000000000ULL; // Bishops on rank 8
    BITBOARD_BLACK_QUEEN = 0x0800000000000000ULL; // Queen on d8
    BITBOARD_BLACK_KING = 0x1000000000000000ULL; // King on e8

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
    BITBOARD_WHITE_PAWN = BITBOARD_WHITE_KNIGHT = BITBOARD_WHITE_BISHOP = 0ULL;
    BITBOARD_WHITE_ROOK = BITBOARD_WHITE_QUEEN = BITBOARD_WHITE_KING = 0ULL;
    BITBOARD_BLACK_PAWN = BITBOARD_BLACK_KNIGHT = BITBOARD_BLACK_BISHOP = 0ULL;
    BITBOARD_BLACK_ROOK = BITBOARD_BLACK_QUEEN = BITBOARD_BLACK_KING = 0ULL;

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
                case 'P': BITBOARD_WHITE_PAWN |= mask;
                    break;
                case 'N': BITBOARD_WHITE_KNIGHT |= mask;
                    break;
                case 'B': BITBOARD_WHITE_BISHOP |= mask;
                    break;
                case 'R': BITBOARD_WHITE_ROOK |= mask;
                    break;
                case 'Q': BITBOARD_WHITE_QUEEN |= mask;
                    break;
                case 'K': BITBOARD_WHITE_KING |= mask;
                    break;

                case 'p': BITBOARD_BLACK_PAWN |= mask;
                    break;
                case 'n': BITBOARD_BLACK_KNIGHT |= mask;
                    break;
                case 'b': BITBOARD_BLACK_BISHOP |= mask;
                    break;
                case 'r': BITBOARD_BLACK_ROOK |= mask;
                    break;
                case 'q': BITBOARD_BLACK_QUEEN |= mask;
                    break;
                case 'k': BITBOARD_BLACK_KING |= mask;
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
