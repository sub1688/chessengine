#pragma once

#include "../engine/board.h"
#include <imgui.h>
#include "GLFW/glfw3.h"
#include "font.embed"

static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)   { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }

namespace Gui {
    inline Board currentBoard;
    inline GLFWwindow* window;
    inline GLuint pieceTextures[12];
    inline PIECE displayPieceMailbox[64];

    void init();
    void setupImgui();
    void render();

    void renderChessBoard(float width, float height);
}
