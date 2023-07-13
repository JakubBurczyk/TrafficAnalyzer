#pragma once
#include "Core/Application.hpp"
#include "Core/Instrumentor.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <imgui.h>


#include "objectDetector.hpp"
#include "widget.hpp"

namespace Traffic{

class TrafficAnalyzerGui{
private:

    std::vector<std::shared_ptr<Widget>> widgets_;

protected:

public:

    void gui(){
        for(auto &widget : widgets_){
            if(widget -> is_hidden()) continue;
            widget -> gui();
        }
    }
    
    void add_widget(std::shared_ptr<Widget> widget){
        widgets_.push_back(widget);
    }

};

} //  namespace Traffic

