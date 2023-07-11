#pragma once
#include "Core/Application.hpp"
#include "Core/Instrumentor.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <imgui.h>


#include "objectDetector.hpp"

class TrafficAnalyzer : public App::Application{
private:
    std::shared_ptr<ObjectDetector> detector_;

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

public:

    explicit TrafficAnalyzer(const std::string& title, std::shared_ptr<ObjectDetector> detector)
        :
         App::Application(title)
        ,detector_{detector}
    {

    }


    
};