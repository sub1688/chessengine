#include "transpositiontable.h"
#include <cstring>

void TranspositionTable::addEntry(uint64_t zobristKey, Move bestMove, int depthSearched, int score, int nodeType) {
    size_t index = zobristKey & TRANSPOSITION_TABLE_MASK;
    transpositionTableBuffer[index] = TranspositionEntry(zobristKey, bestMove, depthSearched, score, nodeType);
    tableEntries++;
}

TranspositionEntry& TranspositionTable::getEntry(uint64_t zobristKey) {
    size_t index = zobristKey & TRANSPOSITION_TABLE_MASK;
    return transpositionTableBuffer[index];
}


void TranspositionTable::clear() {
    std::memset(transpositionTableBuffer, 0, sizeof(transpositionTableBuffer));
    tableEntries = 0;
}

void TranspositionTable::incrementRepetitionEntry(uint64_t zobristKey) {
    size_t index = zobristKey & REPETITION_TABLE_MASK;
    repetitionTableBuffer[index]++;
}

void TranspositionTable::decrementRepetitionEntry(uint64_t zobristKey) {
    size_t index = zobristKey & REPETITION_TABLE_MASK;
    if (repetitionTableBuffer[index] > 0) {
        repetitionTableBuffer[index]--;
    }
}

uint8_t TranspositionTable::getRepetitionEntry(uint64_t zobristKey) {
    return repetitionTableBuffer[zobristKey & REPETITION_TABLE_MASK];
}





