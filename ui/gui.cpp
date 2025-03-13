#include "gui.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

#include <iostream>

// Error callback function
void glfwErrorCallback(int error, const char *description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void Gui::render()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 6.0f));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration
                                | ImGuiWindowFlags_MenuBar;
    ImGui::Begin("Engine", nullptr, windowFlags);
    ImGui::SetWindowSize(ImVec2(1280, 768));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2F, 0.2F, 0.2F, 0.8F));

    if (ImGui::BeginMenuBar())
    {
        ImGui::TextUnformatted("Chess Engine");

        ImGui::SameLine(ImGui::GetWindowWidth() - 49);
        if (ImGui::Button("  X  ")) { glfwSetWindowShouldClose(window, GLFW_TRUE); }

        ImGui::EndMenuBar();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();



    ImGui::End();
}

void Gui::init() {
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

    // Setup Dear Imgui
    setupImgui();

    glfwDestroyWindow(window);
    glfwTerminate();
}


void Gui::setupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.Fonts->Clear();

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 23.F;
    io.Fonts->AddFontFromMemoryTTF((void*) fontEmbed, sizeof(fontEmbed), 23.0F, &fontConfig);

    ImGui::StyleColorsClassic();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10.f, 8.f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.075f, 0.075f, 0.075f, 1.f); // Dark gray menu bar

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

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
