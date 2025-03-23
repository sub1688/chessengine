#include "gui.h"

#include <imgui_internal.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <thread>

#include "../engine/movegen.h"
#include "../engine/search.h"
#include "../util/arrayvec.h"

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

    ImGui::BeginTabBar("tabs");

    if (ImGui::BeginTabItem("Analysis")) {
        ImGui::Columns(2);
        const float evaluationBarWidth = ImGui::GetColumnWidth() / 25.F;
        renderEvaluationBar(Search::currentEval, evaluationBarWidth, ImGui::GetColumnWidth() - ImGui::GetStyle().WindowPadding.x * 3 - evaluationBarWidth);
        ImGui::SameLine();
        renderChessBoard(ImGui::GetColumnWidth() - ImGui::GetStyle().WindowPadding.x * 3 - evaluationBarWidth, ImGui::GetColumnWidth() - ImGui::GetStyle().WindowPadding.x * 3 - evaluationBarWidth, true);
        ImGui::Text("Max Threads:");
        ImGui::SliderInt(" ", &Search::MAX_THREADS, 1, static_cast<int>(std::thread::hardware_concurrency()));
        ImGui::NextColumn();
        std::string str = std::to_string(static_cast<float>((Search::lastSearchTurnIsWhite ? 1 : -1) * Search::currentEval) / 100.F);
        if (abs((Search::lastSearchTurnIsWhite ? 1 : -1) * Search::currentEval) > MATE_THRESHOLD) {

            str = "#" + std::to_string(static_cast<int>((Search::POSITIVE_INFINITY - abs(

                                                             static_cast<float>(

                                                                 (Search::lastSearchTurnIsWhite ? 1 : -1) *

                                                                 Search::currentEval)))

                                                        / 2 + 1));

        }
        ImGui::Text(("Engine Evaluation: " + str).c_str());
        ImGui::Text(("Depth: " + std::to_string(Search::currentDepth)).c_str());
        ImGui::Columns(1);
        ImGui::EndTabItem();

        if (currentSearchType == NO_SEARCH) {
            currentSearchType = ANALYSIS_SEARCH;

            std::thread([]() {
                Search::startIterativeSearch(*board, 100000);
            }).detach();
        }
    }
    if (ImGui::BeginTabItem("Game")) {
        if (currentSearchType == ANALYSIS_SEARCH) {
            currentSearchType = NO_SEARCH;
            Search::searchCancelled = true;
        }
        ImGui::Columns(2);
        renderChessBoard(ImGui::GetColumnWidth() - ImGui::GetStyle().WindowPadding.x * 2, ImGui::GetColumnWidth() - ImGui::GetStyle().WindowPadding.x * 2, false);
        ImGui::NextColumn();
        ImGui::SliderInt("Time to think", &timeToThink, 10, 20000);
        ImGui::Columns(1);

        if (currentSearchType == NO_SEARCH && Search::searchCancelled && !board->isDrawn() && !Movegen::inCheckmate(*board)/* && !board->whiteToMove*/) {
            currentSearchType = GAME_SEARCH;
            std::thread([] {
                Search::startIterativeSearch(*board, timeToThink);
                lastMoveByBot = Search::bestMove;
                moveAnimation = ImVec2(0, 0);
                board->move(Search::bestMove);
                currentSearchType = NO_SEARCH;
                Search::searchCancelled = true;
            }).detach();
        }

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();

}


