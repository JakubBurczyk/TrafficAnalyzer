#pragma once
#include "imgui.h"

#include <GL/gl.h>


#include "imfilebrowser.h"
#include "widget.hpp"


#include "trafficAnalyzer.hpp"

namespace Traffic{

class TrafficAnalyzerWidget : public Widget{

private:

    std::shared_ptr<TrafficAnalyzer> analyzer_;


protected:

    void traffic_analyzer_gui(){
        ImGui::Text("Analyze traffic");

        if(ImGui::Button("Start Analyzer")){
            analyzer_ -> start_analyzer();
        }
        

    }

    void background_esitmator_gui(){
        ImGui::Text("Analyze background");

        if(ImGui::Button("Start background estimator")){
            analyzer_ -> start_background_est();
        }

    }

public:

    TrafficAnalyzerWidget(std::shared_ptr<TrafficAnalyzer> analyzer, bool hidden = false)
    :
        analyzer_{analyzer}
    {
        set_hidden_state(hidden);
    }

    void gui() override {
        ImGui::Begin("Traffic Analyzer");

        if(ImGui::Button("Reset Video Source")){
            analyzer_ -> reset_source();
        }

        ImGui::SameLine();
        if(ImGui::Button("Stop Processing")){
            if(analyzer_ -> stop()){
                ;
            };
        }

        traffic_analyzer_gui();
        background_esitmator_gui();

        ImGui::End();
    }


};

} // namespace Traffic