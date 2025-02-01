#pragma once

#include <cstdint>
#include "board.h"

#define UPPER_BOUND 1
#define EXACT_BOUND 0
#define LOWER_BOUND (-1)

struct TranspositionEntry {
    uint64_t zobristKey;
    Move bestMove;
    int depthSearched;
    int score;
    int nodeType;

    TranspositionEntry() : zobristKey(0), depthSearched(0), score(0), nodeType(0) {}
    TranspositionEntry(uint64_t m_zobristKey, Move m_bestMove, int m_depthSearched, int m_score, int m_nodeType) : zobristKey(m_zobristKey), bestMove(m_bestMove), depthSearched(m_depthSearched), score(m_score), nodeType(m_nodeType) {}
};

namespace TranspositionTable {
    inline constexpr size_t TRANSPOSITION_TABLE_SIZE = 1 << 24; // 32 million entries... around 1.6GB
    inline constexpr size_t TRANSPOSITION_TABLE_MASK = TRANSPOSITION_TABLE_SIZE - 1;

    inline constexpr size_t REPETITION_TABLE_SIZE = 1 << 24; // 32 million repetition entries, should be more than enough
    inline constexpr size_t REPETITION_TABLE_MASK = REPETITION_TABLE_SIZE - 1;

    static TranspositionEntry transpositionTableBuffer[TRANSPOSITION_TABLE_SIZE];
    static uint8_t repetitionTableBuffer[REPETITION_TABLE_SIZE];

    inline int tableEntries = 0;

    TranspositionEntry& getEntry(uint64_t zobristKey);

    void addEntry(uint64_t zobristKey, Move bestMove, int depthSearched, int score, int nodeType);
    void clear();

    void incrementRepetitionEntry(uint64_t zobristKey);
    void decrementRepetitionEntry(uint64_t zobristKey);
    uint8_t getRepetitionEntry(uint64_t zobristKey);
}
