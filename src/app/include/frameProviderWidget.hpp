#pragma once
#include "imgui.h"
#include "frameProvider.hpp"
#include "widget.hpp"
#include "imfilebrowser.h"

namespace Traffic{

class FrameProviderWidget : public Widget{

private:

    std::shared_ptr<FrameProvider> frame_provider_;
    std::shared_ptr<ImGui::FileBrowser> fileDialog;

    bool from_directory_ = false;
    bool video_ = false;

public:

    FrameProviderWidget(std::shared_ptr<FrameProvider> frame_provider, bool hidden = false)
    :
        frame_provider_{frame_provider}
    {
        
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

        ImGui::End();

        if(fileDialog){
    
            fileDialog -> Display();
            if(fileDialog -> HasSelected())
            {
                std::string path = fileDialog -> GetSelected().string();
                frame_provider_ -> set_path(path);
                frame_provider_ -> set_video_mode(video_);
                frame_provider_ -> init();
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