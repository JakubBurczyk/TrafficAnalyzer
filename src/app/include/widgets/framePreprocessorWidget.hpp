#pragma once
#include "imgui.h"

#include <thread>
#include <chrono>

#include "imfilebrowser.h"
#include "widget.hpp"
#include "imageViewer.hpp"

#include "framePreprocessor.hpp"

namespace Traffic{

class FramePreprocessorWidget : public Widget{

private:

    std::shared_ptr<FramePreprocessor> frame_preprocessor_;
    std::shared_ptr<ImGui::FileBrowser> fileDialog;
    
    std::shared_ptr<ImageViewer> image_viewer_;

    int average_fps_ = 0;

    std::thread image_thread_;
    cv::Mat frame_;
    
public:

    FramePreprocessorWidget(std::shared_ptr<FramePreprocessor> frame_preprocessor, bool hidden = false)
    :
        frame_preprocessor_{frame_preprocessor}
    {
        image_viewer_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);

        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(frame_preprocessor_ -> get_mtx_new_frame());
                    frame_preprocessor_ -> get_cv_new_frame().wait(lock);
                    frame_ = frame_preprocessor_ -> get_masked_frame();
                }
            });
    }

    void gui() override {
        ImGui::Begin("Frame Preprocessor");

        if(ImGui::Button("Select Mask File")){
            fileDialog = std::make_shared<ImGui::FileBrowser>();
            fileDialog -> SetTypeFilters({".png", ".jpg",".jpeg"});
            fileDialog -> Open();
        }

        ImGui::SameLine();
        if(ImGui::Button("Remove mask")){
            frame_preprocessor_ -> remove_mask();
        }
        
        if(ImGui::Button("Preview Masked")){
            image_viewer_ -> toggle_enabled();
        }

        ImGui::BeginChild("Frames");
        image_viewer_ -> show_image(frame_);
        ImGui::EndChild();
        
        ImGui::End();
        
        if(fileDialog){
    
            fileDialog -> Display();
            if(fileDialog -> HasSelected())
            {
                std::string path = fileDialog -> GetSelected().string();
                cv::Mat mask = cv::imread(path, cv::ImreadModes::IMREAD_GRAYSCALE);
                frame_preprocessor_ -> set_mask(mask);
                fileDialog -> ClearSelected();
            }
        }

    
    }


};

} // namespace Traffic