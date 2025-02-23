#pragma once
#include <optional>
#include <vector>

#include "board.h"
#include "../util/arrayvec.h"

#define MAGIC_SHIFT = 52

namespace Movegen {
    constexpr uint64_t FILE_A = 0x0101010101010101ULL;
    constexpr uint64_t FILE_H = 0x8080808080808080ULL;
    constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
    constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
    constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
    constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;
    constexpr uint64_t NOT_FILE_H = 0x7F7F7F7F7F7F7F7FULL;
    constexpr uint64_t NOT_FILE_A = 0xFEFEFEFEFEFEFEFEULL;
    constexpr uint64_t NOT_FILE_AB = 0xFCFCFCFCFCFCFCFCULL;
    constexpr uint64_t NOT_FILE_GH = 0x3F3F3F3F3F3F3F3FULL;

    inline constexpr uint8_t PROMOTE_PIECES_WHITE[4] = {
        WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN
    };

    inline constexpr uint8_t PROMOTE_PIECES_BLACK[4] = {
        BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN
    };

    inline constexpr uint64_t ROOK_MAGICS[64] = {
        0x480001080c00320, 0x208008810c00143, 0x410006000906500, 0x20162410002004, 0xa200010210200a00,
        0x804040002984010, 0x1100210020441200, 0x2100008141000422, 0x80900810a0800400, 0x20424024204000,
        0x1000440010202420, 0x92000200420420, 0x400412070480c, 0x10021090022820, 0x1890041041000820, 0x2000400481500664,
        0x300524000208040, 0x2a4082688042448, 0x4200050008c0044, 0x8000020201c05028, 0x2430429800400500,
        0xcc04002802000150, 0x20d5011014004020, 0x1700800021c100, 0x3801200040024050, 0x408320410000840,
        0xa80500080082000, 0x20020040820, 0x4200400801821800, 0x10008016b0980402, 0x8060504410080, 0x2040110062008,
        0x9082041020400040, 0x1000800982080041, 0x100a002a0020, 0x20020004808802, 0x1980c0880400209, 0xc08110200109,
        0x84004c0220800809, 0x190220100400c8, 0x312200010404000, 0xc0445542b0008120, 0x4002000880202100,
        0xb000a00200209000, 0x5110012011d00282, 0x128040002021049, 0x90d4110220, 0xac00883002100, 0x400402411800080,
        0x24400400200c084, 0x1810100180210410, 0x400a00208ca0, 0xc004000422004842, 0x21800802090180, 0x502022000442080,
        0x402006810140, 0x8000c100241281, 0x4002a8410204001, 0x120e00201024806a, 0x20104014080202, 0x1202004c20111042,
        0x8001009410080132, 0x48002002c084081, 0x346008145140122
    };

    inline constexpr uint64_t BISHOP_MAGICS[64] = {
        0x2200104c0300240, 0x2028804400a00040, 0x9040800a01441000, 0x1004030100240004, 0x2000020260000081,
        0x24020901120c0, 0x8a204105400a000, 0x400130009004200, 0x9000418060404300, 0x866110a18, 0x41020c040080f0,
        0x804041a002000260, 0x48000ac00803098, 0x10200803100300, 0x40100400308020a, 0xc080004608002801,
        0x1000800202621900, 0x101008021020150, 0xa4a4012200808, 0x2008a01004021030, 0x40624090214, 0x800428044008a00,
        0x12003023805484, 0x2014c0020500800, 0xc80004092081091a, 0x1118114232401022, 0xa05100c20020821,
        0x80810002802420, 0x401010c000020085, 0x8000c8102020280, 0x20434424008c2040, 0x8889600249010880,
        0x1000088008021086, 0xc08100902404004, 0x80634492004001, 0x2388204804000812, 0x1000800490130080,
        0x24008008000400, 0x3001020a1404084, 0x10000420481a2100, 0x31080082200020, 0x2302008108010200,
        0x22081038804d000, 0x401102106100108, 0x40014805204182, 0x10100612500060, 0x2200200400800025, 0x6008481036448,
        0x2080540a8008, 0x161110a220842081, 0x500100400880000, 0x8000a02800618, 0x10d0001000088004, 0x8000400440028800,
        0x404c20888000, 0x8000208104c00, 0x13084800801040, 0x4340004200061220, 0xa00502041c900, 0x12008084,
        0x80041304008, 0x8800040820001105, 0x4406582001208920, 0x832081055b800200,
    };

