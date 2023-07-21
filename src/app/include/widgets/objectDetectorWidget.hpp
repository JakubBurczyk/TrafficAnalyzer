#pragma once
#include "imgui.h"

#include <GL/gl.h>


#include "imfilebrowser.h"
#include "widget.hpp"
#include "imageViewer.hpp"

#include "objectDetector.hpp"

namespace Traffic{

class ObjectDetectorWidget : public Widget{

private:

    std::shared_ptr<ObjectDetector> object_detector_;

    std::shared_ptr<ImGui::FileBrowser> fileDialog;
    std::shared_ptr<ImageViewer> image_viewer_;

    bool run_detections_ = false;

    std::thread image_thread_;
    cv::Mat visualization_frame_;

    int i = 0;
public:

    ObjectDetectorWidget(std::shared_ptr<ObjectDetector> object_detector, bool hidden = false)
    :
        object_detector_{object_detector}
    {
        image_viewer_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);
        fileDialog = std::make_shared<ImGui::FileBrowser>();
        fileDialog -> SetTypeFilters({".onnx"});
        i++;
        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(object_detector_ -> get_mtx_detections());
                    object_detector_ -> get_cv_detections().wait(lock);
                    visualization_frame_ = object_detector_ -> visualize();
                }
            });
    }

    void gui() override {
        ImGui::Begin("Object Detector");

        if(ImGui::Button("CUDA")){
            object_detector_ -> toggle_CUDA_mode_();
        }

        ImGui::SameLine();
        if(ImGui::Button("Select ONNX file")){
                fileDialog -> Open();
        }

        ImGui::Text("CUDA mode: %i", object_detector_ -> get_CUDA_mode());
        ImGui::Text("Model path: %s", object_detector_ -> get_model_path().c_str());

        if( object_detector_ -> is_ready()){
            if(ImGui::Button("Preview")){
                image_viewer_ -> toggle_enabled();
            }

            ImGui::BeginChild("Frames");
            image_viewer_ -> show_image(visualization_frame_);
            ImGui::EndChild();
            
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