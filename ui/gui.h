#pragma once
#include "../Walnut-cmake/Walnut/src/Walnut/Layer.h"
#include "../Walnut-cmake/vendor/imgui/imgui.h"
#include "../Walnut-cmake/Walnut/src/Walnut/Application.h"
#include "../Walnut-cmake/Walnut/src/Walnut/Input/Input.h"

class MainLayer : public Walnut::Layer {
public:
    Walnut::Application* application;

    void OnAttach() override;
    void OnUIRender() override;

    MainLayer(Walnut::Application* app) : application(app) {}
};