#include "movegen.h"
#include <array>
#include <iostream>
#include <random>
#include <bits/ranges_algobase.h>

#include "san.h"
#include "zobrist.h"

bool Movegen::isMoveCheck(Board& board, Move move) {
    bool white = move.pieceFrom < 6;
    if (board.move(move))
    {
        bool check = isKingInDanger(board, !white);
        board.undoMove(move);
        return check;
    }
    return false;
}


uint64_t Movegen::perft(Board &board, int depth) {
    uint64_t perftCount = 0;

    // Generate all legal moves for the current position
    ArrayVec<Move, 218> moves = generateAllLegalMovesOnBoard(board);

    for (int i = 0; i < moves.elements; i++) {
        Move move = moves.buffer.at(i);
        if (board.move(move)) {
            if (depth == 1) {
                perftCount++; // At depth 1, count the legal move
            } else {
                // Recursively count moves at the next depth
                perftCount += perft(board, depth - 1);
            }
            // Undo the move to restore the board state
            board.undoMove(move);
        }
    }

    return perftCount;
}

ArrayVec<Move, 218> Movegen::generateAllLegalMovesOnBoard(Board &board, bool capturesOnly) {
    bool whiteToMove = board.whiteToMove;
    ArrayVec<Move, 218> legalMoves(0);
    for (int i = whiteToMove ? 0 : 6; i < (whiteToMove ? 6 : 12); i++) {
        uint64_t pieceBitboard = board.BITBOARDS[i];
        while (pieceBitboard) {
            uint8_t index = popLeastSignificantBitAndGetIndex(pieceBitboard);
            generatePseudoLegalMoves(board, index, i, whiteToMove, capturesOnly, legalMoves);
        }
    }
    return legalMoves;
}

ArrayVec<Move, 218> Movegen::generateAllLegalMovesOnBoard(Board &board) {
    return generateAllLegalMovesOnBoard(board, false);
}

void Movegen::generatePseudoLegalMoves(Board &board, uint8_t squareIndex, uint8_t piece, bool white, bool capturesOnly,
                                       ArrayVec<Move, 218> &movesVec) {
    uint64_t moves = 0ULL;
    uint64_t castleMoves = 0ULL;
    uint64_t enPassantMove = 0ULL;
    bool promotion = false;

    switch (piece % 6) {
        case WHITE_PAWN:
            moves = generatePseudoLegalPawnMoves(board, squareIndex, white);
            enPassantMove = generatePseudoLegalEnPassantMoves(board, squareIndex, white);
            promotion = moves & (white ? RANK_8 : RANK_1);
            break;
        case WHITE_KNIGHT:
            moves = generatePseudoLegalKnightMoves(board, squareIndex, white);
            break;
        case WHITE_BISHOP:
            moves = generatePseudoLegalBishopMoves(board, squareIndex, white);
            break;
        case WHITE_ROOK:
            moves = generatePseudoLegalRookMoves(board, squareIndex, white);
            break;
        case WHITE_QUEEN:
            moves = generatePseudoLegalQueenMoves(board, squareIndex, white);
            break;
        case WHITE_KING:
            moves = generatePseudoLegalKingMoves(board, squareIndex, white);
            castleMoves = generatePseudoLegalCastleMoves(board, white);
            break;
        default: break;
    }

    if (capturesOnly) {
        uint64_t opponentBitboard = white ? board.BITBOARD_BLACK_OCCUPANCY : board.BITBOARD_WHITE_OCCUPANCY;
        moves &= opponentBitboard;
    } else {
        while (castleMoves) {
            uint8_t targetIndex = popLeastSignificantBitAndGetIndex(castleMoves);

            Move moveObj = Move(squareIndex, targetIndex);
            moveObj.pieceFrom = piece;
            moveObj.capture = NONE;
            moveObj.castle = true;
            movesVec.buffer[movesVec.elements++] = moveObj;
        }
    }

    if (promotion) {
        while (moves) {
            uint8_t targetIndex = popLeastSignificantBitAndGetIndex(moves);
            for (uint8_t promotedPiece: white ? PROMOTE_PIECES_WHITE : PROMOTE_PIECES_BLACK) {
                Move moveObj = Move(squareIndex, targetIndex);
                moveObj.pieceFrom = piece;
                moveObj.capture = board.getPiece(targetIndex);
                moveObj.promotion = promotedPiece;
                movesVec.buffer[movesVec.elements++] = moveObj;
            }
        }
    } else {
        while (moves) {
            uint8_t targetIndex = popLeastSignificantBitAndGetIndex(moves);
            Move moveObj = Move(squareIndex, targetIndex);
            moveObj.pieceFrom = piece;
            moveObj.capture = board.getPiece(targetIndex);
            movesVec.buffer[movesVec.elements++] = moveObj;
        }
    }

    if (enPassantMove) {
        uint8_t targetIndex = popLeastSignificantBitAndGetIndex(enPassantMove);

        Move moveObj = Move(squareIndex, targetIndex);
        moveObj.pieceFrom = piece;
        moveObj.capture = white ? BLACK_PAWN : WHITE_PAWN   ;
        moveObj.enPassantTarget = piece == WHITE_PAWN ? targetIndex - 8 : targetIndex + 8;
        movesVec.buffer[movesVec.elements++] = moveObj;
    }
}


