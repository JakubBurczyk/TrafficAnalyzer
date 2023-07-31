#pragma once
#include "imgui.h"

#include <GL/gl.h>
#include <thread>

#include "imfilebrowser.h"
#include "widget.hpp"
#include "imageViewer.hpp"

#include "trafficTracker.hpp"

namespace Traffic{

class TrafficTrackerWidget : public Widget{

private:

    std::shared_ptr<TrafficTracker> tracker_;

    std::shared_ptr<ImGui::FileBrowser> fileDialog;
    std::shared_ptr<ImageViewer> image_viewer_;

    std::thread image_thread_;
    cv::Mat visualization_frame_;

    int i = 0;

protected:


public:

    TrafficTrackerWidget(std::shared_ptr<TrafficTracker> tracker, bool hidden = false)
    :
        tracker_{tracker}
    {
        image_viewer_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);
        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(tracker_ -> get_mtx_tracks());
                    tracker_ -> get_cv_tracks().wait(lock);
                    visualization_frame_ = tracker_ -> visualize_tracklets();
                }
            });
    }

    void gui() override {
        ImGui::Begin("Object Tracker");

        if(ImGui::Button("Reset")){
            tracker_ -> reset();
        }

        if(ImGui::Button("Preview")){
            image_viewer_ -> toggle_enabled();
        }

        ImGui::BeginChild("Frames");
        image_viewer_ -> show_image(visualization_frame_);
        ImGui::EndChild();
            
        ImGui::End();
        
    }

};

} // namespace Traffic