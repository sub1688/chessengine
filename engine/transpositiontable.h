#pragma once

#include <cstdint>
#include "board.h"

#define UPPER_BOUND 1
#define EXACT_BOUND 0
#define LOWER_BOUND (-1)

struct TranspositionEntry {
    uint64_t zobristKey;
    Move bestMove;
    uint8_t depthSearched;
    uint8_t nodeType;
    int score;

    TranspositionEntry() : zobristKey(0), bestMove(Move()), depthSearched(0), score(0), nodeType(0) {}
    TranspositionEntry(uint64_t m_zobristKey, Move m_bestMove, uint8_t m_depthSearched, int m_score, uint8_t m_nodeType) : zobristKey(m_zobristKey), bestMove(m_bestMove), depthSearched(m_depthSearched), score(m_score), nodeType(m_nodeType) {}
};

namespace TranspositionTable {
    inline constexpr size_t TRANSPOSITION_TABLE_SIZE = 1 << 24;
    inline constexpr size_t TRANSPOSITION_TABLE_MASK = TRANSPOSITION_TABLE_SIZE - 1;

    static TranspositionEntry transpositionTableBuffer[TRANSPOSITION_TABLE_SIZE];

    inline int tableEntries = 0;
    inline int cutoffs = 0;
    inline int collisions = 0;

    TranspositionEntry& getEntry(uint64_t zobristKey);

    int correctScoreForStorage(int score, int rootDepth);
    int correctScoreForRetrieval(int score, int rootDepth);

    void addEntry(uint64_t zobristKey, Move bestMove, int rootDepth, int depthSearched, int score, int nodeType);
    void clear();
}
