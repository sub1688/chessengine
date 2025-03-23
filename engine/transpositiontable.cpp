#include "transpositiontable.h"
#include <cstring>
#include "search.h"

void TranspositionTable::addEntry(uint64_t zobristKey, Move bestMove, int rootDepth, int depthSearched, int score,
                                  int nodeType) {
    uint64_t index = zobristKey & TRANSPOSITION_TABLE_MASK;

    TranspositionEntry entry;
    bool replace = false;
    if (tableLookup(zobristKey, entry)) {
        int nodeTypeOld = EXTRACT_NODE_TYPE(entry.data);
        int depth = EXTRACT_DEPTH_SEARCHED(entry.data);
        if (nodeType == EXACT_BOUND && nodeTypeOld != EXACT_BOUND && depth <= depthSearched) {
            replace = true;
        }
    }else {
        replace = true;
    }

    if (replace)
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
