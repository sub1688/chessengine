#pragma once

#include "../engine/board.h"
#include <imgui.h>
#include "GLFW/glfw3.h"
#include "font.embed"

static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)   { return {lhs.x + rhs.x, lhs.y + rhs.y}; }
static ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)   { return {lhs.x - rhs.x, lhs.y - rhs.y}; }
static bool operator>(const ImVec2& lhs, const ImVec2& rhs)   { return lhs.x > rhs.x && lhs.y > rhs.y; }
static bool operator<(const ImVec2& lhs, const ImVec2& rhs)   { return lhs.x < rhs.x && lhs.y < rhs.y; }

namespace Gui {
    inline Board* board;
    inline GLFWwindow* window;
    inline GLuint pieceTextures[12];
    inline PIECE displayPieceMailbox[64];

    inline bool thinking = false;

    void init(Board* board);
    void setupImgui();
    void render();

    void renderChessBoard(float width, float height);
}
