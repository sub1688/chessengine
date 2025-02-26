#include "transpositiontable.h"
#include <cstring>

#include "search.h"

void TranspositionTable::addEntry(uint64_t zobristKey, Move bestMove, int rootDepth, int depthSearched, int score,
                                  int nodeType) {
    uint64_t index = zobristKey & TRANSPOSITION_TABLE_MASK;

    // Always replace item in tt!!
    transpositionTableBuffer[index] = TranspositionEntry(zobristKey, bestMove, depthSearched,
                                                         correctScoreForStorage(score, rootDepth), nodeType);
    if (tableEntries < TRANSPOSITION_TABLE_SIZE)
        tableEntries++;
}

int TranspositionTable::correctScoreForRetrieval(int score, int rootDepth) {
    if (abs(score) >= MATE_THRESHOLD) {
        int sign = score > 0 ? 1 : -1;
        return (score * sign - rootDepth) * sign;
    }
    return score;
}

int TranspositionTable::correctScoreForStorage(int score, int rootDepth) {
    if (abs(score) >= MATE_THRESHOLD) {
        int sign = score > 0 ? 1 : -1;
        return (score * sign + rootDepth) * sign;
    }
    return score;
}

int TranspositionTable::tableLookup(uint64_t zobristKey, TranspositionEntry &out) {
    uint64_t index = zobristKey & TRANSPOSITION_TABLE_MASK;
    TranspositionEntry entry = transpositionTableBuffer[index];

    if (entry.zobristKey == 0) {
        return TRANSPOSITION_TABLE_LOOKUP_FAILURE;
    }

    if (zobristKey == (entry.zobristKey ^ entry.data)) {
        out = entry;
        return TRANSPOSITION_TABLE_LOOKUP_SUCCESS;
    }

    return TRANSPOSITION_TABLE_LOOKUP_FAILURE;
}


void TranspositionTable::clear() {
    std::memset(&transpositionTableBuffer, 0, sizeof(transpositionTableBuffer));
    tableEntries = 0;
}

void TranspositionTable::storeKillerMove(Move move, int depth) {
    if (move.capture != NONE || move.promotion != NONE)
        return;

    if (killerMoves[depth][0] == move) return;

    /*
     * the previous killer move is shifted is so that the first killer move in the array is always the most recent one, which is likely to be better in more positions
     */
    killerMoves[depth][1] = killerMoves[depth][0];
    killerMoves[depth][0] = move;
}

