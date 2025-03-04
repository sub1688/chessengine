#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <string>

#include "engine/san.h"
#include "engine/movegen.h"
#include "engine/openingbook.h"
#include "engine/piecesquaretable.h"
#include "engine/search.h"
#include "engine/zobrist.h"
#include "ui/window.h"

bool over = false;

void startCLIListening(Board& board) {
    int passes = 0;
    while (passes++ < 100000) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit")
            break;
        if (input == "ping") {
            std::cout << "pong\n";
        }
        else if (input == "reset") {
            over = false;
            board.setStartingPosition();
            Search::transpositionTable.clear();
        }
        else if (input == "print") {
            board.printBoard();
        }
        else if (input.starts_with("move")) {
            if (input.length() <= 5) {
                std::cout << "Error: provide a valid move\n";
                continue;
            }
            std::string move = input.substr(5);

            ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard(board);

            int prevMoveNumber = board.moveNumber;
            for (int i = 0; i < moves.elements; i++) {
                Move moveObj = moves.buffer[i];

                if (StandardAlgebraicNotation::boardToSan(board, moveObj) == move) {
                    board.move(moveObj);
                    std::cout << "Moved: " << move << std::endl;

                    if (Movegen::inCheckmate(board) || Movegen::inStalemate(board) || board.isDrawn()) {
                        std::cout << "game over" << std::endl;
                        over = true;
                    }
                    break;
                }
            }
            if (board.moveNumber == prevMoveNumber) {
                std::cout << "Error: finding move: " << move << " is` not legal" << std::endl;
            }
        }
        else if (input.starts_with("search")) {
            if (input.length() <= 7) {
                std::cout << "Error: provide a valid time setting\n";
                continue;
            }

            if (over) {
                std::cout << "game already ended" << std::endl;
                continue;
            }

            int milliseconds = std::stoi(input.substr(7));

            Search::startIterativeSearch(board, milliseconds);
            std::cout << "Search complete.\n";
        }
    }
}

void benchmarkPerft(Board& board, int depth) {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    uint64_t perft = Movegen::perft(board, depth);
    auto stop = high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start).count();
    // Calculate nodes per second
    double nodesPerSecond = perft / duration;
    std::cout << "Perft nodes: " << perft
        << " Speed: " << std::fixed << std::setprecision(2) << (nodesPerSecond / 1'000'000) << " Mn/s" << std::endl;
}


int main() {
    std::cout << "[+] Generating random zobrist keys...\n";
    Zobrist::init();
    std::cout << "[+] Setting Board Startpos...\n";

    Board board;
    board.setStartingPosition();

    std::cout << "[+] Move generator init...\n";
    Movegen::init();

    std::cout << "[+] Square bias table init...\n";
    PieceSquareTable::initializePieceSquareTable();

    std::cout << "[+] Loading Opening Book..\n";
    OpeningBook::loadOpeningBook("assets/openingbook.txt");
    // board.importFEN("8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1");

    std::cout << "[+] Done! (Maximum Threads: " << Search::MAX_THREADS << ")\n";
    // startCLIListening(board);

    BoardWindow::init(&board);
}
