#include "san.h"
#include "movegen.h"
#include <string>

std::string StandardAlgebraicNotation::boardToSan(Board &board, const Move &move) {
    if (move.castle) {
        if (move.to % 8 == 6) {
            return "O-O";
        }
        return "O-O-O";
    }

    char pieceChar = getSanPieceChar(move.pieceFrom);
    std::string dest = squareToString(move.to);

    // Pawn moves (e.g., "e4")
    if (pieceChar == 'P') {
        if (move.capture != NONE) {
            if (move.promotion != NONE) {
                dest += "=";
                dest += getSanPieceChar(move.promotion); // Append char separately
            }

            return squareToFile(move.from) + "x" + dest;
        }

        if (move.promotion != NONE) {
            dest += "=";
            dest += getSanPieceChar(move.promotion); // Append char separately
        }
        return dest;
    }

    // Normal piece moves (e.g., "Nf3", "Bb5")
    std::string sanMove = std::string(1, pieceChar);

    // Handle disambiguation (if necessary)
    if (requiresDisambiguation(board, move)) {
        sanMove += disambiguation(board, move);
    }

    // Capture notation
    if (move.capture != NONE) {
        sanMove += "x";
    }

    sanMove += dest;

    // Handle promotions (e.g., "e8=Q")
    if (move.promotion != NONE) {
        sanMove += "=" + getSanPieceChar(move.promotion);
    }

    return sanMove;
}

std::vector<std::string> StandardAlgebraicNotation::split(const std::string &str, char delimiter) {
    std::vector<std::string> result;
    size_t start = 0, end;

    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(str.substr(start)); // Last part

    return result;
}

std::string StandardAlgebraicNotation::squareToString(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
}

int StandardAlgebraicNotation::stringToSquare(const std::string &square) {
    if (square.length() != 2) return -1; // Invalid input

    char file = square[0];
    char rank = square[1];

    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') return -1; // Out of range

    return (rank - '1') * 8 + (file - 'a');
}

char StandardAlgebraicNotation::getSanPieceChar(int piece) {
    const char pieceChars[] = {'P', 'N', 'B', 'Q', 'K', 'R'};
    return (piece >= 6) ? pieceChars[piece - 6] : pieceChars[piece];
}

char StandardAlgebraicNotation::getSanPiecePromotion(int promotion) {
    const char pieceChars[] = {'N', 'B', 'R', 'Q'};
    return pieceChars[promotion];
}

std::string StandardAlgebraicNotation::squareToFile(int square) {
    return std::string(1, 'a' + (square % 8));
}

// Checks if a move requires disambiguation
bool StandardAlgebraicNotation::requiresDisambiguation(Board &board, const Move &move) {
    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard(board);
    for (int i = 0; i < moves.elements; i++) {
        Move m = moves.buffer[i];
        if (board.move(m)) {
            if (m.pieceFrom == move.pieceFrom && m.to == move.to && m.from != move.from) {
                board.undoMove(m);
                return true;
            }
            board.undoMove(m);
        }
    }
    return false;
}

// Generates the disambiguation string if needed
std::string StandardAlgebraicNotation::disambiguation(Board &board, const Move &move) {
    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard(board);
    bool fileConflict = false, rankConflict = false, otherConflict = false;

    for (int i = 0; i < moves.elements; i++) {
        Move m = moves.buffer[i];
        // If another piece of the same type can also move to the same destination
        if (m.pieceFrom == move.pieceFrom && m.to == move.to && m.from != move.from) {
            if ((m.from % 8) == (move.from % 8)) {
                rankConflict = true;
            }
            if ((m.from / 8) == (move.from / 8)) {
                fileConflict = true;
            }
            if (!fileConflict && !rankConflict) {
                otherConflict = true; // Conflicting piece on different rank & file
            }
        }
    }

    // If another piece of the same type can move to the same destination but is neither in the same rank nor file
    if (otherConflict) {
        return squareToFile(move.from); // Full square notation needed
    }
    if (rankConflict) {
        return std::string(1, '1' + (move.from / 8));
    }
    if (fileConflict) {
        return squareToFile(move.from);
    }

    return "";
}

std::string StandardAlgebraicNotation::toUci(const Move &move) {
    std::string uci;
    uci += 'a' + (move.from % 8); // File of from-square
    uci += '1' + (move.from / 8); // Rank of from-square
    uci += 'a' + (move.to % 8); // File of to-square
    uci += '1' + (move.to / 8); // Rank of to-square

    // Append promotion piece if applicable (Q, R, B, N)
    if (move.promotion != NONE) {
        char promoChar = "nbrq"[move.promotion]; // Assuming 0 = Knight, 1 = Bishop, 2 = Rook, 3 = Queen
        uci += promoChar;
    }

    return uci;
}
