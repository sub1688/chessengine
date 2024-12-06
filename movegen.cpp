#include "movegen.h"
#include <array>
#include <iostream>
#include <optional>
#include <random>

uint64_t Movegen::perft(int depth) {
    uint64_t perftCount = 0;

    // Generate all legal moves for the current position
    std::array<std::optional<Move>, 216> moves = generateAllLegalMovesOnBoard();

    for (const auto& move : moves) {
        if (!move.has_value())
            break; // Stop iterating when no more moves are available

        if (depth == 1) {
            perftCount++; // At depth 1, simply count the moves
        } else {
            // Make the move
            if (Board::move(move.value())) {
                // Recursively count moves at the next depth
                perftCount += perft(depth - 1);
                // Undo the move
                Board::undoMove(move.value());
            }
        }
    }

    return perftCount;
}

std::array<std::optional<Move>, 216> Movegen::generateAllLegalMovesOnBoard() {
    std::array<std::optional<Move>, 216> legalMoves;

    int arrayIndex = 0;
    bool whiteToMove = Board::whiteToMove;

    uint64_t currentOccupancy = whiteToMove ? Board::BITBOARD_WHITE_OCCUPANCY : Board::BITBOARD_BLACK_OCCUPANCY;
    while (currentOccupancy) {
        int pieceIndex = popLeastSignificantBitAndGetIndex(currentOccupancy);
        uint64_t pseudoLegalMoves = 0ULL;
        uint8_t piece = Board::getPiece(pieceIndex);
        switch (piece) {
            case BLACK_PAWN:
            case WHITE_PAWN:
                pseudoLegalMoves = generatePseudoLegalPawnMoves(pieceIndex, whiteToMove);
                break;
            case BLACK_KNIGHT:
            case WHITE_KNIGHT:
                pseudoLegalMoves = generatePseudoLegalKnightMoves(pieceIndex, whiteToMove);
                break;
            case WHITE_BISHOP:
            case BLACK_BISHOP:
                pseudoLegalMoves = generatePseudoLegalBishopMoves(pieceIndex, whiteToMove);
                break;
            case WHITE_ROOK:
            case BLACK_ROOK:
                pseudoLegalMoves = generatePseudoLegalRookMoves(pieceIndex, whiteToMove);
                break;
            case WHITE_KING:
            case BLACK_KING:
                pseudoLegalMoves = generatePseudoLegalKingMoves(pieceIndex, whiteToMove);
                break;
            case BLACK_QUEEN:
            case WHITE_QUEEN:
                pseudoLegalMoves = generatePseudoLegalQueenMoves(pieceIndex, whiteToMove);
                break;
            default:
                continue;
        }

        if (!pseudoLegalMoves)
            continue;

        while (pseudoLegalMoves) {
            uint8_t targetIndex = popLeastSignificantBitAndGetIndex(pseudoLegalMoves);
            Move move = Move(pieceIndex, targetIndex);
            move.capture = Board::getPiece(targetIndex);
            move.pieceFrom = piece;
            legalMoves[arrayIndex++] = move;
        }
    }

    return legalMoves;
}

bool Movegen::isKingInDanger(bool white) {
    uint8_t kingIndex = std::countr_zero(white ? Board::BITBOARDS[WHITE_KING] : Board::BITBOARDS[BLACK_KING]);
    uint8_t opponentOccupancy = white ? Board::BITBOARD_BLACK_OCCUPANCY : Board::BITBOARD_WHITE_OCCUPANCY;

    uint64_t availableMoves = generatePseudoLegalKingMoves(kingIndex, white) | generatePseudoLegalQueenMoves(kingIndex, white) | generatePseudoLegalKnightMoves(kingIndex, white);
    return availableMoves & opponentOccupancy;
}


