#pragma once
#include "imgui.h"
#include "objectDetector.hpp"
#include "widget.hpp"
#include "imfilebrowser.h"

namespace Traffic{

class ObjectDetectorWidget : public Widget{

private:

    std::shared_ptr<ObjectDetector> object_detector_;



public:

    ObjectDetectorWidget(std::shared_ptr<ObjectDetector> object_detector, bool hidden = false)
    :
        object_detector_{object_detector}
    {
        
        set_hidden_state(hidden);
    }

    void gui() override {
        ImGui::Begin("Object Detector");
        ImGui::End();
    }


};

} // namespace Traffic