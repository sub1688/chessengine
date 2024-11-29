#include <array>

#include "board.h"
#include "movegen.h"
#include <iostream>

int main() {
    Board::setStartingPosition();
    Board::printBoard();

    Movegen::precomputeRookMovegenTable();
    Movegen::precomputeBishopMovegenTable();

    // Movegen::printMovementMask(Movegen::generateLegalBishopMoves(35, true));
    Movegen::printMovementMask(Movegen::generateRookMovementMask(32));
}
