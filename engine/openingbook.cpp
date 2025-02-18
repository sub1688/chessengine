#include "openingbook.h"

#include <fstream>
#include <random>
#include <sstream>

#include "movegen.h"
#include "san.h"
#include "search.h"

void OpeningBook::loadOpeningBook(std::string filename) {
    Board::setStartingPosition();

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening " << filename << std::endl;
        return;
    }

    // Read entire file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string line;
    std::string currentFen;
    int lines = 0;

    // Use stringstream to process lines
    std::istringstream stream(buffer.str());
    while (std::getline(stream, line)) {
        if (lines++ > 2000) break;

        if (line.starts_with("pos ")) {
            currentFen = line.substr(4);
            Board::importFEN(currentFen);
            openingBook[currentFen] = std::vector<Move>(); // Initialize entry
        } else {
            std::vector<std::string> split = StandardAlgebraicNotation::split(line, ' ');
            if (split.empty()) continue;

            std::string move = split.at(0);
            int squareFrom = StandardAlgebraicNotation::stringToSquare(move.substr(0, move.length() - 2));
            int squareTo = StandardAlgebraicNotation::stringToSquare(move.substr(2));

            Move moveObj;
            ArrayVec<Move, 218> availableMoves = Movegen::generateAllLegalMovesOnBoard();
            for (int i = 0; i < availableMoves.elements; i++) {
                if (availableMoves.buffer[i].from == squareFrom && availableMoves.buffer[i].to == squareTo) {
                    moveObj = availableMoves.buffer[i];
                    break;
                }
            }

            if (moveObj.from != moveObj.to) {
                openingBook[currentFen].push_back(moveObj);
            }
        }
    }

    Board::setStartingPosition();
}

Move OpeningBook::fetchNextBookMove() {

    // Check if there's an entry in the opening book
    auto it = openingBook.find(Board::generateFEN());
    if (it == openingBook.end() || it->second.empty()) {
        return {}; // Return empty move if not found
    }

    // Get reference to the move list
    const std::vector<Move>& moves = it->second;

    // Randomly select a move
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);

    return moves[0];
}