bool Movegen::isKingInDanger(Board &board, bool white) {
    uint8_t kingIndex = __builtin_ctzll(white ? board.BITBOARDS[WHITE_KING] : board.BITBOARDS[BLACK_KING]);
    return isSquareAttacked(board, kingIndex, white);
}

bool Movegen::isSquareAttacked(Board &board, uint8_t kingIndex, bool white) {
    // 1. Check for pawn attacks
    uint64_t opposingPawnBitboard = white ? board.BITBOARDS[BLACK_PAWN] : board.BITBOARDS[WHITE_PAWN];
    if (PAWN_ATTACK_MASKS[!white][kingIndex] & opposingPawnBitboard)
        return true;

    // 2. Check for knight attacks
    uint64_t knightMoves = KNIGHT_MOVEMENT_MASKS[kingIndex];
    uint64_t opposingKnightBitboard = white ? board.BITBOARDS[BLACK_KNIGHT] : board.BITBOARDS[WHITE_KNIGHT];
    if (knightMoves & opposingKnightBitboard)
        return true;

    // 3. Check for king attacks
    uint64_t kingMoves = KING_MOVEMENT_MASKS[kingIndex];
    uint64_t opposingKingBitboard = white ? board.BITBOARDS[BLACK_KING] : board.BITBOARDS[WHITE_KING];
    if (kingMoves & opposingKingBitboard)
        return true;

    // 4. Check for bishop/queen diagonal attacks
    uint64_t bishopMoves = generatePseudoLegalBishopMoves(board, kingIndex, white);
    uint64_t opposingBishopBitboard = white ? board.BITBOARDS[BLACK_BISHOP] : board.BITBOARDS[WHITE_BISHOP];
    uint64_t opposingQueenBitboard = white ? board.BITBOARDS[BLACK_QUEEN] : board.BITBOARDS[WHITE_QUEEN];
    if (bishopMoves & (opposingBishopBitboard | opposingQueenBitboard))
        return true;

    // 5. Check for rook/queen straight attacks
    uint64_t rookMoves = generatePseudoLegalRookMoves(board, kingIndex, white);
    uint64_t opposingRookBitboard = white ? board.BITBOARDS[BLACK_ROOK] : board.BITBOARDS[WHITE_ROOK];
    if (rookMoves & (opposingRookBitboard | opposingQueenBitboard))
        return true;

    // If no attacks are found
    return false;
}


uint64_t Movegen::generatePawnMovementMask(uint8_t squareIndex, bool white) {
    uint64_t moves = 0ULL;
    if (white) {
        uint64_t singlePush = 1ULL << (squareIndex + 8);
        moves |= singlePush;
        if (singlePush & RANK_3) {
            moves |= 1ULL << (squareIndex + 16);
        }
    } else {
        uint64_t singlePush = 1ULL << (squareIndex - 8);
        moves |= singlePush;
        if (singlePush & RANK_6) {
            moves |= 1ULL << (squareIndex - 16);
        }
    }
    return moves;
}

