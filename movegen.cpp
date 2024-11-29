#include "movegen.h"

#include <array>
#include <iostream>
#include <random>

uint64_t Movegen::generateLegalBishopMoves(uint8_t squareIndex, bool white) {
    uint64_t movementMask = generateBishopMovementMask(squareIndex);
    uint64_t blockers = movementMask & Board::BITBOARD_OCCUPANCY; // Mask blockers from the board occupancy
    uint64_t index = (blockers * BISHOP_MAGICS[squareIndex]) >> 52;
    return BISHOP_MOVE_TABLE[squareIndex][index] & (white ? ~Board::BITBOARD_WHITE_OCCUPANCY : ~Board::BITBOARD_BLACK_OCCUPANCY);
}

uint64_t Movegen::generateLegalRookMoves(uint8_t squareIndex, bool white) {
    uint64_t movementMask = generateRookMovementMask(squareIndex);
    uint64_t blockers = movementMask & Board::BITBOARD_OCCUPANCY; // Mask blockers from the board occupancy
    uint64_t index = (blockers * ROOK_MAGICS[squareIndex]) >> 52;
    return ROOK_MOVE_TABLE[squareIndex][index] & (white ? ~Board::BITBOARD_WHITE_OCCUPANCY : ~Board::BITBOARD_BLACK_OCCUPANCY);
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
    uint64_t mask = 0ULL;
    int rank = squareIndex / 8;
    int file = squareIndex % 8;

    // Generate rank mask: Shift the full rank to the correct position
    uint64_t rankMask = (RANK_1 << (rank * 8)) & ~(1ULL << squareIndex);

    // Generate file mask: Use FILE_A shifted to the correct file
    uint64_t fileMask = ((FILE_A << file) & ~FILE_A) & ~(1ULL << squareIndex);

    mask = rankMask | fileMask;
    return mask;
}
uint64_t Movegen::generateBishopMovementMask(uint8_t squareIndex) {
    uint64_t mask = 0ULL;
    uint64_t pos = (1ULL << squareIndex);

    // Compute diagonals
    uint64_t diagonalMask = ((pos * FILE_A) & ~FILE_A) | ((pos * FILE_H) & ~FILE_H);

    // Compute anti-diagonals
    uint64_t antiDiagonalMask = ((pos & ~FILE_A) / FILE_A) | ((pos & ~FILE_H) / FILE_H);

    mask = (diagonalMask | antiDiagonalMask) & ~(1ULL << squareIndex);
    return mask;
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
    u1 = (uint64_t) (random()) & 0xFFFF;
    u2 = (uint64_t) (random()) & 0xFFFF;
    u3 = (uint64_t) (random()) & 0xFFFF;
    u4 = (uint64_t) (random()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}
