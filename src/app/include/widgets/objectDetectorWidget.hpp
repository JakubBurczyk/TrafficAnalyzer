#pragma once
#include "imgui.h"
#include "imfilebrowser.h"

#include <GL/gl.h>

#include "widget.hpp"
#include "objectDetector.hpp"


namespace Traffic{

class ObjectDetectorWidget : public Widget{

private:

    std::shared_ptr<ObjectDetector> object_detector_;
    std::shared_ptr<ImGui::FileBrowser> fileDialog;

public:

    ObjectDetectorWidget(std::shared_ptr<ObjectDetector> object_detector, bool hidden = false)
    :
        object_detector_{object_detector}
    {
        set_hidden_state(hidden);
        fileDialog = std::make_shared<ImGui::FileBrowser>();
        fileDialog -> SetTypeFilters({".onnx"});
    }

    void gui() override {
        ImGui::Begin("Object Detector");

        if(ImGui::Button("Select ONNX file")){
                fileDialog -> Open();
        }

        ImGui::End();

        if(fileDialog){
            fileDialog -> Display();

            if(fileDialog -> HasSelected()){
                std::string path = fileDialog -> GetSelected().string();
                object_detector_ -> set_onnx_path(path);
                object_detector_ -> init();
                fileDialog -> ClearSelected();
            }
        }
        
    }


};

} // namespace Traffic