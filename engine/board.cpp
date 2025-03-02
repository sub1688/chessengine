#include "board.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>

#include "movegen.h"
#include "zobrist.h"

namespace Movegen {
    bool isKingInDanger(Board &board, bool white);
}

void Board::setPiece(int index, uint8_t piece) {
    assert(index >= 0 && index < 64);
    assert(piece <= 12); // 12 represents NONE

    uint64_t mask = 1ULL << index;
    if (mailbox[index] != NONE) {
        BITBOARDS[mailbox[index]] &= ~mask;
    }
    if (piece < 12) {
        BITBOARDS[piece] |= mask;
    }

    mailbox[index] = piece;
}

void Board::nullMove() {
    whiteToMove = !whiteToMove;
    currentZobristKey ^= Zobrist::whiteToMove;
    moveNumber++;

    // Zobrist en passant
    uint64_t prevEnPassantMask = epMasks[moveNumber - 1];
    if (prevEnPassantMask != 0) {
        int index = __builtin_ctzll(prevEnPassantMask);
        int file = index & 7;
        currentZobristKey ^= Zobrist::enPassantKeys[file];
    }
}

void Board::undoNullMove() {
    whiteToMove = !whiteToMove;
    moveNumber--;
    currentZobristKey ^= Zobrist::whiteToMove;

    // Remove en passant hash from the last move
    uint64_t prevEnPassantMask = epMasks[moveNumber + 1];
    if (prevEnPassantMask != 0) {
        int index = __builtin_ctzll(prevEnPassantMask);
        int file = index & 7;
        currentZobristKey ^= Zobrist::enPassantKeys[file];
    }

    // Restore en passant hash from before the move
    uint64_t oldEnPassantMask = epMasks[moveNumber];
    if (oldEnPassantMask != 0) {
        int index = __builtin_ctzll(oldEnPassantMask);
        int file = index & 7;
        currentZobristKey ^= Zobrist::enPassantKeys[file];
    }
}


bool Board::move(Move m) {
    setPiece(m.to, m.promotion != NONE ? m.promotion : m.pieceFrom);
    setPiece(m.from, NONE);

    int enPassantZobrist = NONE;
    if (m.enPassantTarget != 0) {
        enPassantZobrist = mailbox[m.enPassantTarget];
        setPiece(m.enPassantTarget, NONE);
    }

    uint8_t from = 0, to = 0;

    if (m.castle) {
        bool kingside = (m.to & 0b111) > 4;
        from = m.to + (kingside ? 1 : -2);
        to = m.to + (kingside ? -1 : 1);

        setPiece(to, getPiece(from));
        setPiece(from, NONE);
    }

    updateOccupancy();
    if (Movegen::isKingInDanger(*this, whiteToMove)) {
        whiteToMove = !whiteToMove;
        moveNumber++;
        undoMove(m, true);
        return false;
    }
    moveNumber++;


    if (m.castle) {
        currentZobristKey ^= Zobrist::pieceSquareKeys[from][whiteToMove ? WHITE_ROOK : BLACK_ROOK];
        currentZobristKey ^= Zobrist::pieceSquareKeys[to][whiteToMove ? WHITE_ROOK : BLACK_ROOK];
    }

    if (enPassantZobrist != NONE) {
        currentZobristKey ^= Zobrist::pieceSquareKeys[m.enPassantTarget][enPassantZobrist];
    }


    // Zobrist en passant
    uint64_t prevEnPassantMask = epMasks[moveNumber - 1];
    if (prevEnPassantMask != 0) {
        int index = __builtin_ctzll(prevEnPassantMask);
        int file = index & 7;
        currentZobristKey ^= Zobrist::enPassantKeys[file];
    }

    fiftyMoveHistory[moveNumber] = fiftyMoveHistory[moveNumber - 1] + 1;

    // en passant
    bool isDoublePawnPush = abs(m.to - m.from) == 16;
    if (isDoublePawnPush && (m.pieceFrom == WHITE_PAWN || m.pieceFrom == BLACK_PAWN)) {
        uint8_t epTarget = whiteToMove ? m.from + 8 : m.from - 8;
        uint8_t epFile = epTarget & 7;
        epMasks[moveNumber] = 1ULL << epTarget;
        currentZobristKey ^= Zobrist::enPassantKeys[epFile];
        fiftyMoveHistory[moveNumber] = 0;
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
        } else if (m.from == 0) {
            setWhiteCastleQueenside(moveNumber, false);
        }
    } else if (m.pieceFrom == BLACK_ROOK) {
        if (m.from == 63) {
            setBlackCastleKingside(moveNumber, false);
        } else if (m.from == 56) {
            setBlackCastleQueenside(moveNumber, false);
        }
    }

    whiteToMove = !whiteToMove;

    currentZobristKey ^= Zobrist::whiteToMove;
    if (m.capture != NONE && m.enPassantTarget == 0) {
        currentZobristKey ^= Zobrist::pieceSquareKeys[m.to][m.capture];
        fiftyMoveHistory[moveNumber] = 0;
    }
    currentZobristKey ^= Zobrist::pieceSquareKeys[m.from][m.pieceFrom];
    currentZobristKey ^= Zobrist::pieceSquareKeys[m.to][mailbox[m.to]];

    currentZobristKey ^= Zobrist::castleRightsKeys[castleRights[moveNumber - 1]];
    currentZobristKey ^= Zobrist::castleRightsKeys[castleRights[moveNumber]];

    zobristHistory[moveNumber] = currentZobristKey;

    return true;
}