uint64_t Movegen::generatePawnAttackMask(uint8_t squareIndex, bool white) {
    uint64_t moves = 0ULL;
    if (white) {
        moves |= 1ULL << (squareIndex + 9) & NOT_FILE_A;
        moves |= 1ULL << (squareIndex + 7) & NOT_FILE_H;
    } else {
        moves |= 1ULL << (squareIndex - 9) & NOT_FILE_H;
        moves |= 1ULL << (squareIndex - 7) & NOT_FILE_A;
    }
    return moves;
}


uint64_t Movegen::generatePseudoLegalQueenMoves(Board &board, uint8_t squareIndex, bool white) {
    return generatePseudoLegalBishopMoves(board, squareIndex, white) | generatePseudoLegalRookMoves(
               board, squareIndex, white);
}


uint64_t Movegen::generatePseudoLegalBishopMoves(Board &board, uint8_t squareIndex, bool white) {
    uint64_t movementMask = BISHOP_MOVEMENT_MASKS[squareIndex];
    uint64_t blockers = movementMask & board.BITBOARD_OCCUPANCY;
    uint64_t index = (blockers * BISHOP_MAGICS[squareIndex]) >> 52;
    return BISHOP_MOVE_TABLE[squareIndex][index] & (white
                                                        ? ~board.BITBOARD_WHITE_OCCUPANCY
                                                        : ~board.BITBOARD_BLACK_OCCUPANCY);
}

uint64_t Movegen::generatePseudoLegalRookMoves(Board &board, uint8_t squareIndex, bool white) {
    uint64_t movementMask = ROOK_MOVEMENT_MASKS[squareIndex];
    uint64_t blockers = movementMask & board.BITBOARD_OCCUPANCY;
    uint64_t index = (blockers * ROOK_MAGICS[squareIndex]) >> 52;
    return ROOK_MOVE_TABLE[squareIndex][index] & (white
                                                      ? ~board.BITBOARD_WHITE_OCCUPANCY
                                                      : ~board.BITBOARD_BLACK_OCCUPANCY);
}

uint64_t Movegen::generatePseudoLegalKingMoves(Board &board, uint8_t squareIndex, bool white) {
    uint64_t movementMask = KING_MOVEMENT_MASKS[squareIndex];
    return movementMask & (white ? ~board.BITBOARD_WHITE_OCCUPANCY : ~board.BITBOARD_BLACK_OCCUPANCY);
}

uint64_t Movegen::generatePseudoLegalCastleMoves(Board &board, bool white) {
    uint64_t castleMoves = 0ULL;

    if (isKingInDanger(board, white))
        return 0ULL;

    if (white) {
        if (0x80ULL & board.BITBOARDS[WHITE_ROOK] && board.canWhiteCastleKingside(board.moveNumber) &&
            !(board.BITBOARD_OCCUPANCY & 0x60ULL) &&
            !isSquareAttacked(board, 5, true)) {
            castleMoves |= 0x40ULL;
        }

        if (0x1ULL & board.BITBOARDS[WHITE_ROOK] && board.canWhiteCastleQueenside(board.moveNumber) &&
            !(board.BITBOARD_OCCUPANCY & 0xEULL) &&
            !isSquareAttacked(board, 3, true)) {
            castleMoves |= 0x4ULL;
        }
    } else {
        if (0x8000000000000000ULL & board.BITBOARDS[BLACK_ROOK] && board.canBlackCastleKingside(board.moveNumber) &&
            !(board.BITBOARD_OCCUPANCY & 0x6000000000000000ULL) &&
            !isSquareAttacked(board, 61, false)) {
            castleMoves |= 0x4000000000000000ULL;
        }

        if (0x100000000000000ULL & board.BITBOARDS[BLACK_ROOK] && board.canBlackCastleQueenside(board.moveNumber) &&
            !(board.BITBOARD_OCCUPANCY & 0xE00000000000000ULL) &&
            !isSquareAttacked(board, 59, false)) {
            castleMoves |= 0x400000000000000ULL;
        }
    }

    return castleMoves;
}

