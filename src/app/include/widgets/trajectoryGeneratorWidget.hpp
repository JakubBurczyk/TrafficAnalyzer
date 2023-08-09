#pragma once
#include "imgui.h"

#include <thread>
#include <chrono>
#include <utility>

#include "widget.hpp"
#include "imageViewer.hpp"

#include "trajectoryGenerator.hpp"



namespace Traffic{

class TrajectoryGeneratorWidget : public Widget{

private:
    bool show_options_ = false;
    std::shared_ptr<TrajectoryGenerator> trajectory_;

    std::shared_ptr<ImageViewer> image_viewer_heatmap_;

    std::thread image_thread_;
    
    cv::Mat frame_heatmap_;

    TrajectoryOptions& options;
    
    std::shared_ptr<ImGui::FileBrowser> fileDialog;

protected:


public:

    TrajectoryGeneratorWidget(std::shared_ptr<TrajectoryGenerator> trajectory,  bool hidden = false)
        :
        trajectory_{trajectory},
        options{trajectory_ -> get_options_ref()}
    {
        image_viewer_heatmap_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);

        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(trajectory_ -> get_mtx_update());
                    trajectory_ -> get_cv_update().wait(lock);
                    // std::cout << "TRAJECTORIES HAVE BEEN UPDATED"<< std::endl;
                }
            });
    }

    void gui() override {
        ImGui::Begin("Trajectory Generator");
        if(ImGui::Button("Toggle heatmap")){

            image_viewer_heatmap_ -> toggle_enabled();
        }

        if(ImGui::Button("Presence Heatmap")){
            if(image_viewer_heatmap_ -> is_enabled()){
                frame_heatmap_ = trajectory_ -> generate_presence_heatmap();
            }
        }

        ImGui::SameLine();
        if(ImGui::Button("Speed Heatmap")){
            if(image_viewer_heatmap_ -> is_enabled()){
                frame_heatmap_ = trajectory_ -> generate_avg_speed_heatmap();
            }
        }

        ImGui::SameLine();
        if(ImGui::Button("X velocity Heatmap")){
            if(image_viewer_heatmap_ -> is_enabled()){
                frame_heatmap_ = trajectory_ -> generate_x_speed_heatmap();
            }
        }

        ImGui::SameLine();
        if(ImGui::Button("Y velocity Heatmap")){
            if(image_viewer_heatmap_ -> is_enabled()){
                frame_heatmap_ = trajectory_ -> generate_y_speed_heatmap();
            }
        }

        if(ImGui::Button("Select Save Directory")){
            fileDialog = std::make_shared<ImGui::FileBrowser>(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_SelectDirectory);
            fileDialog -> Open();
        }

        static std::string results_path = "./";
        static char results_filename[512] = "trajectory";
        ImGui::InputTextWithHint("Filename", "filename",results_filename,512);
        ImGui::SameLine();

        if(ImGui::Button("Save Trajectory Generation Results")){

        }

        ImGui::BeginChild("Background");
        image_viewer_heatmap_ -> show_image(frame_heatmap_);
        ImGui::EndChild();

        ImGui::End();

        if(fileDialog){
    
            fileDialog -> Display();
            if(fileDialog -> HasSelected())
            {
                results_path = fileDialog -> GetSelected().string();

                fileDialog -> ClearSelected();
            }
        }
    }

};

} // namespace Traffic