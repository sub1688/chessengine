#pragma once

#define MATE_THRESHOLD 30000

#include <chrono>
#include <limits>
#include <thread>

#include "board.h"
#include "transpositiontable.h"
#include "../util/arrayvec.h"

struct SearchResult {
    int evaluation;
    Move bestMove;

    SearchResult() : evaluation(0), bestMove(Move()) {}

    SearchResult(int m_evaluation, Move m_bestMove) : evaluation(m_evaluation), bestMove(m_bestMove) {
    }
};

struct ThreadWorkerInfo
{
    int threadNumber;
    int depthToSearch;

    Board board;

    ThreadWorkerInfo(int m_threadNumber, int m_depthToSearch) : threadNumber(m_threadNumber), depthToSearch(m_depthToSearch) {}
};

namespace Search {
    static unsigned int MAX_THREADS = 3;

    inline constexpr Move NULL_MOVE = Move();

    inline constexpr uint64_t WHITE_PASSED_PAWN_MASKS[64] = {
        0x303030303030300ULL,
        0x707070707070700ULL,
        0xe0e0e0e0e0e0e00ULL,
        0x1c1c1c1c1c1c1c00ULL,
        0x3838383838383800ULL,
        0x7070707070707000ULL,
        0xe0e0e0e0e0e0e000ULL,
        0xc0c0c0c0c0c0c000ULL,
        0x303030303030000ULL,
        0x707070707070000ULL,
        0xe0e0e0e0e0e0000ULL,
        0x1c1c1c1c1c1c0000ULL,
        0x3838383838380000ULL,
        0x7070707070700000ULL,
        0xe0e0e0e0e0e00000ULL,
        0xc0c0c0c0c0c00000ULL,
        0x303030303000000ULL,
        0x707070707000000ULL,
        0xe0e0e0e0e000000ULL,
        0x1c1c1c1c1c000000ULL,
        0x3838383838000000ULL,
        0x7070707070000000ULL,
        0xe0e0e0e0e0000000ULL,
        0xc0c0c0c0c0000000ULL,
        0x303030300000000ULL,
        0x707070700000000ULL,
        0xe0e0e0e00000000ULL,
        0x1c1c1c1c00000000ULL,
        0x3838383800000000ULL,
        0x7070707000000000ULL,
        0xe0e0e0e000000000ULL,
        0xc0c0c0c000000000ULL,
        0x303030000000000ULL,
        0x707070000000000ULL,
        0xe0e0e0000000000ULL,
        0x1c1c1c0000000000ULL,
        0x3838380000000000ULL,
        0x7070700000000000ULL,
        0xe0e0e00000000000ULL,
        0xc0c0c00000000000ULL,
        0x303000000000000ULL,
        0x707000000000000ULL,
        0xe0e000000000000ULL,
        0x1c1c000000000000ULL,
        0x3838000000000000ULL,
        0x7070000000000000ULL,
        0xe0e0000000000000ULL,
        0xc0c0000000000000ULL,
        0x300000000000000ULL,
        0x700000000000000ULL,
        0xe00000000000000ULL,
        0x1c00000000000000ULL,
        0x3800000000000000ULL,
        0x7000000000000000ULL,
        0xe000000000000000ULL,
        0xc000000000000000ULL,
        0x303030303030303ULL,
        0x707070707070707ULL,
        0xe0e0e0e0e0e0e0eULL,
        0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL,
        0x7070707070707070ULL,
        0xe0e0e0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0c0c0ULL
    };

