#include <array>
#include <chrono>

#include "board.h"
#include "movegen.h"
#include <iostream>
#include <optional>

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


int main() {
    Board::setStartingPosition();
    Board::printBoard();

    // init
    Movegen::precomputeMovementMasks();

    Movegen::precomputeRookMovegenTable();
    Movegen::precomputeBishopMovegenTable();

    benchmarkPerft(2);
}