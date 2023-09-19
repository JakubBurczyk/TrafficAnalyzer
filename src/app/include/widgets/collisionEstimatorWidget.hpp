#pragma once
#include "imgui.h"

#include <thread>
#include <chrono>
#include <utility>

#include "widget.hpp"
#include "imageViewer.hpp"

#include "collisionEstimator.hpp"



namespace Traffic{

class CollisionEstimatorWidget : public Widget{

private:

    std::shared_ptr<CollisionEstimator> collision_;
    std::shared_ptr<ImageViewer> image_viewer_colliders_;

    CollistionOptions& options;

    cv::Mat frame_colliders_;
    std::thread image_thread_;

protected:



    // void gui_options_CUDA_MOG_2(){
    //     ImGui::Checkbox("CUDA MOG 2 | Detect Shadows", &options.CUDA_MOG_2.detectShadows);
    //     ImGui::InputInt("CUDA MOG 2 | history", &options.CUDA_MOG_2.history);
    //     ImGui::InputDouble(" CUDA MOG 2 | var Threshold", &options.CUDA_MOG_2.varThreshold);
    // }



public:

    CollisionEstimatorWidget(std::shared_ptr<CollisionEstimator> collision,  bool hidden = false)
        :
        collision_{collision},
        options{collision_ -> get_options_ref()}
    {
        image_viewer_colliders_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);

        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(collision_ -> get_mtx_update());
                    collision_ -> get_cv_update().wait(lock);
                    frame_colliders_ = collision_ -> visualize_colliders();
                }
            });
    }

    

    void gui() override {
        ImGui::Begin("Collision Estimator");


        ImGui::SameLine();
        if(ImGui::Button("Preview colliders")){
            image_viewer_colliders_ -> toggle_enabled();
        }


        ImGui::BeginChild("Colliders");
        image_viewer_colliders_ -> show_image(frame_colliders_);
        ImGui::EndChild();

        ImGui::End();

    }

};

} // namespace Traffic