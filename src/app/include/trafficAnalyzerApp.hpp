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

namespace Traffic{

class TrafficAnalyzerApp : public App::Application{
private:
    std::shared_ptr<Traffic::TrafficAnalyzerGui> gui_;
    std::shared_ptr<Traffic::TrafficAnalyzer> analyzer_;

protected:
    void gui() override{
        gui_ -> gui();
    }

public:

    explicit TrafficAnalyzerApp(const std::string& title, std::shared_ptr<Traffic::TrafficAnalyzer> analyzer):
        App::Application(title),
        analyzer_{analyzer}
    {
        gui_ = std::make_shared<Traffic::TrafficAnalyzerGui>();

        gui_ -> add_widget(std::make_shared<ObjectDetectorWidget>(analyzer_->get_object_detector()));
        
        auto frame_provider = analyzer_ -> get_object_detector() -> get_frame_provider();
        gui_ -> add_widget(std::make_shared<FrameProviderWidget>(frame_provider));


        gui_ -> add_widget(std::make_shared<TrafficAnalyzerWidget>(analyzer_));
    }


    
};

} //namespace Traffic