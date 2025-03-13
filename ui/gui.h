#pragma once
#include "GLFW/glfw3.h"
#include "font.embed"

namespace Gui {
    inline GLFWwindow* window;

    void init();
    void setupImgui();
    void render();
}
