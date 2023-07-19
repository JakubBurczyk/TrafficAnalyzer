#pragma once
#include "imgui.h"
#include "frameProvider.hpp"
#include "widget.hpp"
#include "imfilebrowser.h"
#include <thread>
#include <chrono>

#include "imageViewer.hpp"

namespace Traffic{

class FrameProviderWidget : public Widget{

private:

    std::shared_ptr<FrameProvider> frame_provider_;
    std::shared_ptr<ImGui::FileBrowser> fileDialog;
    
    std::shared_ptr<ImageViewer> image_viewer_;

    bool from_directory_ = false;
    bool video_ = false;
    bool display_frames = false;

    int average_fps_ = 0;

    std::thread image_thread_;
    cv::Mat frame_;
    
public:

    FrameProviderWidget(std::shared_ptr<FrameProvider> frame_provider, bool hidden = false)
    :
        frame_provider_{frame_provider}
    {
        image_viewer_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);

        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(frame_provider_ -> get_mtx_new_frame());
                    frame_provider_ -> get_cv_new_frame().wait(lock);
                    frame_ = frame_provider_ -> get_frame();
                }
            });
    }

    void gui() override {
        ImGui::Begin("Image/Video Source");
    
        static int selection = 0;
        ImGui::RadioButton("Images",&selection, 0); ImGui::SameLine();
        ImGui::RadioButton("Video", &selection, 1);
        video_ = selection;

        ImGui::SameLine();
        if(video_){
            if(ImGui::Button("Select File")){
                fileDialog = std::make_shared<ImGui::FileBrowser>();
                set_file_types();
                fileDialog -> Open();
            }
        }else{
            if(ImGui::Button("Select Directory")){
                fileDialog = std::make_shared<ImGui::FileBrowser>(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_SelectDirectory);
                set_file_types();
                fileDialog -> Open();
            }
        }


        ImGui::Text("Image source path: %s", frame_provider_ ->get_path_().c_str());
        
        if(frame_provider_ -> has_path_()){
            
            average_fps_ += 1000.0 / (frame_provider_ -> get_ms_since_read().count());
            average_fps_ = average_fps_ / 2;

            ImGui::Text("FPS: %d", average_fps_);
            ImGui::Text("Frame: %d", frame_provider_ -> get_frame_number());

            if(ImGui::Button("Preview")){
                image_viewer_ -> toggle_enabled();
            }

            ImGui::BeginChild("Frames");
            image_viewer_ -> show_image(frame_);
            ImGui::EndChild();
        }
        
        ImGui::End();
        
        if(fileDialog){
    
            fileDialog -> Display();
            if(fileDialog -> HasSelected())
            {
                std::string path = fileDialog -> GetSelected().string();
                frame_provider_ -> stop();
                frame_provider_ -> set_path(path);
                frame_provider_ -> set_video_mode(video_);
                frame_provider_ -> start();
                fileDialog -> ClearSelected();
            }
        }

    
    }

    void set_file_types(){
        if(video_){
            fileDialog -> SetTypeFilters({".mkv", ".mp4", ".avi"});
        }
        else{
            fileDialog -> SetTypeFilters({".jpeg", ".jpg", ".png"});
        }
    }

};

} // namespace Traffic