void Board::undoMove(Move m) {
    undoMove(m, false);
}


void Board::undoMove(Move m, bool noZobrist) {
    setPiece(m.from, m.pieceFrom);
    if (m.enPassantTarget != 0) {
        setPiece(m.enPassantTarget, m.capture);
        setPiece(m.to, NONE);
    } else {
        setPiece(m.to, m.capture);
    }

    if (m.castle) {
        int from = 0, to = 0;
        bool kingside = (m.to & 0b111) > 4;
        from = m.to + (kingside ? 1 : -2);
        to = m.to + (kingside ? -1 : 1);

        setPiece(from, whiteToMove ? BLACK_ROOK : WHITE_ROOK);
        setPiece(to, NONE);
    }

    moveNumber--;

    if (!noZobrist) {
        currentZobristKey = zobristHistory[moveNumber];
    }

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
    fiftyMoveHistory[0] = 0;
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

    moveNumber = 0;

    epMasks[0] = 0ULL;

    setWhiteCastleKingside(0, true);
    setWhiteCastleQueenside(0, true);
    setBlackCastleKingside(0, true);
    setBlackCastleQueenside(0, true);

    updateOccupancy();

    for (int square = 0; square < 64; square++) {
        mailbox[square] = NONE;
        for (int i = 0; i < 12; i++) {
            uint64_t bitboard = BITBOARDS[i];
            if (bitboard & 1ULL << square) {
                mailbox[square] = i;
            }
        }
    }
    currentZobristKey = Zobrist::calculateZobristKey(*this);
    zobristHistory[0] = currentZobristKey;
    zobristHistory[1] = currentZobristKey;
    whiteToMove = true;
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
    fiftyMoveHistory[0] = 0;
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

    whiteToMove = fen[index] == 'w';
    index += 2;

    moveNumber = 1;
    // Parse castling rights
    castleRights[moveNumber] = 0; // Reset castling rights
    while (fen[index] != ' ') {
        char c = fen[index];
        switch (c) {
            case 'K': setWhiteCastleKingside(moveNumber, true);
                break;
            case 'Q': setWhiteCastleQueenside(moveNumber, true);
                break;
            case 'k': setBlackCastleKingside(moveNumber, true);
                break;
            case 'q': setBlackCastleQueenside(moveNumber, true);
                break;
            case '-': break; // No castling rights
            default:
                throw std::invalid_argument("Invalid castling rights character: " + std::string(1, c));
        }
        index++;
    }

    // Update occupancy bitboards
    updateOccupancy();

    for (int square = 0; square < 64; square++) {
        mailbox[square] = NONE;
        for (int i = 0; i < 12; i++) {
            uint64_t bitboard = BITBOARDS[i];
            if (bitboard & 1ULL << square) {
                mailbox[square] = i;
            }
        }
    }
    currentZobristKey = Zobrist::calculateZobristKey(*this);
    zobristHistory[0] = currentZobristKey;
    zobristHistory[1] = currentZobristKey;
}

bool Board::canWhiteCastleKingside(uint32_t moveNumber) {
    return 1U & castleRights[moveNumber];
}

bool Board::canWhiteCastleQueenside(uint32_t moveNumber) {
    return 1U & (castleRights[moveNumber] >> 1);
}