    inline uint64_t ROOK_MOVEMENT_MASKS[64];
    inline uint64_t BISHOP_MOVEMENT_MASKS[64];
    inline uint64_t KNIGHT_MOVEMENT_MASKS[64];
    inline uint64_t KING_MOVEMENT_MASKS[64];
    inline uint64_t PAWN_MOVEMENT_MASKS[2][64];
    inline uint64_t PAWN_ATTACK_MASKS[2][64];

    inline uint64_t ROOK_MOVE_TABLE[64][4096];
    inline uint64_t BISHOP_MOVE_TABLE[64][4096];

    bool inCheckmate(Board &board);

    uint64_t perft(Board &board, int depth);

    void generatePseudoLegalMoves(Board& board, uint8_t squareIndex, uint8_t piece, bool white, bool capturesOnly, ArrayVec<Move, 218> &movesVec);

    uint64_t generatePseudoLegalBishopMoves(Board &board, uint8_t squareIndex, bool white);

    uint64_t generatePseudoLegalRookMoves(Board &board, uint8_t squareIndex, bool white);

    uint64_t generatePseudoLegalQueenMoves(Board &board, uint8_t squareIndex, bool white);

    uint64_t generatePseudoLegalKingMoves(Board &board, uint8_t squareIndex, bool white);

    uint64_t generatePseudoLegalKnightMoves(Board &board, uint8_t squareIndex, bool white);

    uint64_t generatePseudoLegalPawnMoves(Board &board, uint8_t squareIndex, bool white);

    uint64_t generatePseudoLegalEnPassantMoves(Board &board, uint8_t squareIndex, bool white);

    uint64_t generatePseudoLegalCastleMoves(Board &board, bool white);

    uint64_t random_uint64();

    uint64_t generateRandomMagic();

    uint64_t generateMagicNumber(uint8_t squareIndex, bool bishop);

    uint64_t generatePawnMovementMask(uint8_t squareIndex, bool white);

    uint64_t generatePawnAttackMask(uint8_t squareIndex, bool white);

    uint64_t generateRookMovementMask(uint8_t squareIndex);

    uint64_t generateBishopMovementMask(uint8_t squareIndex);

    uint64_t generateQueenMovementMask(uint8_t squareIndex);

    uint64_t generateKnightMovementMask(uint8_t squareIndex);

    uint64_t generateKingMovementMask(uint8_t squareIndex);

    std::vector<uint64_t> generateAllBlockers(uint64_t movementMask);

    uint64_t precomputeRookMovesWithBlocker(uint8_t squareIndex, uint64_t blocker);

    void precomputeMovementMasks();

    void precomputeRookMovegenTable();

    uint64_t precomputeBishopMovesWithBlocker(uint8_t squareIndex, uint64_t blocker);

    void precomputeBishopMovegenTable();

    void printMovementMask(uint64_t mask);

    uint8_t popLeastSignificantBitAndGetIndex(uint64_t &num);

    ArrayVec<Move, 218> generateAllLegalMovesOnBoard(Board& board);

    ArrayVec<Move, 218> generateAllLegalMovesOnBoard(Board& board, bool capturesOnly);

    bool isSquareAttacked(Board &board, uint8_t kingIndex, bool white);

    bool isKingInDanger(Board &board, bool white);

    void init();

    bool inStalemate(Board &board);
}
