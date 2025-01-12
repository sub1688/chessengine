#include <array>
#include <chrono>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "engine/movegen.h"
#include "engine/piecesquaretable.h"
#include "engine/search.h"
#include "engine/zobrist.h"
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
    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();

    for (int i = 0; i < moves.elements; i++) {
        Move move = moves.buffer[i];

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

void printMoves(ArrayVec<Move, 218> moves) {
    for (int i = 0; i < moves.elements; i++) {
        std::cout << indexToChessNotation(moves.buffer[i].from) << " " << indexToChessNotation(moves.buffer[i].to) << std::endl;
    }
}

void replaceAll(std::string &str, const std::string &from, const std::string &to) {
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // Advance past the replacementpr
    }
}

void test_secure_distribution(int num_samples, int num_buckets) {
    std::vector<int> buckets(num_buckets, 0);
    uint64_t bucket_size = UINT64_MAX / num_buckets;

    // Generate random numbers and count how many fall in each bucket
    for (int i = 0; i < num_samples; ++i) {
        uint64_t random_number = Zobrist::secureRandomUnsigned64();
        int bucket_index = random_number / bucket_size;
        buckets[bucket_index]++;
    }

    // Display the distribution
    std::cout << "Bucket distribution (using std::random_device):\n";
    for (int i = 0; i < num_buckets; ++i) {
        double percentage = (buckets[i] * 100.0) / num_samples;
        std::cout << "Bucket " << i << ": " << buckets[i]
                  << " (" << std::fixed << std::setprecision(2)
                  << percentage << "%)\n";
    }
}

int main() {
    test_secure_distribution(100000, 100);

    std::unordered_map<char, int> notationPieces = {
        {'N', WHITE_KNIGHT}, {'B', WHITE_BISHOP}, {'K', WHITE_KING}, {'Q', WHITE_QUEEN}, {'R', WHITE_ROOK}
    };

    std::unordered_map<char, int> notationPromotion = {
        {'N', PROMOTE_KNIGHT}, {'B', PROMOTE_BISHOP}, {'Q', PROMOTE_QUEEN}, {'R', PROMOTE_ROOK}
    };

    Board::setStartingPosition();

    Movegen::init();
    PieceSquareTable::initializePieceSquareTable();

    BoardWindow::init();
    return 0;
    int flag = 0;

    while (flag++ < 1000000) {
        std::string input;
        std::cin >> input;

        if (input == "exit")
            break;

        if (input == "reset")
            Board::setStartingPosition();

        if (input == "print")
            Board::printBoard();

        if (input.starts_with("search:")) {
            std::string time = input.substr(7);
            long millis = std::stol(time);

            // Search::startIterativeSearch(millis);
            Board::move(Search::bestMove);
            std::cout << "bestMove:" << static_cast<int>(Search::bestMove.from) << "," << static_cast<int>(Search::bestMove.to) << std::endl;
        }

        if (input.starts_with("move:")) {
            std::string move = input.substr(5);

            replaceAll(move, "+", "");
            replaceAll(move, "x", "");
            replaceAll(move, "#", "");

            int length = move.length();

            if (length > 1) {
                Move resultMove = Move(0, 0);

                int promotion = -1;

                if (move.at(length - 2) == '=') {
                    promotion = notationPromotion.at(move.at(length - 1));
                    move = move.substr(0, length - 2);
                    length -= 2;
                }

                std::string squareComponent = move.substr(length - 2, length);
                int toIndex = notationToIndex(squareComponent.at(0), squareComponent.at(1));

                ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();

                for (int i = 0; i < moves.elements; i++) {
                    Move moveObj = moves.buffer[i];

                    if (moveObj.to == toIndex && promotion == moveObj.promotion) {
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