bool Board::canBlackCastleKingside(uint32_t moveNumber) {
    return 1U & (castleRights[moveNumber] >> 2);
}

bool Board::canBlackCastleQueenside(uint32_t moveNumber) {
    return 1U & (castleRights[moveNumber] >> 3);
}

void Board::setWhiteCastleKingside(uint32_t moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= 1U; // Set bit 0
    } else {
        castleRights[moveNumber] &= ~1U; // Clear bit 0
    }
}

void Board::setWhiteCastleQueenside(uint32_t moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= (1U << 1); // Set bit 1
    } else {
        castleRights[moveNumber] &= ~(1U << 1); // Clear bit 1
    }
}

void Board::setBlackCastleKingside(uint32_t moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= (1U << 2); // Set bit 2
    } else {
        castleRights[moveNumber] &= ~(1U << 2); // Clear bit 2
    }
}

void Board::setBlackCastleQueenside(uint32_t moveNumber, bool value) {
    if (value) {
        castleRights[moveNumber] |= (1U << 3); // Set bit 3
    } else {
        castleRights[moveNumber] &= ~(1U << 3); // Clear bit 3
    }
}

bool Board::isDrawn() {
    if (moveNumber == 0)
        return false;
    if (fiftyMoveHistory[moveNumber] >= 100) {
        return true;
    }

    int count = 0;
    uint64_t currentKey = zobristHistory[moveNumber - 1]; // The Zobrist key of the current position
    for (int i = moveNumber - 1; i > 0; i--) {
        // Iterate through history in reverse
        if (zobristHistory[i] == currentKey) {
            count++;
            if (count >= 3) return true; // If the position has occurred at least 3 times, it's a draw
        }
    }
    return false;
}


std::string Board::generateFEN() {
    std::string fen = "";

    // Piece placement
    for (int rank = 7; rank >= 0; rank--) {
        int emptySquares = 0;
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int piece = mailbox[square];

            if (piece == NONE) {
                emptySquares++;
            } else {
                if (emptySquares > 0) {
                    fen += std::to_string(emptySquares);
                    emptySquares = 0;
                }

                switch (piece) {
                    case WHITE_PAWN: fen += 'P';
                        break;
                    case WHITE_KNIGHT: fen += 'N';
                        break;
                    case WHITE_BISHOP: fen += 'B';
                        break;
                    case WHITE_ROOK: fen += 'R';
                        break;
                    case WHITE_QUEEN: fen += 'Q';
                        break;
                    case WHITE_KING: fen += 'K';
                        break;
                    case BLACK_PAWN: fen += 'p';
                        break;
                    case BLACK_KNIGHT: fen += 'n';
                        break;
                    case BLACK_BISHOP: fen += 'b';
                        break;
                    case BLACK_ROOK: fen += 'r';
                        break;
                    case BLACK_QUEEN: fen += 'q';
                        break;
                    case BLACK_KING: fen += 'k';
                        break;
                }
            }
        }
        if (emptySquares > 0) fen += std::to_string(emptySquares);
        if (rank > 0) fen += '/';
    }

    // Active color
    fen += " ";
    fen += (whiteToMove ? 'w' : 'b');

    // Castling rights
    fen += " ";
    bool hasCastling = false;
    if (canWhiteCastleKingside(moveNumber)) {
        fen += 'K';
        hasCastling = true;
    }
    if (canWhiteCastleQueenside(moveNumber)) {
        fen += 'Q';
        hasCastling = true;
    }
    if (canBlackCastleKingside(moveNumber)) {
        fen += 'k';
        hasCastling = true;
    }
    if (canBlackCastleQueenside(moveNumber)) {
        fen += 'q';
        hasCastling = true;
    }
    if (!hasCastling) fen += "-";

    // En passant target
    fen += " -";

    return fen;
}

uint64_t Board::majorPieceBitboards(bool white) const {
    if (white) {
        return BITBOARDS[WHITE_KNIGHT] | BITBOARDS[WHITE_BISHOP] | BITBOARDS[WHITE_ROOK] | BITBOARDS[WHITE_QUEEN];
    }
    return BITBOARDS[BLACK_KNIGHT] | BITBOARDS[BLACK_BISHOP] | BITBOARDS[BLACK_ROOK] | BITBOARDS[BLACK_QUEEN];
}

uint64_t Board::minorPieceBitboards(bool white) const {
    return BITBOARDS[white ? WHITE_PAWN : BLACK_PAWN];
}