void Gui::renderChessBoard(float width, float height, bool analyze) {
    const char* id = "board";

    ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
    ImVec2 size = ImVec2(width, height);
    ImRect rect = ImRect(pos, pos + size);
    ImGui::ItemSize(rect);
    ImGui::ItemAdd(rect, ImGui::GetCurrentWindow()->GetID(id));

    static int draggingPieceIndex = -1;

    // Piece dragging logic
    ImVec2 relativeBoardPos = pos - ImGui::GetWindowPos();
    ImVec2 relativeMousePos = ImGui::GetIO().MousePos - ImGui::GetWindowPos() - relativeBoardPos;
    if (relativeMousePos > ImVec2(0, 0) && relativeMousePos < size) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (draggingPieceIndex == -1) {
                int squareX = static_cast<int>(relativeMousePos.x) / static_cast<int>(size.x / 8);
                int squareY = static_cast<int>(relativeMousePos.y) / static_cast<int>(size.y / 8);
                if (board->getPiece(draggingPieceIndex) != NONE) {
                    draggingPieceIndex = squareX + (7 - squareY) * 8;
                    Search::searchCancelled = true;
                }
            }
        }else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && draggingPieceIndex != -1) {
            int squareX = static_cast<int>(relativeMousePos.x) / static_cast<int>(size.x / 8);
            int squareY = 7 - static_cast<int>(relativeMousePos.y) / static_cast<int>(size.y / 8);
            int newSquareIndex = squareX + squareY * 8;

            ArrayVec<Move, 218> legalMoves = Movegen::generateAllLegalMovesOnBoard(*board);
            for (int i = 0; i < legalMoves.elements; i++) {
                Move move = legalMoves.buffer[i];
                if (board->move(move)) {
                    if (move.from == draggingPieceIndex && move.to == newSquareIndex) {
                        currentSearchType = NO_SEARCH;
                        break;
                    }
                    board->undoMove(move);
                }
            }
            draggingPieceIndex = -1;
        }
    }

    // Draw checkerboard
    ImVec2 squareSize(width / 8.F, height / 8.F);
    for (int i = 0; i < 64; i++) {
        auto squareX = i & 7, squareY = i / 8;

        ImVec2 squarePos(pos.x + static_cast<float>(squareX) * squareSize.x, pos.y + static_cast<float>(squareY) * squareSize.y);

        ImU32 squareColor = ImGui::ColorConvertFloat4ToU32(
            (squareX + squareY) % 2 ? ImVec4(0.71F, 0.53F, 0.39F, 1.F) : ImVec4(0.94F, 0.85F, 0.71F, 1.F)
        );

        ImGui::GetWindowDrawList()->AddRectFilled(squarePos, squarePos + squareSize, squareColor);
    }

    if (draggingPieceIndex != -1) {
        ArrayVec<Move, 218> legalMoves = Movegen::generateAllLegalMovesOnBoard(*board);
        for (int i = 0; i < legalMoves.elements; i++) {
            Move move = legalMoves.buffer[i];
            if (move.from != draggingPieceIndex)
                continue;
            if (board->move(move)) {
                board->undoMove(move);

                ImVec2 squarePos(pos.x + static_cast<float>(move.to & 7) * squareSize.x, pos.y + static_cast<float>(7 - move.to / 8) * squareSize.y);
                ImGui::GetWindowDrawList()->AddRectFilled(squarePos, squarePos + squareSize, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5F, 0.9F, 0.5F, 0.3F)));
            }
        }
    }

    // Draw pieces
    for (int i = 0; i < 64; i++) {
        displayPieceMailbox[i] = board->mailbox[i];

        if (i == draggingPieceIndex)
            continue;

        auto squareX = i & 7, squareY = 7 - i / 8;

        ImVec2 imagePos(pos.x + static_cast<float>(squareX) * squareSize.x, pos.y + static_cast<float>(squareY) * squareSize.y);

        if (displayPieceMailbox[i] != NONE) {
            if (!Search::isNullMove(lastMoveByBot) && lastMoveByBot.to == i && lastMoveByBot.pieceFrom == displayPieceMailbox[i]) {
                auto prevSquareX = lastMoveByBot.from & 7, prevSquareY = 7 - lastMoveByBot.from / 8;
                ImVec2 prevImagePos(pos.x + static_cast<float>(prevSquareX) * squareSize.x, pos.y + static_cast<float>(prevSquareY) * squareSize.y);
                if (moveAnimation.x == 0 && moveAnimation.y == 0) {
                    moveAnimation = prevImagePos;
                }
                ImVec2 addition = ImVec2((imagePos.x - moveAnimation.x) / 6.F,(imagePos.y - moveAnimation.y) / 6.F);
                moveAnimation = moveAnimation + addition;

                ImGui::GetWindowDrawList()->AddImage(pieceTextures[displayPieceMailbox[i]], moveAnimation,
                                                     moveAnimation + squareSize);;
            }else {
                ImGui::GetWindowDrawList()->AddImage(pieceTextures[displayPieceMailbox[i]], imagePos,
                                                         imagePos + squareSize);
            }
        }
    }

    if (draggingPieceIndex != -1 && displayPieceMailbox[draggingPieceIndex] != NONE) {
        ImVec2 drawPos = relativeMousePos - ImVec2(squareSize.x / 2, squareSize.y / 2);
        ImGui::GetWindowDrawList()->AddImage(pieceTextures[displayPieceMailbox[draggingPieceIndex]], pos + drawPos, pos + drawPos + squareSize);
    }

    if (!Search::isNullMove(Search::bestMove) && analyze) {
        ImVec2 squarePos(pos.x + static_cast<float>(Search::bestMove.to & 7) * squareSize.x + squareSize.x / 2, pos.y + static_cast<float>(7 - Search::bestMove.to / 8) * squareSize.y + squareSize.y / 2);
        ImVec2 fromSquarePos(pos.x + static_cast<float>(Search::bestMove.from & 7) * squareSize.x + squareSize.x / 2, pos.y + static_cast<float>(7 - Search::bestMove.from / 8) * squareSize.y + squareSize.y / 2);
        drawArrow(fromSquarePos, squarePos, width * 0.086F, width * 0.02323F, ImGui::ColorConvertFloat4ToU32(ImVec4(0.5, 0.7, 0.5, 0.6)));
    }
}