uint64_t Movegen::generatePseudoLegalPawnMoves(uint8_t squareIndex, bool white) {
    uint64_t moves = 0ULL;
    uint64_t emptyBitboard = ~Board::BITBOARD_OCCUPANCY;
    uint64_t opponentBitboard = white ? Board::BITBOARD_BLACK_OCCUPANCY : Board::BITBOARD_WHITE_OCCUPANCY;

    if (white) {
        uint64_t singlePush = 1ULL << (squareIndex + 8) & emptyBitboard;
        moves |= singlePush;
        if (singlePush & RANK_3) {
            moves |= 1ULL << (squareIndex + 16) & emptyBitboard;
        }
        moves |= 1ULL << (squareIndex + 9) & opponentBitboard & NOT_FILE_A;
        moves |= 1ULL << (squareIndex + 7) & opponentBitboard & NOT_FILE_H;
    }else {
        uint64_t singlePush = 1ULL << (squareIndex - 8) & emptyBitboard;
        moves |= singlePush;
        if (singlePush & RANK_6) {
            moves |= 1ULL << (squareIndex - 16) & emptyBitboard;
        }
        moves |= 1ULL << (squareIndex - 9) & opponentBitboard & NOT_FILE_H;
        moves |= 1ULL << (squareIndex - 7) & opponentBitboard & NOT_FILE_A;
    }
    return moves;
}

uint64_t Movegen::generatePseudoLegalQueenMoves(uint8_t squareIndex, bool white) {
    return generatePseudoLegalBishopMoves(squareIndex, white) | generatePseudoLegalRookMoves(squareIndex, white);
}


uint64_t Movegen::generatePseudoLegalBishopMoves(uint8_t squareIndex, bool white) {
    uint64_t movementMask = BISHOP_MOVEMENT_MASKS[squareIndex];
    uint64_t blockers = movementMask & Board::BITBOARD_OCCUPANCY;
    uint64_t index = (blockers * BISHOP_MAGICS[squareIndex]) >> 52;
    return BISHOP_MOVE_TABLE[squareIndex][index] & (white ? ~Board::BITBOARD_WHITE_OCCUPANCY : ~Board::BITBOARD_BLACK_OCCUPANCY);
}

uint64_t Movegen::generatePseudoLegalRookMoves(uint8_t squareIndex, bool white) {
    uint64_t movementMask = ROOK_MOVEMENT_MASKS[squareIndex];
    uint64_t blockers = movementMask & Board::BITBOARD_OCCUPANCY;
    uint64_t index = (blockers * ROOK_MAGICS[squareIndex]) >> 52;
    return ROOK_MOVE_TABLE[squareIndex][index] & (white ? ~Board::BITBOARD_WHITE_OCCUPANCY : ~Board::BITBOARD_BLACK_OCCUPANCY);
}

uint64_t Movegen::generatePseudoLegalKingMoves(uint8_t squareIndex, bool white) {
    uint64_t movementMask = KING_MOVEMENT_MASKS[squareIndex];
    return movementMask & (white ? ~Board::BITBOARD_WHITE_OCCUPANCY : ~Board::BITBOARD_BLACK_OCCUPANCY);
}

uint64_t Movegen::generatePseudoLegalKnightMoves(uint8_t squareIndex, bool white) {
    uint64_t movementMask = KNIGHT_MOVEMENT_MASKS[squareIndex];
    return movementMask & (white ? ~Board::BITBOARD_WHITE_OCCUPANCY : ~Board::BITBOARD_BLACK_OCCUPANCY);
}

void Movegen::precomputeMovementMasks() {
    for (int i = 0; i < 64; i++) {
        ROOK_MOVEMENT_MASKS[i] = generateRookMovementMask(i);
        BISHOP_MOVEMENT_MASKS[i] = generateBishopMovementMask(i);
        KING_MOVEMENT_MASKS[i] = generateKingMovementMask(i);
        KNIGHT_MOVEMENT_MASKS[i] = generateKnightMovementMask(i);
    }
}

void Movegen::precomputeRookMovegenTable() {
    for (int i = 0; i < 64; i++) {
        uint64_t movementMask = generateRookMovementMask(i);
        for (uint64_t blocker : generateAllBlockers(movementMask)) {
            uint64_t index = blocker * ROOK_MAGICS[i] >> 52;
            ROOK_MOVE_TABLE[i][index] = precomputeRookMovesWithBlocker(i, blocker);
        }
    }
}

void Movegen::precomputeBishopMovegenTable() {
    for (int i = 0; i < 64; i++) {
        uint64_t movementMask = generateBishopMovementMask(i);
        for (uint64_t blocker : generateAllBlockers(movementMask)) {
            uint64_t index = blocker * BISHOP_MAGICS[i] >> 52;
            BISHOP_MOVE_TABLE[i][index] = precomputeBishopMovesWithBlocker(i, blocker);
        }
    }
}

