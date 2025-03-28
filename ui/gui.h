#pragma once

#include "../engine/board.h"
#include <imgui.h>
#include <vector>

#include "GLFW/glfw3.h"
#include "font.embed"

#define ANALYSIS_SEARCH 1
#define GAME_SEARCH 2
#define NO_SEARCH 0

static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)   { return {lhs.x + rhs.x, lhs.y + rhs.y}; }
static ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)   { return {lhs.x - rhs.x, lhs.y - rhs.y}; }
static bool operator>(const ImVec2& lhs, const ImVec2& rhs)   { return lhs.x > rhs.x && lhs.y > rhs.y; }
static bool operator<(const ImVec2& lhs, const ImVec2& rhs)   { return lhs.x < rhs.x && lhs.y < rhs.y; }

namespace Gui {
    inline Board* board;
    inline GLFWwindow* window;
    inline GLuint pieceTextures[12];
    inline PIECE displayPieceMailbox[64];

    inline int currentSearchType = NO_SEARCH;
    inline float smoothEvalOffset = 0;
    inline Move lastMoveByBot = Move();
    inline ImVec2 moveAnimation = ImVec2(0, 0);
    inline int timeToThink = 5000;
    inline std::vector<Move> arrows = std::vector<Move>();

    void init(Board* board);
    void setupImgui();
    void render();

    void renderChessBoard(float width, float height, bool analyze);
    void renderEvaluationBar(int eval, float width, float height);
    void drawArrow(ImVec2 from, ImVec2 to, float arrowSize, float thickness, ImU32 color);
}