uint64_t Movegen::generatePseudoLegalPawnMoves(Board &board, uint8_t squareIndex, bool white) {
    uint64_t movementMask = PAWN_MOVEMENT_MASKS[!white][squareIndex];
    uint64_t attackMask = PAWN_ATTACK_MASKS[!white][squareIndex];
    uint64_t opponentBitboard = white ? board.BITBOARD_BLACK_OCCUPANCY : board.BITBOARD_WHITE_OCCUPANCY;
    uint64_t resultMask = 0;

    if (1ULL << squareIndex + (white ? 8 : -8) & ~board.BITBOARD_OCCUPANCY) {
        resultMask |= movementMask & ~board.BITBOARD_OCCUPANCY;
    }

    return resultMask | attackMask & opponentBitboard;
}

uint64_t Movegen::generatePseudoLegalEnPassantMoves(Board &board, uint8_t squareIndex, bool white) {
    return PAWN_ATTACK_MASKS[!white][squareIndex] & board.epMasks[board.moveNumber];
}

uint64_t Movegen::generatePseudoLegalKnightMoves(Board &board, uint8_t squareIndex, bool white) {
    uint64_t movementMask = KNIGHT_MOVEMENT_MASKS[squareIndex];
    return movementMask & (white ? ~board.BITBOARD_WHITE_OCCUPANCY : ~board.BITBOARD_BLACK_OCCUPANCY);
}

bool Movegen::inCheckmate(Board &board) {
    if (!isKingInDanger(board, board.whiteToMove))
        return false;

    ArrayVec<Move, 218> moves = generateAllLegalMovesOnBoard(board);
    for (int i = 0; i < moves.elements; i++) {
        if (board.move(moves.buffer[i])) {
            board.undoMove(moves.buffer[i]);
            return false;
        }
    }
    return true;
}


bool Movegen::inStalemate(Board &board) {
    ArrayVec<Move, 218> moves = generateAllLegalMovesOnBoard(board);
    for (int i = 0; i < moves.elements; i++) {
        if (board.move(moves.buffer[i])) {
            board.undoMove(moves.buffer[i]);
            return false;
        }
    }
    return !isKingInDanger(board, board.whiteToMove);
}

void Movegen::precomputeMovementMasks() {
    for (int i = 0; i < 64; i++) {
        ROOK_MOVEMENT_MASKS[i] = generateRookMovementMask(i);
        BISHOP_MOVEMENT_MASKS[i] = generateBishopMovementMask(i);
        KING_MOVEMENT_MASKS[i] = generateKingMovementMask(i);
        KNIGHT_MOVEMENT_MASKS[i] = generateKnightMovementMask(i);
        PAWN_MOVEMENT_MASKS[0][i] = generatePawnMovementMask(i, true);
        PAWN_MOVEMENT_MASKS[1][i] = generatePawnMovementMask(i, false);
        PAWN_ATTACK_MASKS[0][i] = generatePawnAttackMask(i, true);
        PAWN_ATTACK_MASKS[1][i] = generatePawnAttackMask(i, false);
    }
}

void Movegen::precomputeRookMovegenTable() {
    for (int i = 0; i < 64; i++) {
        uint64_t movementMask = generateRookMovementMask(i);
        for (uint64_t blocker: generateAllBlockers(movementMask)) {
            uint64_t index = blocker * ROOK_MAGICS[i] >> 52;
            ROOK_MOVE_TABLE[i][index] = precomputeRookMovesWithBlocker(i, blocker);
        }
    }
}

void Movegen::precomputeBishopMovegenTable() {
    for (int i = 0; i < 64; i++) {
        uint64_t movementMask = generateBishopMovementMask(i);
        for (uint64_t blocker: generateAllBlockers(movementMask)) {
            uint64_t index = blocker * BISHOP_MAGICS[i] >> 52;
            BISHOP_MOVE_TABLE[i][index] = precomputeBishopMovesWithBlocker(i, blocker);
        }
    }
}

uint64_t Movegen::precomputeBishopMovesWithBlocker(uint8_t squareIndex, uint64_t blocker) {
    uint64_t legalMoves = 0ULL;

    int rank = squareIndex / 8;
    int file = squareIndex & 7;

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
    int file = squareIndex & 7;

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
    int file = squareIndex & 7;

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
    int file = squareIndex & 7;
    // & 7 is used to replace mod 8 because it is slightly faster as it doesnt require division. This trick only works for integers that are a power of 2 tho.a

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


void Movegen::init() {
    precomputeMovementMasks();
    precomputeRookMovegenTable();
    precomputeBishopMovegenTable();
}
