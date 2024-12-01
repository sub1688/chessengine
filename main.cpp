#include <array>

#include "board.h"
#include "movegen.h"
#include <iostream>
#include <optional>

int main() {
    Board::setStartingPosition();
    Board::printBoard();

    // init
    Movegen::precomputeMovementMasks();

    Movegen::precomputeRookMovegenTable();
    Movegen::precomputeBishopMovegenTable();

    std::array<std::optional<Move>, 216> allMoves = Movegen::generateAllLegalMovesOnBoard();

    for (int i = 0; i < allMoves.size(); i++) {
        if (!allMoves[i].has_value()) {
            break;
        }

        std::cout << i << " " << static_cast<int>(allMoves[i].value().to) << std::endl;
    }

}
