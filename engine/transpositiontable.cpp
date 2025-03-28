#include "transpositiontable.h"
#include <cstring>
#include "search.h"

void TranspositionTable::addEntry(uint64_t zobristKey, Move bestMove, int rootDepth, int depthSearched, int score,
                                  int nodeType) {
    uint64_t index = zobristKey & TRANSPOSITION_TABLE_MASK;


    transpositionTableBuffer[index] = TranspositionEntry(zobristKey, bestMove, depthSearched,
                                                         correctScoreForStorage(score, rootDepth), nodeType);
    if (tableEntries < TRANSPOSITION_TABLE_SIZE)
        tableEntries++;
}

int TranspositionTable::correctScoreForRetrieval(int score, int rootDepth) {
    if (score <= -MATE_THRESHOLD) {
        return score + rootDepth;
    }
    if (score >= MATE_THRESHOLD) {
        return score - rootDepth;
    }
    return score;
}

int TranspositionTable::correctScoreForStorage(int score, int rootDepth) {
    if (score <= -MATE_THRESHOLD) {
        return score - rootDepth;
    }
    if (score >= MATE_THRESHOLD) {
        return score + rootDepth;
    }
    return score;
}

int TranspositionTable::tableLookup(uint64_t zobristKey, TranspositionEntry &out) {
    uint64_t index = zobristKey & TRANSPOSITION_TABLE_MASK;
    TranspositionEntry entry = transpositionTableBuffer[index];

    if (entry.zobristKey == 0) {
        return TRANSPOSITION_TABLE_LOOKUP_FAILURE;
    }

    if (entry.zobristKey == (zobristKey ^ entry.data)) {
        out = entry;
        return TRANSPOSITION_TABLE_LOOKUP_SUCCESS;
    }

    return TRANSPOSITION_TABLE_LOOKUP_FAILURE;
}


void TranspositionTable::clear() {
    std::memset(&transpositionTableBuffer, 0, sizeof(transpositionTableBuffer));
    tableEntries = 0;
}
