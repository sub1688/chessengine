#include <array>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "engine/movegen.h"
#include "engine/piecesquaretable.h"
#include "engine/search.h"
#include "ui/window.h"

std::string indexToChessNotation(int index) {
    if (index < 0 || index > 63) {
        return ""; // Invalid index
    }

    char file = 'a' + (index % 8); // File is based on the remainder when divided by 8
    char rank = '1' + (index / 8); // Rank is based on the quotient when divided by 8

    return std::string(1, file) + rank; // Combine file and rank into a string
}

int notationToFile(char file) {
    return file - 'a';
}

int notationToRank(char rank) {
    return rank - '1';
}

int notationToIndex(char file, char rank) {
    return notationToFile(file) + notationToRank(rank) * 8;
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
            if (Board::move(move)) {
                std::cout << indexToChessNotation(move.from) << indexToChessNotation(move.to) << " 1" << std::endl;
                Board::undoMove(move);
            }
            continue;
        }

        if (Board::move(move)) {
            uint64_t perft = Movegen::perft(depth - 1);

            std::cout << indexToChessNotation(move.from) << indexToChessNotation(move.to) << " " << perft << std::endl;
            Board::undoMove(move);
        }
    }
}

void replaceAll(std::string &str, const std::string &from, const std::string &to) {
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // Advance past the replacement
    }
}

int main() {
    std::unordered_map<char, int> notationPieces = {
        {'N', WHITE_KNIGHT}, {'B', WHITE_BISHOP}, {'K', WHITE_KING}, {'Q', WHITE_QUEEN}, {'R', WHITE_ROOK}
    };

    Board::setStartingPosition();

    Movegen::init();
    PieceSquareTable::initializePieceSquareTable();

    BoardWindow::init();

    while (true) {
        std::string input;
        std::cin >> input;

        if (input == "exit")
            break;

        if (input == "reset")
            Board::setStartingPosition();

        if (input == "print")
            Board::printBoard();

        if (input.starts_with("move:")) {
            std::string move = input.substr(5);

            replaceAll(move, "+", "");
            replaceAll(move, "x", "");
            replaceAll(move, "#", "");

            int length = move.length();

            if (length > 1) {
                Move resultMove = Move(0, 0);

                if (move.at(length - 2) == '=') {
                }

                std::string squareComponent = move.substr(length - 2, length);
                int toIndex = notationToIndex(squareComponent.at(0), squareComponent.at(1));

                std::array<std::optional<Move>, 216> moves = Movegen::generateAllLegalMovesOnBoard();

                for (int i = 0; i < 216; i++) {
                    if (!moves[i].has_value())
                        break;

                    Move moveObj = moves[i].value();

                    if (moveObj.to == toIndex) {
                        std::cout << toIndex << std::endl;
                        if (length == 2) {
                            if (moveObj.capture == NONE && (
                                    moveObj.pieceFrom == WHITE_PAWN || moveObj.pieceFrom == BLACK_PAWN)) {
                                resultMove = moveObj;
                                break;
                            }
                        } else {
                            if (length == 3) {
                                if (std::islower(move.at(0))) {
                                    if (moveObj.pieceFrom == WHITE_PAWN || moveObj.pieceFrom == BLACK_PAWN) {
                                        int file = notationToFile(move.at(0));
                                        if (moveObj.from % 8 == file) {
                                            resultMove = moveObj;
                                            break;
                                        }
                                    }
                                } else {
                                    int piece = notationPieces.at(move.at(0)) + (Board::whiteToMove ? 0 : 6);
                                    if (piece == moveObj.pieceFrom) {
                                        resultMove = moveObj;
                                        break;
                                    }
                                }
                            }
                            if (length == 4) {
                                int piece = notationPieces.at(move.at(0)) + (Board::whiteToMove ? 0 : 6);
                                int file = notationToFile(move.at(1));

                                if (piece == moveObj.pieceFrom && file == (moveObj.from % 8)) {
                                    resultMove = moveObj;
                                    break;
                                }
                            }
                        }
                    }
                }

                std::cout << indexToChessNotation(resultMove.from) << indexToChessNotation(resultMove.to) << std::endl;
                Board::move(resultMove);
                Board::printBoard();
            }
        }
    }
}
