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

public:

    FrameProviderWidget(std::shared_ptr<FrameProvider> frame_provider, bool hidden = false)
    :
        frame_provider_{frame_provider}
    {
        
        set_hidden_state(hidden);
    }

    void gui() override {
        ImGui::Begin("Image/Video Source");

        if(ImGui::Button("Select File")){
            fileDialog = std::make_shared<ImGui::FileBrowser>();
            fileDialog -> Open();
        }
        ImGui::SameLine();
        if(ImGui::Button("Select Directory")){
            fileDialog = std::make_shared<ImGui::FileBrowser>(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_SelectDirectory);
            fileDialog -> Open();
        }

        ImGui::End();

        if(fileDialog){
            fileDialog -> Display();

            if(fileDialog -> HasSelected())
            {
                std::cout << "Selected filename" << fileDialog -> GetSelected().string() << std::endl;
                fileDialog -> ClearSelected();
            }
        }
        

    }

};

} // namespace Traffic