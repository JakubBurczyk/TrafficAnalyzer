#pragma once
#include "Core/Application.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <imgui.h>

#include "Core/Instrumentor.hpp"

class TrafficAnalyzer : public App::Application{
public:

    explicit TrafficAnalyzer(const std::string& title):
        App::Application(title)
    {

    }

protected:
    void gui() override{

        if (m_state.show_some_panel) {
        ImGui::Begin("Some panel", &m_state.show_some_panel);
        // NOLINTNEXTLINE
        ImGui::Text("Hello World");
        ImGui::Text("DUPA");
        ImGui::End();
        }
    }
    
};