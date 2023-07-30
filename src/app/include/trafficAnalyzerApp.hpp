#pragma once
#include "Core/Application.hpp"
#include "Core/Instrumentor.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <imgui.h>


#include "trafficAnalyzer.hpp"
#include "trafficAnalyzerGui.hpp"
#include "trafficAnalyzerWidget.hpp"
#include "frameProviderWidget.hpp"
#include "objectDetectorWidget.hpp"
#include "backgroundEstimatorWidget.hpp"
#include "framePreprocessorWidget.hpp"
#include "trafficTrackerWidget.hpp"

namespace Traffic{

class TrafficAnalyzerApp : public App::Application{
private:
    std::shared_ptr<TrafficAnalyzerGui> gui_;
    std::shared_ptr<TrafficAnalyzer> analyzer_;

protected:
    void gui() override{
        gui_ -> gui();
    }

public:

    explicit TrafficAnalyzerApp(const std::string& title, std::shared_ptr<TrafficAnalyzer> analyzer):
        App::Application(title),
        analyzer_{analyzer}
    {
        gui_ = std::make_shared<TrafficAnalyzerGui>();

        gui_ -> add_widget(std::make_shared<ObjectDetectorWidget>(analyzer_->get_object_detector()));
        
        gui_ -> add_widget(std::make_shared<FrameProviderWidget>(analyzer_ -> get_frame_provider()));

        gui_ -> add_widget(std::make_shared<FramePreprocessorWidget>(analyzer_ -> get_frame_preprocessor()));

        gui_ -> add_widget(std::make_shared<BackgroundEstimatorWidget>(analyzer_ -> get_background_estimator()));

        gui_ -> add_widget(std::make_shared<TrafficTrackerWidget>(analyzer_ -> get_traffic_tracker()));

        gui_ -> add_widget(std::make_shared<TrafficAnalyzerWidget>(analyzer_));

        
    }


    
};

} //namespace Traffic