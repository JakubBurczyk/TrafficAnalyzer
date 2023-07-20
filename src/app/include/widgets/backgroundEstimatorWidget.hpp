#pragma once
#include "imgui.h"

#include <thread>
#include <chrono>


#include "widget.hpp"
#include "imageViewer.hpp"

#include "backgroundEstimator.hpp"



namespace Traffic{

class BackgroundEstimatorWidget : public Widget{

private:

    std::shared_ptr<BackgroundEstimator> background_est_;
    std::shared_ptr<ImageViewer> image_viewer_bg_;
    std::shared_ptr<ImageViewer> image_viewer_mask_;
    std::thread image_thread_;
    cv::Mat frame_bg_;
    cv::Mat frame_mask_;
protected:

    void estimator_choce_gui(){
        // ImGui::Checkbox("CNT", background_est_ -> get_options_ref().type);
    }
public:

    BackgroundEstimatorWidget(std::shared_ptr<BackgroundEstimator> background,  bool hidden = false)
        :
        background_est_{background}
    {
        image_viewer_bg_ = std::make_shared<ImageViewer>();
        image_viewer_mask_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);

        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(background_est_ -> get_mtx_update());
                    background_est_ -> get_cv_update().wait(lock);
                    frame_bg_ = background_est_ -> get_background();
                    frame_mask_ = background_est_ -> get_mask();
                    cv::cvtColor(frame_mask_, frame_mask_, cv::ColorConversionCodes::COLOR_GRAY2RGB);
                }
            });
    }


    void gui() override {
        ImGui::Begin("Background Estimator");

        if(ImGui::Button("Preview Background")){
            image_viewer_bg_ -> toggle_enabled();
        }

        ImGui::SameLine();
        if(ImGui::Button("Preview Mask")){
            image_viewer_mask_ -> toggle_enabled();
        }

        ImGui::BeginChild("Background");
        image_viewer_bg_ -> show_image(frame_bg_);
        image_viewer_mask_ -> show_image(frame_mask_);
        ImGui::EndChild();

        ImGui::End();
    }

};

} // namespace Traffic