void Gui::renderEvaluationBar(int eval, float width, float height) {
    const char* id = "bar";

    ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
    ImVec2 size = ImVec2(width, height);
    ImRect rect = ImRect(pos, pos + size);
    ImGui::ItemSize(rect);
    ImGui::ItemAdd(rect, ImGui::GetCurrentWindow()->GetID(id));

    ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size, ImGui::ColorConvertFloat4ToU32(ImVec4(0.17F, 0.17F, 0.17F, 1.0F)));

    float trueEval = static_cast<float>((Search::lastSearchTurnIsWhite ? 1 : -1) * eval) / 100.F;

    float evalOffset = trueEval >= 0
                           ? height / 2 + height / 2 / -(trueEval / 5.F + 1.F)
                           : -(height / 2 + height / 2 / -(-trueEval / 5.F + 1.F));

    if (abs(eval) > 30000) {
        smoothEvalOffset += (trueEval < 0 ? -height / 2 - smoothEvalOffset : height / 2 - smoothEvalOffset) / 8.F;
    }else {
        smoothEvalOffset += (evalOffset - smoothEvalOffset) / 20.F;
    }

    float evalHeight = height / 2 + smoothEvalOffset;

    ImGui::GetWindowDrawList()->AddRectFilled(pos + ImVec2(0, height - evalHeight), pos + size, 0xFFFFFFFF);
}


void Gui::drawArrow(ImVec2 from, ImVec2 to, float arrowSize = 40.0f, float thickness = 20.0f, ImU32 color = IM_COL32(255, 255, 255, 255))
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Compute direction vector (normalized)
    ImVec2 dir = ImVec2(to.x - from.x, to.y - from.y);
    float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (length == 0.0f) return; // Prevent division by zero
    dir.x /= length;
    dir.y /= length;

    // Perpendicular vector for width
    ImVec2 perp(-dir.y, dir.x);

    // Adjusted shaft endpoint to avoid overlap with arrowhead
    ImVec2 shaft_end = ImVec2(to.x - dir.x * arrowSize, to.y - dir.y * arrowSize);

    // Compute shaft quad vertices
    ImVec2 shaft_left = ImVec2(from.x + perp.x * thickness * 0.5f, from.y + perp.y * thickness * 0.5f);
    ImVec2 shaft_right = ImVec2(from.x - perp.x * thickness * 0.5f, from.y - perp.y * thickness * 0.5f);
    ImVec2 end_left = ImVec2(shaft_end.x + perp.x * thickness * 0.5f, shaft_end.y + perp.y * thickness * 0.5f);
    ImVec2 end_right = ImVec2(shaft_end.x - perp.x * thickness * 0.5f, shaft_end.y - perp.y * thickness * 0.5f);

    // Compute arrowhead points
    ImVec2 arrow_left = ImVec2(to.x - dir.x * arrowSize + perp.x * arrowSize * 0.65f,
                               to.y - dir.y * arrowSize + perp.y * arrowSize * 0.65f);
    ImVec2 arrow_right = ImVec2(to.x - dir.x * arrowSize - perp.x * arrowSize * 0.65f,
                                to.y - dir.y * arrowSize - perp.y * arrowSize * 0.65f);

    // Draw shaft (filled quad)
    draw_list->AddQuadFilled(shaft_left, shaft_right, end_right, end_left, color);

    // Draw arrowhead (filled triangle)
    draw_list->AddTriangleFilled(to, arrow_left, arrow_right, color);

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
    Gui::board = board;

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
