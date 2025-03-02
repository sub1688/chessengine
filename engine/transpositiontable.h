#pragma once

#include <cassert>
#include <cstdint>
#include "board.h"

#define TRANSPOSITION_TABLE_LOOKUP_SUCCESS 1
#define TRANSPOSITION_TABLE_LOOKUP_FAILURE 0

#define UPPER_BOUND 2
#define EXACT_BOUND 1
#define LOWER_BOUND 0

#define EXTRACT_BEST_MOVE_BITS(x) (x & 0xFFFFFFF)
#define EXTRACT_DEPTH_SEARCHED(x) ((x >> 28) & 0xFF)
#define EXTRACT_SCORE(x) (((x >> 36) & 0xFFFF) - 30000)
#define EXTRACT_NODE_TYPE(x) ((x >> 52) & 0b11)

/**
 *  TRANSPOSITION ENTRY:
 *  Zobrist Key: 64 bits
 *  data:
 *      Move: 28 bits
 *      depth: 8 bits
 *      score: 16 bits
 *      node type: 2 bits
 *
 *      Total: 54 bits
 *
 *  Total: 118 bits used
 */
struct TranspositionEntry {
    uint64_t zobristKey;
    uint64_t data;

    TranspositionEntry() : zobristKey(0), data(0) {}
    TranspositionEntry(uint64_t m_zobristKey, Move m_bestMove, uint8_t m_depthSearched, int m_score, uint8_t m_nodeType) {
        data = getBitsFromData(m_bestMove, m_depthSearched, m_score, m_nodeType);
        zobristKey = m_zobristKey ^ data;
    }

    [[nodiscard]] uint64_t getBitsFromData(Move bestMove, uint8_t depthSearched, int score, uint8_t nodeType) const {
        return bestMove.getMoveBits() |
           static_cast<uint64_t>(depthSearched) << 28ULL |
           static_cast<uint64_t>(score + 30000) << 36ULL |
           static_cast<uint64_t>(nodeType) << 52ULL;
    }
};

class TranspositionTable {
public:
    static constexpr size_t TRANSPOSITION_TABLE_SIZE = 1 << 25;
    static constexpr size_t TRANSPOSITION_TABLE_MASK = TRANSPOSITION_TABLE_SIZE - 1;

    static constexpr size_t KILLER_DEPTHS = 256;
    static constexpr size_t KILLERS_PER_PLY = 2;

    TranspositionEntry transpositionTableBuffer[TRANSPOSITION_TABLE_SIZE];
    Move killerMoves[KILLER_DEPTHS][KILLERS_PER_PLY];

    int tableEntries = 0;
    int cutoffs = 0;

    int tableLookup(uint64_t zobristKey, TranspositionEntry &out);

    int correctScoreForStorage(int score, int rootDepth);
    int correctScoreForRetrieval(int score, int rootDepth);

    void addEntry(uint64_t zobristKey, Move bestMove, int rootDepth, int depthSearched, int score, int nodeType);
    void clear();

    void storeKillerMove(Move move, int depth);
};
