#include <array>
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
void startCLIListening() {
    int passes = 0;
    while (passes++ < 100000) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit")
            break;
        if (input == "ping") {
            std::cout << "pong\n";
        } else if (input == "reset") {
            over = false;
            Board::setStartingPosition();
        } else if (input == "print") {
            Board::printBoard();
        } else if (input.starts_with("move")) {
            if (input.length() <= 5) {
                std::cout << "Error: provide a valid move\n";
                continue;
            }
            std::string move = input.substr(5);

            ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();

            int prevMoveNumber = Board::moveNumber;
            for (int i = 0; i < moves.elements; i++) {
                Move moveObj = moves.buffer[i];

                if (StandardAlgebraicNotation::boardToSan(moveObj) == move) {
                    Board::move(moveObj);
                    std::cout << "Moved: " << move << std::endl;

                    if (Movegen::inCheckmate() || Movegen::inStalemate() || Board::isDrawnByRepetition()) {
                        std::cout << "game over" << std::endl;
                        over = true;
                    }
                    break;
                }
            }
            if (Board::moveNumber == prevMoveNumber) {
                std::cout << "Error: finding move: " << move << " is` not legal" << std::endl;
            }
        } else if (input.starts_with("search")) {
            if (input.length() <= 7) {
                std::cout << "Error: provide a valid time setting\n";
                continue;
            }

            if (over) {
                std::cout << "game already ended" << std::endl;
                continue;
            }

            int milliseconds = std::stoi(input.substr(7));

            Search::startIterativeSearch(milliseconds);
            std::cout << "Search complete.\n";
        }
    }
}

int main() {
    std::cout << "[+] Generating random zobrist keys...\n";
    Zobrist::init();
    std::cout << "[+] Setting Board Startpos...\n";
    Board::setStartingPosition();
    std::cout << "[+] Movegen init...\n";
    Movegen::init();
    std::cout << "[+] PST init...\n";
    PieceSquareTable::initializePieceSquareTable();
    std::cout << "[+] Loading Opening Book..\n";
    OpeningBook::loadOpeningBook("assets/openingbook.txt");
    std::cout << "[+] Done!\n";

    // Board::importFEN("7K/P1p1p1p1/2P1P1Pk/6pP/3p2P1/1P6/3P4/8 w - - 0 1");

    // startCLIListening();

    BoardWindow::init();
}