    inline constexpr uint64_t BLACK_PASSED_PAWN_MASKS[64] = {
        0x3ULL,
        0x7ULL,
        0xeULL,
        0x1cULL,
        0x38ULL,
        0x70ULL,
        0xe0ULL,
        0xc0ULL,
        0x303ULL,
        0x707ULL,
        0xe0eULL,
        0x1c1cULL,
        0x3838ULL,
        0x7070ULL,
        0xe0e0ULL,
        0xc0c0ULL,
        0x30303ULL,
        0x70707ULL,
        0xe0e0eULL,
        0x1c1c1cULL,
        0x383838ULL,
        0x707070ULL,
        0xe0e0e0ULL,
        0xc0c0c0ULL,
        0x3030303ULL,
        0x7070707ULL,
        0xe0e0e0eULL,
        0x1c1c1c1cULL,
        0x38383838ULL,
        0x70707070ULL,
        0xe0e0e0e0ULL,
        0xc0c0c0c0ULL,
        0x303030303ULL,
        0x707070707ULL,
        0xe0e0e0e0eULL,
        0x1c1c1c1c1cULL,
        0x3838383838ULL,
        0x7070707070ULL,
        0xe0e0e0e0e0ULL,
        0xc0c0c0c0c0ULL,
        0x30303030303ULL,
        0x70707070707ULL,
        0xe0e0e0e0e0eULL,
        0x1c1c1c1c1c1cULL,
        0x383838383838ULL,
        0x707070707070ULL,
        0xe0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0ULL,
        0x3030303030303ULL,
        0x7070707070707ULL,
        0xe0e0e0e0e0e0eULL,
        0x1c1c1c1c1c1c1cULL,
        0x38383838383838ULL,
        0x70707070707070ULL,
        0xe0e0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0c0ULL,
        0x303030303030303ULL,
        0x707070707070707ULL,
        0xe0e0e0e0e0e0e0eULL,
        0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL,
        0x7070707070707070ULL,
        0xe0e0e0e0e0e0e0e0ULL,
        0xc0c0c0c0c0c0c0c0ULL,
    };

    inline constexpr int PIECE_VALUES[12] = {
        100,
        300,
        300,
        910,
        100000,
        500,
        -100,
        -300,
        -300,
        -910,
        -100000,
        -500
    };

    /**
     * These scores are not ACTUALLY infinity, they are just representative of a really high/low score to represent
     * situations with checkmate. I specifically chose 32000 as it is close to the 16 bit integer limit, which would
     * make the maximum/minimum score low enough to be fit into a single 64 bit integer along with other data in the
     * transposition table.
    */
    inline constexpr int NEGATIVE_INFINITY = -32000;
    inline constexpr int POSITIVE_INFINITY = 32000;

    inline constexpr int TRANSPOSITION_TABLE_BIAS = 10000000;
    inline constexpr int KILLER_MOVE_BIAS = 9000000;
    inline constexpr int LOSING_CAPTURE_BIAS = 2000000;
    inline constexpr int WINNING_CAPTURE_BIAS = 8000000;
    inline constexpr int PROMOTE_BIAS = 6000000;


    inline long currentTimeMillis = 0;
    inline long searchDuration = 0;
    inline int currentEval = 0;
    inline long times[256] = {};
    inline int currentDepth = 0;
    inline bool lastSearchTurnIsWhite = true;
    inline long nodesCounted = 0;
    inline double nodesPerSecond;
    inline long nullPrunes = 0;

    inline TranspositionTable transpositionTable = TranspositionTable();

    volatile inline bool searchCancelled = false;

    inline Move bestMove = NULL_MOVE;

    void orderMoves(ArrayVec<Move, 218> &moveVector, int rootDepth, int threadNumber, Move ttMove, int depth);

    void startIterativeSearch(Board& board, long time);

    void threadSearch(ThreadWorkerInfo *info);

    SearchResult search(Board& board, int threadNumber, int rootDepth, int depth, int alpha, int beta, bool wasNullSearch, bool inPrincipalVariation);

    SearchResult search(Board& board, int threadNumber, int depth);

    int negatedPrincipalVariationSearch(Board& board, int threadNumber, Move move, bool &firstMove, int moved, int rootDepth, int depth, int alpha, int beta, bool wasNullSearch, bool inPrincipalVariation);

    int evaluate(Board& board);

    int quiesce(Board& board, int alpha, int beta);

    int getPieceValue(uint8_t piece);

    double getEndGameBias(Board& board);

    bool canNullMove(Board& board);

    bool isNullMove(Move move);

    int evaluatePassedPawn(Board& board, uint8_t squareIndex, uint8_t piece);

    int evaluateKingDistance(uint8_t squareIndex, uint8_t otherKingIndex, uint8_t piece, int materialDelta);

    inline long getMillisSinceEpoch()
    {
        const auto now     = std::chrono::system_clock::now();
        const auto epoch   = now.time_since_epoch();
        const auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        return seconds.count();
    }
}
