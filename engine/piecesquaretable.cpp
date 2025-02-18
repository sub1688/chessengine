#include "piecesquaretable.h"
#include <algorithm>
#include "board.h"

void PieceSquareTable::initializePieceSquareTable() {
    auto flipVertically = [](const int table[64], int flippedTable[64]) {
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                flippedTable[(7 - row) * 8 + col] = table[row * 8 + col];
            }
        }
    };

    // Fill white piece tables
    std::copy(std::begin(PAWN_TABLE), std::end(PAWN_TABLE), PIECE_SQUARE_TABLE[WHITE_PAWN]);
    std::copy(std::begin(KNIGHT_TABLE), std::end(KNIGHT_TABLE), PIECE_SQUARE_TABLE[WHITE_KNIGHT]);
    std::copy(std::begin(BISHOP_TABLE), std::end(BISHOP_TABLE), PIECE_SQUARE_TABLE[WHITE_BISHOP]);
    std::copy(std::begin(QUEEN_TABLE), std::end(QUEEN_TABLE), PIECE_SQUARE_TABLE[WHITE_QUEEN]);
    std::copy(std::begin(KING_TABLE), std::end(KING_TABLE), PIECE_SQUARE_TABLE[WHITE_KING]);
    std::copy(std::begin(ROOK_TABLE), std::end(ROOK_TABLE), PIECE_SQUARE_TABLE[WHITE_ROOK]);

    // Flip for black pieces
    int flippedTable[64];
    flipVertically(PAWN_TABLE, flippedTable);
    std::copy(std::begin(flippedTable), std::end(flippedTable), PIECE_SQUARE_TABLE[BLACK_PAWN]);

    flipVertically(KNIGHT_TABLE, flippedTable);
    std::copy(std::begin(flippedTable), std::end(flippedTable), PIECE_SQUARE_TABLE[BLACK_KNIGHT]);

    flipVertically(BISHOP_TABLE, flippedTable);
    std::copy(std::begin(flippedTable), std::end(flippedTable), PIECE_SQUARE_TABLE[BLACK_BISHOP]);

    flipVertically(QUEEN_TABLE, flippedTable);
    std::copy(std::begin(flippedTable), std::end(flippedTable), PIECE_SQUARE_TABLE[BLACK_QUEEN]);

    flipVertically(KING_TABLE, flippedTable);
    std::copy(std::begin(flippedTable), std::end(flippedTable), PIECE_SQUARE_TABLE[BLACK_KING]);

    flipVertically(ROOK_TABLE, flippedTable);
    std::copy(std::begin(flippedTable), std::end(flippedTable), PIECE_SQUARE_TABLE[BLACK_ROOK]);

    // Fill white piece tables
    std::copy(std::begin(PAWN_ENDGAME_TABLE), std::end(PAWN_ENDGAME_TABLE), PIECE_SQUARE_TABLE_ENDGAME[WHITE_PAWN]);
    std::copy(std::begin(KNIGHT_TABLE), std::end(KNIGHT_TABLE), PIECE_SQUARE_TABLE_ENDGAME[WHITE_KNIGHT]);
    std::copy(std::begin(BISHOP_TABLE), std::end(BISHOP_TABLE), PIECE_SQUARE_TABLE_ENDGAME[WHITE_BISHOP]);
    std::copy(std::begin(QUEEN_TABLE), std::end(QUEEN_TABLE), PIECE_SQUARE_TABLE_ENDGAME[WHITE_QUEEN]);
    std::copy(std::begin(KING_ENDGAME_TABLE), std::end(KING_ENDGAME_TABLE), PIECE_SQUARE_TABLE_ENDGAME[WHITE_KING]);
    std::copy(std::begin(ROOK_TABLE), std::end(ROOK_TABLE), PIECE_SQUARE_TABLE_ENDGAME[WHITE_ROOK]);

    // Flip for black pieces
    int flippedEndgameTable[64];
    flipVertically(PAWN_ENDGAME_TABLE, flippedEndgameTable);
    std::copy(std::begin(flippedEndgameTable), std::end(flippedEndgameTable), PIECE_SQUARE_TABLE_ENDGAME[BLACK_PAWN]);

    flipVertically(KNIGHT_TABLE, flippedEndgameTable);
    std::copy(std::begin(flippedEndgameTable), std::end(flippedEndgameTable), PIECE_SQUARE_TABLE_ENDGAME[BLACK_KNIGHT]);

    flipVertically(BISHOP_TABLE, flippedEndgameTable);
    std::copy(std::begin(flippedEndgameTable), std::end(flippedEndgameTable), PIECE_SQUARE_TABLE_ENDGAME[BLACK_BISHOP]);

    flipVertically(QUEEN_TABLE, flippedEndgameTable);
    std::copy(std::begin(flippedEndgameTable), std::end(flippedEndgameTable), PIECE_SQUARE_TABLE_ENDGAME[BLACK_QUEEN]);

    flipVertically(KING_ENDGAME_TABLE, flippedEndgameTable);
    std::copy(std::begin(flippedEndgameTable), std::end(flippedEndgameTable), PIECE_SQUARE_TABLE_ENDGAME[BLACK_KING]);

}

