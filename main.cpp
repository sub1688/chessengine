#include <array>
#include <chrono>

#include "board.h"
#include "movegen.h"
#include <iostream>
#include <optional>

#include <string>

std::string indexToChessNotation(int index) {
    if (index < 0 || index > 63) {
        return ""; // Invalid index
    }

    char file = 'a' + (index % 8); // File is based on the remainder when divided by 8
    char rank = '1' + (index / 8); // Rank is based on the quotient when divided by 8

    return std::string(1, file) + rank; // Combine file and rank into a string
}


void benchmarkPerft(int depth) {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    uint64_t perft = Movegen::perft(depth);
    auto stop = high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start).count();
    // Calculate nodes per second
    double nodesPerSecond = perft / duration;
    std::cout << "Perft nodes: " << perft
              << " Speed: " << std::fixed << std::setprecision(2) << (nodesPerSecond / 1'000) << " Kn/s" << std::endl;
}

void debugPerft(int depth) {
    std::array<std::optional<Move>, 216> moves = Movegen::generateAllLegalMovesOnBoard();

    for (int i = 0; i < 216; i++) {
        if (!moves[i].has_value())
            break;

        Move move = moves[i].value();

        if (depth == 1) {
            std::cout << indexToChessNotation(move.from) << indexToChessNotation(move.to) << " 1" << std::endl;
            continue;
        }

        Board::move(move);

        uint64_t perft = Movegen::perft(depth - 1);
        std::cout << indexToChessNotation(move.from) << indexToChessNotation(move.to) << " " << perft << std::endl;
        Board::undoMove(move);
    }
}

int main() {
    Board::setStartingPosition();
    Board::printBoard();

    // init
    Movegen::precomputeMovementMasks();
    Movegen::precomputeRookMovegenTable();
    Movegen::precomputeBishopMovegenTable();

    benchmarkPerft(4);
}