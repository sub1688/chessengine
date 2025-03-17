#include "gui.h"

#include <imgui_internal.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <thread>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

/**
 * Texture loading code from Dear ImGui documentation:
 * https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
 */
inline bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width,
                                  int* out_height) {
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width,
                                                      &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
inline bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height) {
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    fclose(f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}

// Error callback function
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void Gui::render() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 6.0f));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;
    ImGui::Begin("Engine", nullptr, windowFlags);
    ImGui::SetWindowSize(ImVec2(1280, 768));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2F, 0.2F, 0.2F, 0.8F));

    if (ImGui::BeginMenuBar()) {
        ImGui::TextUnformatted("Chess");

        ImGui::SameLine(ImGui::GetWindowWidth() - 49);
        if (ImGui::Button("  X  ")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::GetIO().MousePos.y - ImGui::GetWindowPos().y <
            ImGui::GetCurrentWindow()->MenuBarHeight) {
            ImVec2 newPos = ImGui::GetWindowPos() + ImGui::GetMouseDragDelta();
            ImGui::SetWindowPos(newPos);
            ImGui::ResetMouseDragDelta(); // Reset so movement doesn't accumulate incorrectly
        }

        ImGui::EndMenuBar();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    renderChessBoard(600.F, 600.F);

    ImGui::Text("Test");

    ImGui::End();
}


void Gui::renderChessBoard(float width, float height) {
    const char* id = "board";

    ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
    ImVec2 size = ImVec2(width, height);
    ImRect rect = ImRect(pos, pos + size);
    ImGui::ItemSize(rect);
    ImGui::ItemAdd(rect, ImGui::GetCurrentWindow()->GetID(id));

    static int draggingPieceIndex = -1;

    // Draw checkerboard
    for (int i = 0; i < 64; i++) {
        auto squareX = i & 7, squareY = i / 8;

        ImVec2 squareSize(width / 8.F, height / 8.F);
        ImVec2 squarePos(pos.x + squareX * squareSize.x, pos.y + squareY * squareSize.y);

        ImU32 squareColor = ImGui::ColorConvertFloat4ToU32(
            (squareX + squareY) % 2 ? ImVec4(0.71F, 0.53F, 0.39F, 1.F) : ImVec4(0.94F, 0.85F, 0.71F, 1.F)
        );

        ImGui::GetForegroundDrawList()->AddRectFilled(squarePos, squarePos + squareSize, squareColor);
    }

    // Draw pieces
    for (int i = 0; i < 64; i++) {
        displayPieceMailbox[i] = currentBoard->mailbox[i];

        auto squareX = i & 7, squareY = 7 - i / 8;

        ImVec2 squareSize(width / 8.F, height / 8.F);
        ImVec2 imagePos(pos.x + squareX * squareSize.x, pos.y + squareY * squareSize.y);

        if (displayPieceMailbox[i] != NONE) {
            ImGui::GetForegroundDrawList()->AddImage(pieceTextures[displayPieceMailbox[i]], imagePos,
                                                     imagePos + squareSize);
        }
    }

    ImVec2 relativeMousePos = ImGui::GetIO().MousePos - ImGui::GetWindowPos() - pos;
    ImGui::Text((std::to_string(pos.x) + " " + std::to_string(pos.y)).c_str());
    if (relativeMousePos > ImVec2(0, 0) && relativeMousePos < size) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (draggingPieceIndex == -1) {
                int squareX = static_cast<int>(relativeMousePos.x) / static_cast<int>(size.x / 8);
                int squareY = static_cast<int>(relativeMousePos.y) / static_cast<int>(size.y / 8);
                std::cout << std::to_string(squareX) << " " << std::to_string(squareY) << std::endl;;
            }
        }
    }
}


void Gui::init(Board* board) {
    std::cout << "[+] Imgui opengl3 init...\n";

    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // Create Window
    window = glfwCreateWindow(1, 1, "Chess Engine", nullptr, nullptr);
    glfwSetWindowPos(window, -1, -1);
    if (window == nullptr)
        return;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Enable high-DPI scaling
    glfwSetWindowContentScaleCallback(window, [](GLFWwindow* window, float xscale, float yscale) {
        // Adjust the scale factor as needed, ImGui will handle scaling from here
        ImGuiIO& io = ImGui::GetIO();
        io.DisplayFramebufferScale = ImVec2(xscale, yscale);
    });

    // Fix window problems
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);

    // New Board
    currentBoard = board;

    // Setup Dear Imgui
    setupImgui();

    glfwDestroyWindow(window);
    glfwTerminate();
}


void Gui::setupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.Fonts->Clear();

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 23.F;
    io.Fonts->AddFontFromMemoryTTF((void*)fontEmbed, sizeof(fontEmbed), 23.0F, &fontConfig);

    ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10.f, 8.f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.075f, 0.075f, 0.075f, 1.f); // Dark gray menu bar

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load piece textures
    for (int piece = 0; piece < 12; piece++) {
        std::string path = "assets/" + std::to_string(piece) + ".png";
        int width, height;
        bool ret = LoadTextureFromFile(path.c_str(), &pieceTextures[piece], &width, &height);
        if (!ret) {
            std::cout << "Failed to load texture " << path << std::endl;
            exit(0);
        }
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Handle multiple viewports (so ImGui windows can be moved out)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0, 0, 0, 0); // Fully transparent background
        glClear(GL_COLOR_BUFFER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
