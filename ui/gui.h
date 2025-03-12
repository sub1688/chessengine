#pragma once
#include "GLFW/glfw3.h"

namespace Gui {
    inline GLFWwindow* window;

    void init();
    void setupImgui();
    void render();
}