uint64_t Movegen::precomputeBishopMovesWithBlocker(uint8_t squareIndex, uint64_t blocker) {
    uint64_t legalMoves = 0ULL;

    int rank = squareIndex / 8;
    int file = squareIndex % 8;

    // North-East (rank increases, file increases)
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f) {
        int targetSquare = r * 8 + f;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    // North-West (rank increases, file decreases)
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f) {
        int targetSquare = r * 8 + f;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    // South-East (rank decreases, file increases)
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f) {
        int targetSquare = r * 8 + f;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    // South-West (rank decreases, file decreases)
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
        int targetSquare = r * 8 + f;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    return legalMoves;
}


uint64_t Movegen::precomputeRookMovesWithBlocker(uint8_t squareIndex, uint64_t blocker) {
    uint64_t legalMoves = 0ULL;

    int rank = squareIndex / 8;
    int file = squareIndex % 8;

    // North (increasing rank)
    for (int r = rank + 1; r < 8; ++r) {
        int targetSquare = r * 8 + file;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    // South (decreasing rank)
    for (int r = rank - 1; r >= 0; --r) {
        int targetSquare = r * 8 + file;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    // East (increasing file)
    for (int f = file + 1; f < 8; ++f) {
        int targetSquare = rank * 8 + f;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    // West (decreasing file)
    for (int f = file - 1; f >= 0; --f) {
        int targetSquare = rank * 8 + f;
        legalMoves |= (1ULL << targetSquare);
        if (blocker & (1ULL << targetSquare)) break; // Stop at the first blocker
    }

    return legalMoves;
}


uint64_t Movegen::generateMagicNumber(uint8_t squareIndex, bool bishop) {
    uint64_t movementMask = bishop ? generateBishopMovementMask(squareIndex) : generateRookMovementMask(squareIndex);
    std::vector<uint64_t> blockers = generateAllBlockers(movementMask);
    uint64_t bestMagicNumber = 0ULL;

    // Try random magic numbers
    for (int attempts = 0; attempts < 1000000; ++attempts) {
        uint64_t candidateMagic = generateRandomMagic();
        std::array<uint8_t, 4096> occupied = {}; // Use uint8_t to avoid bit-packing
        bool valid = true;
        for (uint64_t blocker: blockers) {
            // Generate the index for this blocker
            uint64_t index = (blocker * candidateMagic) >> 52;
            if (index >= 4096) {
                valid = false;
                break;
            }
            // Check if the index is unique
            if (occupied[index]) {
                valid = false; // Duplicate index found
                break;
            }
            occupied[index] = true;
        }
        if (valid) {
            bestMagicNumber = candidateMagic;
        }
    }

    return bestMagicNumber;
}

std::vector<uint64_t> Movegen::generateAllBlockers(uint64_t movementMask) {
    std::vector<int> moveSquareIndices;
    for (int i = 0; i < 64; i++) {
        if (movementMask >> i & 1) {
            moveSquareIndices.push_back(i);
        }
    }

    int numPatterns = 1 << moveSquareIndices.size();
    std::vector<uint64_t> blockerBitboards(numPatterns, 0ULL);

    for (int patternIndex = 0; patternIndex < numPatterns; patternIndex++) {
        for (int bitIndex = 0; bitIndex < moveSquareIndices.size(); bitIndex++) {
            uint64_t bit = (patternIndex >> bitIndex) & 1ULL;
            blockerBitboards[patternIndex] |= bit << moveSquareIndices.at(bitIndex);
        }
    }

    return blockerBitboards;
}

uint64_t Movegen::generateRookMovementMask(uint8_t squareIndex) {
    uint64_t movementMask = 0ULL;

    // Calculate north, south, east, west rays
    int rank = squareIndex / 8;
    int file = squareIndex % 8;

    // North ray (rank increases)
    for (int r = rank + 1; r < 7; ++r) {
        // Stop before rank 8
        movementMask |= (1ULL << (r * 8 + file));
    }

    // South ray (rank decreases)
    for (int r = rank - 1; r > 0; --r) {
        // Stop before rank 1
        movementMask |= (1ULL << (r * 8 + file));
    }

    // East ray (file increases)
    for (int f = file + 1; f < 7; ++f) {
        // Stop before file H
        movementMask |= (1ULL << (rank * 8 + f));
    }

    // West ray (file decreases)
    for (int f = file - 1; f > 0; --f) {
        // Stop before file A
        movementMask |= (1ULL << (rank * 8 + f));
    }

    return movementMask;
}

uint64_t Movegen::generateBishopMovementMask(uint8_t squareIndex) {
    uint64_t movementMask = 0ULL;

    int rank = squareIndex / 8;
    int file = squareIndex % 8;

    // North-East (rank++, file++)
    for (int r = rank + 1, f = file + 1; r < 7 && f < 7; ++r, ++f) {
        movementMask |= (1ULL << (r * 8 + f));
    }

    // North-West (rank++, file--)
    for (int r = rank + 1, f = file - 1; r < 7 && f > 0; ++r, --f) {
        movementMask |= (1ULL << (r * 8 + f));
    }

    // South-East (rank--, file++)
    for (int r = rank - 1, f = file + 1; r > 0 && f < 7; --r, ++f) {
        movementMask |= (1ULL << (r * 8 + f));
    }

    // South-West (rank--, file--)
    for (int r = rank - 1, f = file - 1; r > 0 && f > 0; --r, --f) {
        movementMask |= (1ULL << (r * 8 + f));
    }

    return movementMask;
}
uint64_t Movegen::generateQueenMovementMask(uint8_t squareIndex) {
    return generateRookMovementMask(squareIndex) | generateBishopMovementMask(squareIndex);
}

uint64_t Movegen::generateKnightMovementMask(uint8_t squareIndex) {
    uint64_t knightMask = 0ULL;

    // Position bitboard for the given square
    uint64_t position = 1ULL << squareIndex;

    // Calculate possible knight moves
    knightMask |= (position << 17) & NOT_FILE_A; // Move up 2 and right 1
    knightMask |= (position << 15) & NOT_FILE_H; // Move up 2 and left 1
    knightMask |= (position << 10) & NOT_FILE_AB; // Move up 1 and right 2
    knightMask |= (position << 6) & NOT_FILE_GH; // Move up 1 and left 2
    knightMask |= (position >> 17) & NOT_FILE_H; // Move down 2 and left 1
    knightMask |= (position >> 15) & NOT_FILE_A; // Move down 2 and right 1
    knightMask |= (position >> 10) & NOT_FILE_GH; // Move down 1 and left 2
    knightMask |= (position >> 6) & NOT_FILE_AB; // Move down 1 and right 2

    return knightMask;
}

uint64_t Movegen::generateKingMovementMask(uint8_t squareIndex) {
    uint64_t kingMask = 0ULL;

    // Position bitboard for the given square
    uint64_t position = 1ULL << squareIndex;

    // Calculate possible king moves
    kingMask |= (position << 8); // Move up
    kingMask |= (position >> 8); // Move down
    kingMask |= (position << 1) & NOT_FILE_A; // Move right
    kingMask |= (position >> 1) & NOT_FILE_H; // Move left
    kingMask |= (position << 9) & NOT_FILE_A; // Move up-right
    kingMask |= (position << 7) & NOT_FILE_H; // Move up-left
    kingMask |= (position >> 7) & NOT_FILE_A; // Move down-right
    kingMask |= (position >> 9) & NOT_FILE_H; // Move down-left

    return kingMask;
}


void Movegen::printMovementMask(uint64_t movementMask) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int squareIndex = rank * 8 + file;
            if (movementMask & (1ULL << squareIndex)) {
                std::cout << "1  ";
            } else {
                std::cout << ".  ";
            }
        }
        std::cout << "\n"; // End of rank
    }
    std::cout << "\n";
}

uint64_t Movegen::generateRandomMagic() {
    return random_uint64() & random_uint64() & random_uint64();
}


uint64_t Movegen::random_uint64() {
    uint64_t u1, u2, u3, u4;
    u1 = random_uint64() & 0xFFFF;
    u2 = random_uint64() & 0xFFFF;
    u3 = random_uint64() & 0xFFFF;
    u4 = random_uint64() & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint8_t Movegen::popLeastSignificantBitAndGetIndex(uint64_t &b) {
    auto index = std::countr_zero(b);
    b &= b - 1;
    return index;
}
