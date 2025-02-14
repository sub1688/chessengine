#include <array>
#include <chrono>
#include <iostream>
#include <string>

#include "engine/movegen.h"
#include "engine/zobrist.h"
#include "engine/piecesquaretable.h"
#include "engine/search.h"
#include "ui/window.h"

namespace TranspositionTable {
    inline uint64_t tableEntries;
    void addEntry(uint64_t zobristKey, Move bestMove, int depthSearched, int score, int nodeType);
    void clear();
}

uint64_t testPerftTranspositionTable(int depth) {
    uint64_t perftCount = 0;

    // Generate all legal moves for the current position
    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();

    for (int i = 0; i < moves.elements; i++) {
        Move move = moves.buffer.at(i);

        if (Board::move(move)) {
            TranspositionTable::addEntry(Board::currentZobristKey, Search::NULL_MOVE, 0, 0, 0);
            if (depth == 1) {
                perftCount++; // At depth 1, count the legal move
            }
            else {
                // Recursively count moves at the next depth
                perftCount += testPerftTranspositionTable(depth - 1);
            }
            // Undo the move to restore the board state
            Board::undoMove(move);
        }
    }

    return perftCount;
}

int main() {
    Zobrist::init();
    Board::setStartingPosition();
    // Board::importFEN("8/8/k7/8/K1R5/8/8/8 w - - 0 1");
    Board::importFEN("k7/8/8/8/8/8/8/1RK5 w - - 0 1");

    Movegen::init();
    PieceSquareTable::initializePieceSquareTable();

    TranspositionTable::clear();
    // testPerftTranspositionTable(6);

    // std::cout << std::to_string(TranspositionTable::tableEntries) << std::endl;

    BoardWindow::init();
    return 0;
}
