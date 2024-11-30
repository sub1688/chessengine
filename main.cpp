#include <array>

#include "board.h"
#include "movegen.h"
#include <iostream>

int main() {
    Board::setStartingPosition();
    Board::printBoard();

    // init
    Movegen::precomputeMovementMasks();

    Movegen::precomputeRookMovegenTable();
    Movegen::precomputeBishopMovegenTable();

    Movegen::printMovementMask(Movegen::generatePseudoLegalPawnMoves(50, false));

}
