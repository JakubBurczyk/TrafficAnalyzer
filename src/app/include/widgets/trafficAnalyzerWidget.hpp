#pragma once
#include "imgui.h"
#include "imfilebrowser.h"

#include <GL/gl.h>

#include "widget.hpp"
#include "trafficAnalyzer.hpp"


namespace Traffic{

class TrafficAnalyzerWidget : public Widget{

private:

    std::shared_ptr<TrafficAnalyzer> analyzer_;

public:

    TrafficAnalyzerWidget(std::shared_ptr<TrafficAnalyzer> analyzer, bool hidden = false)
    :
        analyzer_{analyzer}
    {
        set_hidden_state(hidden);
    }

    void gui() override {
        ImGui::Begin("Traffic Analyzer");

        

        ImGui::End();
    }


};

} // namespace Traffic