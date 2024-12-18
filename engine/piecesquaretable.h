#pragma once

namespace PieceSquareTable {
    inline constexpr int PAWN_ENDGAME_TABLE[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10,
        20, 20, 20, 20, 20, 20, 20, 20,
        30, 30, 30, 30, 30, 30, 30, 30,
        50, 50, 50, 50, 50, 50, 50, 50,
        80, 80, 80, 80, 80, 80, 80, 80,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    inline constexpr int KING_ENDGAME_TABLE[64] = {
        -50, -30, -30, -30, -30, -30, -30, -50,
        -30, -25, 0, 0, 0, 0, -25, -30,
        -25, -20, 20, 25, 25, 20, -20, -25,
        -20, -15, 30, 40, 40, 30, -15, -20,
        -15, -10, 35, 45, 45, 35, -10, -15,
        -10, -5, 20, 30, 30, 20, -5, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    inline constexpr int PAWN_TABLE[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 0, 0, 50, 50, 50,
        10, 10, 20, 0, 0, 20, 10, 10,
        5, 5, 10, 25, 25, 10, 5, 5,
        0, 0, 0, 20, 20, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -20, -20, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    inline constexpr int ROOK_TABLE[64] = {
        0, 0, 0, 5, 5, 0, 0, 0,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        5, 10, 10, 10, 10, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0

    };

    inline constexpr int KNIGHT_TABLE[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50,
    };

    inline constexpr int BISHOP_TABLE[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20,
    };

    inline constexpr int QUEEN_TABLE[64] = {
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 5, 5, 5, 0, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        0, 0, 5, 5, 5, 5, 0, -5,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -20, -10, -10, -5, -5, -10, -10, -20
    };

    inline constexpr int KING_TABLE[64] = {
        20, 30, 10, 0, 0, 10, 30, 20,
        20, 20, -5, -5, -5, -5, 20, 20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -40, -50, -50, -60, -60, -50, -50, -40,
        -60, -60, -60, -60, -60, -60, -60, -60,
        -80, -70, -70, -70, -70, -70, -70, -80
    };


    inline int PIECE_SQUARE_TABLE[12][64] = {};
    inline int PIECE_SQUARE_TABLE_ENDGAME[12][64] = {};

    void initializePieceSquareTable();
}