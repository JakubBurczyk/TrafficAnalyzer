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
    
public:

    FrameProviderWidget(std::shared_ptr<FrameProvider> frame_provider, bool hidden = false)
    :
        frame_provider_{frame_provider}
    {
        image_viewer_ = std::make_shared<ImageViewer>();
        set_hidden_state(hidden);
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

        char path[512];
        std::string provider_path = frame_provider_ -> get_path_();
        std::copy(provider_path.begin(),provider_path.end(), path);

        ImGui::InputText("Path", path, 512, ImGuiInputTextFlags_ReadOnly );

        if(frame_provider_ -> is_ready()){

            if(ImGui::Button("Show frames")){
                frame_provider_ -> start();
                display_frames = true;
            }

            ImGui::SameLine();
            if(ImGui::Button("Stop preview")){
                frame_provider_ -> stop();
                display_frames = false;
            }

            ImGui::SameLine();
            if(ImGui::Button("Toggle Image Controls")){
                image_viewer_ -> toggle_controls();
            }


            if(display_frames){
                cv::Mat frame = frame_provider_ -> get_next_frame();
                
                if(frame.empty()){
                    frame_provider_ -> stop();
                }
                else{

                    ImGui::BeginChild("Frames");
                    APP_DEBUG(FP_DEBUG_NAME "Displaying frame {}", frame_provider_ -> get_frame_number() );
                    image_viewer_ -> show_image(frame);
                    ImGui::EndChild();
                    
                }
            }
        }
        
        ImGui::End();
        
        if(fileDialog){
    
            fileDialog -> Display();
            if(fileDialog -> HasSelected())
            {
                std::string path = fileDialog -> GetSelected().string();
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