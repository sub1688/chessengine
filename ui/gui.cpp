#include "gui.h"

#include "imgui_internal.h"
#include "../Walnut-cmake/vendor/imgui/imgui.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

void MainLayer::OnUIRender() {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("Chess Engine GUI", nullptr, windowFlags);

    ImGui::SetWindowSize(ImVec2(1124, 768));

    // Set Background window size and pos to that of the imgui panel
    glfwSetWindowSize(application->GetWindowHandle(), ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    glfwSetWindowPos(application->GetWindowHandle(), ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

    ImGui::Button("Button");
    ImGui::End();

}

void MainLayer::OnAttach() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags ^= ImGuiConfigFlags_DockingEnable;

    ImGuiStyle& style = ImGui::GetStyle();

    glfwSetWindowSize(application->GetWindowHandle(), 800, 600);
    glfwSetWindowAttrib(application->GetWindowHandle(), GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowAttrib(application->GetWindowHandle(), GLFW_RESIZABLE, GLFW_FALSE);

    ImGui::StyleColorsClassic();
}
