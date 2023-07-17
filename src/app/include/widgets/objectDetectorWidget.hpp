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
    std::shared_ptr<FrameProvider> frame_provider_;
    std::shared_ptr<ImGui::FileBrowser> fileDialog;
    std::shared_ptr<ImageViewer> image_viewer_;

    bool run_detections_ = false;
public:

    ObjectDetectorWidget(std::shared_ptr<ObjectDetector> object_detector, bool hidden = false)
    :
        object_detector_{object_detector}
    {
        image_viewer_ = std::make_shared<ImageViewer>();
        frame_provider_ = object_detector_ -> get_frame_provider();

        set_hidden_state(hidden);
        fileDialog = std::make_shared<ImGui::FileBrowser>();
        fileDialog -> SetTypeFilters({".onnx"});
    }

    void gui() override {
        ImGui::Begin("Object Detector");

        if(ImGui::Button("CUDA")){
            object_detector_ -> toggle_CUDA_mode_();
        }

        if(ImGui::Button("Select ONNX file")){
                fileDialog -> Open();
        }

        ImGui::Text("CUDA mode: %b", object_detector_ -> get_CUDA_mode());

        if( object_detector_ -> is_ready()){
            
            ImGui::Text("Model path: %s", object_detector_ -> get_model_path().c_str());

            if(ImGui::Button("Start detections")){
                frame_provider_ -> start();
                run_detections_= true;
            }

            ImGui::SameLine();
            if(ImGui::Button("Stop detections")){
                frame_provider_ -> stop();
                run_detections_ = false;
            }

            if(run_detections_ && frame_provider_ -> is_ready())
            {
                cv::Mat frame = frame_provider_ -> get_frame();

                object_detector_ -> detect(frame);
                cv::Mat visualization = object_detector_ -> visualize(frame);

                for(auto &detection : object_detector_ -> get_detections()){
                    cv::Rect box = detection.box;
                    cv::Scalar color = cv::Scalar(255,0,0);

                    cv::rectangle(visualization, box, color, 2);
                }

                if(!frame_provider_ -> next_frame()){
                    frame_provider_ -> stop();
                    run_detections_ = false;

                }else{
                    ImGui::BeginChild("Frames");
                    std::cout << "Displaying frame :" <<  frame_provider_ -> get_frame_number() << "\n";

                    image_viewer_ -> show_image(visualization);
                    ImGui::EndChild();
                }


            }

            
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