#include "san.h"
#include "movegen.h"
#include <string>

std::string StandardAlgebraicNotation::boardToSan(const Move &move) {
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
            return squareToFile(move.from) + "x" + dest;
        }
        return dest;
    }

    // Normal piece moves (e.g., "Nf3", "Bb5")
    std::string sanMove = std::string(1, pieceChar);

    // Handle disambiguation (if necessary)
    if (requiresDisambiguation(move)) {
        sanMove += disambiguation(move);
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

std::string StandardAlgebraicNotation::squareToString(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
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
bool StandardAlgebraicNotation::requiresDisambiguation(const Move &move) {
    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();
    for (int i = 0; i < moves.elements; i++) {
        Move m = moves.buffer[i];
        if (m.pieceFrom == move.pieceFrom && m.to == move.to && m.from != move.from) {
            return true;
        }
    }
    return false;
}

// Generates the disambiguation string if needed
std::string StandardAlgebraicNotation::disambiguation(const Move &move) {
    ArrayVec<Move, 218> moves = Movegen::generateAllLegalMovesOnBoard();
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
    if (move.promotion != -1) {
        char promoChar = "nbrq"[move.promotion]; // Assuming 0 = Knight, 1 = Bishop, 2 = Rook, 3 = Queen
        uci += promoChar;
    }

    return uci;
}
