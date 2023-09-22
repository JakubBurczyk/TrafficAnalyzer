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
        ImGui::Text("Traffic collision analysis");

        if(ImGui::Button("Start traffic analysis")){
            analyzer_ -> start_processor(PROCESSING_TYPE::TRAFFIC_ANALYZER);
        }
        

    }

    void background_esitmator_gui(){
        ImGui::Text("Analyze background");

        if(ImGui::Button("Start background estimator")){
            analyzer_ -> start_processor(PROCESSING_TYPE::BACKGROUND_ESTIMATOR);
            // analyzer_ -> start_background_est();
        }

        if(ImGui::Button("Send to detector")){
            cv::Mat frame = analyzer_ -> get_background_estimator() -> get_background();
            analyzer_ -> get_object_detector() -> detect(frame);
        }

    }

    void trajectory_generator_gui(){
        ImGui::Text("Generate trajectories");

        if(ImGui::Button("Start trajectory generator")){
            analyzer_ -> start_processor(PROCESSING_TYPE::TRAJECTORY_GENERATOR);
            // analyzer_ -> start_trajectory_gen();
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
        trajectory_generator_gui();
        background_esitmator_gui();

        ImGui::End();
    }


};

} // namespace Traffic