#include "transpositiontable.h"
#include <cstring>

#include "search.h"

void TranspositionTable::addEntry(uint64_t zobristKey, Move bestMove, int rootDepth, int depthSearched, int score,
                                  int nodeType) {
    uint64_t index = zobristKey & TRANSPOSITION_TABLE_MASK;

    TranspositionEntry entry = getEntry(zobristKey);
    if (entry.zobristKey != 0 && entry.zobristKey != zobristKey) {
        collisions++;
        return;
    }
    // Replace if empty or different key (collision), or if the new depth is greater
    transpositionTableBuffer[index] = TranspositionEntry(zobristKey, bestMove, depthSearched,
                                                         correctScoreForStorage(score, rootDepth), nodeType);
    if (tableEntries < TRANSPOSITION_TABLE_SIZE)
        tableEntries++;
}

int TranspositionTable::correctScoreForRetrieval(int score, int rootDepth) {
    if (abs(score) >= MATE_THRESHOLD) {
        int sign = score > 0 ? 1 : -1;
        return (score * sign - rootDepth * 100) * sign;
    }
    return score;
}

int TranspositionTable::correctScoreForStorage(int score, int rootDepth) {
    if (abs(score) >= MATE_THRESHOLD) {
        int sign = score > 0 ? 1 : -1;
        return (score * sign + rootDepth * 100) * sign;
    }
    return score;
}

TranspositionEntry &TranspositionTable::getEntry(uint64_t zobristKey) {
    uint64_t index = zobristKey & TRANSPOSITION_TABLE_MASK;
    return transpositionTableBuffer[index];
}


void TranspositionTable::clear() {
    std::memset(transpositionTableBuffer, 0, TRANSPOSITION_TABLE_SIZE * sizeof(TranspositionEntry));
    tableEntries = 0;
}