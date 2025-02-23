#include "openingbook.h"

#include <fstream>
#include <random>
#include <sstream>
#include <iostream>

#include "movegen.h"
#include "san.h"

void OpeningBook::loadOpeningBook(std::string filename) {
    Board board;
    board.setStartingPosition();

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

    // Use stringstream to process lines
    std::istringstream stream(buffer.str());
    while (std::getline(stream, line)) {
        if (line.starts_with("pos ")) {
            currentFen = line.substr(4);
            board.importFEN(currentFen);
            openingBook[currentFen] = std::vector<MoveEntry>(); // Initialize entry
        } else {
            std::vector<std::string> split = StandardAlgebraicNotation::split(line, ' ');
            if (split.empty()) continue;

            const std::string& move = split.at(0);
            int probability = std::stoi(split.at(1));

            openingBook[currentFen].emplace_back(move, probability);
        }
    }
}

Move OpeningBook::fetchNextBookMove(Board& board) {

    // Check if there's an entry in the opening book
    auto it = openingBook.find(board.generateFEN());
    if (it == openingBook.end() || it->second.empty()) {
        return {};
    }

    // Get reference to the move list
    const std::vector<MoveEntry>& moves = it->second;

    // Calculate total sum of probabilities
    int totalProbability = 0;
    for (const auto& moveEntry : moves) {
        totalProbability += moveEntry.probability;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, totalProbability); // Random number between 1 and totalProbability

    // Generate random number to pick a move
    int randomNumber = dis(gen);

    // Determine which move corresponds to the random number
    int cumulativeProbability = 0;
    for (const auto& moveEntry : moves) {
        cumulativeProbability += moveEntry.probability;
        if (randomNumber <= cumulativeProbability) {
            ArrayVec<Move, 218> availableMoves = Movegen::generateAllLegalMovesOnBoard(board);
            for (int i = 0; i < availableMoves.elements; i++) {
                if (StandardAlgebraicNotation::toUci(availableMoves.buffer[i]) == moveEntry.move) {
                    return availableMoves.buffer[i];
                }
            }
        }
    }

    return {};
}
