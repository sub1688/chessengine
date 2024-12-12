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
    switch (m.promotion) {
        case PROMOTE_QUEEN:
            setPiece(m.to, whiteToMove ? WHITE_QUEEN : BLACK_QUEEN);
        break;
        case PROMOTE_BISHOP:
            setPiece(m.to, whiteToMove ? WHITE_BISHOP : BLACK_KNIGHT);
        break;
        case PROMOTE_KNIGHT:
            setPiece(m.to, whiteToMove ? WHITE_KNIGHT : BLACK_KNIGHT);
        break;
        case PROMOTE_ROOK:
            setPiece(m.to, whiteToMove ? WHITE_ROOK : BLACK_ROOK);
        break;
        default:
            setPiece(m.to, m.pieceFrom);
    }
    setPiece(m.from, NONE);

    if (m.enPassantTarget != -1) {
        setPiece(m.enPassantTarget, NONE);
    }

    if (m.castle) {
        switch (m.to) {
            case 6:
                setPiece(7, NONE);
                setPiece(5, WHITE_ROOK);
            break;
            case 2:
                setPiece(0, NONE);
                setPiece(3, WHITE_ROOK);
            break;
            case 62:
                setPiece(63, NONE);
                setPiece(61, BLACK_ROOK);
            break;
            case 58:
                setPiece(56, NONE);
                setPiece(59, BLACK_ROOK);
            break;
        }
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
    } else {
        epMasks[moveNumber] = 0ULL;
    }

    // Preset previous castle rights
    setWhiteCastleKingside(moveNumber, canWhiteCastleKingside(moveNumber - 1));
    setWhiteCastleQueenside(moveNumber, canWhiteCastleQueenside(moveNumber - 1));
    setBlackCastleKingside(moveNumber, canBlackCastleKingside(moveNumber - 1));
    setBlackCastleQueenside(moveNumber, canBlackCastleQueenside(moveNumber - 1));

    // castle rights
    if (m.pieceFrom == WHITE_KING) {
        setWhiteCastleKingside(moveNumber, false);
        setWhiteCastleQueenside(moveNumber, false);
    } else if (m.pieceFrom == BLACK_KING) {
        setBlackCastleKingside(moveNumber, false);
        setBlackCastleQueenside(moveNumber, false);
    } else if (m.pieceFrom == WHITE_ROOK) {
        if (m.from == 7) {
            setWhiteCastleKingside(moveNumber, false);
        }else if (m.from == 0) {
            setWhiteCastleQueenside(moveNumber, false);
        }
    } else if (m.pieceFrom == BLACK_ROOK) {
        if (m.from == 63) {
            setBlackCastleKingside(moveNumber, false);
        }else if (m.from == 56) {
            setBlackCastleQueenside(moveNumber, false);
        }
    }

    whiteToMove = !whiteToMove;
    return true;
}

void Board::undoMove(Move m) {
    setPiece(m.from, m.pieceFrom);
    if (m.enPassantTarget != -1) {
        setPiece(m.enPassantTarget, m.capture);
        setPiece(m.to, NONE);
    } else {
        setPiece(m.to, m.capture);
    }

    if (m.castle) {
        switch (m.to) {
            case 6:
                setPiece(7, WHITE_ROOK);
                setPiece(5, NONE);
            break;
            case 2:
                setPiece(0, WHITE_ROOK);
                setPiece(3, NONE);
            break;
            case 62:
                setPiece(63, BLACK_ROOK);
                setPiece(61, NONE);
            break;
            case 58:
                setPiece(56, BLACK_ROOK);
                setPiece(59, NONE);
            break;
            default:
                break;
        }
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

    setWhiteCastleKingside(0, true);
    setWhiteCastleQueenside(0, true);
    setBlackCastleKingside(0, true);
    setBlackCastleQueenside(0, true);

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

    index++;

    Board::whiteToMove = fen[index] == 'w';
    index += 2;

    // Parse castling rights
    castleRights[moveNumber] = 0; // Reset castling rights
    while (fen[index] != ' ') {
        char c = fen[index];
        switch (c) {
            case 'K': setWhiteCastleKingside(moveNumber, true); break;
            case 'Q': setWhiteCastleQueenside(moveNumber, true); break;
            case 'k': setBlackCastleKingside(moveNumber, true); break;
            case 'q': setBlackCastleQueenside(moveNumber, true); break;
            case '-': break; // No castling rights
            default:
                throw std::invalid_argument("Invalid castling rights character: " + std::string(1, c));
        }
        index++;
    }

    // Update occupancy bitboards
    updateOccupancy();
}

bool Board::canWhiteCastleKingside(int moveNumber) {
    return 1U & castleRights[moveNumber];
}

bool Board::canWhiteCastleQueenside(int moveNumber) {
    return 1U & (castleRights[moveNumber] >> 1);
}

bool Board::canBlackCastleKingside(int moveNumber) {
    return 1U & (castleRights[moveNumber] >> 2);
}

bool Board::canBlackCastleQueenside(int moveNumber) {
    return 1U & (castleRights[moveNumber] >> 3);
}

void Board::setWhiteCastleKingside(int moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= 1U; // Set bit 0
    } else {
        castleRights[moveNumber] &= ~1U; // Clear bit 0
    }
}

void Board::setWhiteCastleQueenside(int moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= (1U << 1); // Set bit 1
    } else {
        castleRights[moveNumber] &= ~(1U << 1); // Clear bit 1
    }
}

void Board::setBlackCastleKingside(int moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= (1U << 2); // Set bit 2
    } else {
        castleRights[moveNumber] &= ~(1U << 2); // Clear bit 2
    }
}

void Board::setBlackCastleQueenside(int moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= (1U << 3); // Set bit 3
    } else {
        castleRights[moveNumber] &= ~(1U << 3); // Clear bit 3
    }
}
