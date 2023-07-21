#pragma once
#include "imgui.h"

#include <thread>
#include <chrono>
#include <utility>

#include "widget.hpp"
#include "imageViewer.hpp"

#include "backgroundEstimator.hpp"



namespace Traffic{

class BackgroundEstimatorWidget : public Widget{

private:
    bool show_options_ = false;
    std::shared_ptr<BackgroundEstimator> background_est_;
    std::shared_ptr<ImageViewer> image_viewer_bg_;
    std::shared_ptr<ImageViewer> image_viewer_mask_;
    std::thread image_thread_;
    cv::Mat frame_bg_;
    cv::Mat frame_mask_;
    BGEstimatorOptions& options;

protected:

    void gui_options_CNT(){
        ImGui::InputInt("CNT | Min Pixel Stability", &options.CNT.minPixelStability);
        ImGui::InputInt("CNT | Max Pixel Stability", &options.CNT.maxPixelStability);

        ImGui::Checkbox("CNT | Use History", &options.CNT.useHistory);
        ImGui::SameLine();
        ImGui::Checkbox("CNT | Is Parallel", &options.CNT.useHistory);
    }

    void gui_options_GMG(){
        ImGui::InputInt("GMG | Initialization Frames ", &options.GMG.initializationFrames);
        ImGui::InputDouble("GMG | Decision Threshold", &options.GMG.decisionThreshold);
    }

    void gui_options_GSOC(){
        static bool compensate_camera = false;
        ImGui::Checkbox("Compensate camera", &compensate_camera);
        if(compensate_camera){
            options.GSOC.mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_LK;
        }else{
            options.GSOC.mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
        }
    }

    void gui_switch_estimator(){
        static int32_t* opt_type =  reinterpret_cast<std::underlying_type_t<BGEstimatorType>*>(&(options.type));
        static int32_t type = *opt_type;
        ImGui::RadioButton("CNT", &type, static_cast<std::underlying_type_t<BGEstimatorType>>(BGEstimatorType::CNT));
        ImGui::SameLine();
        ImGui::RadioButton("GMG", &type, static_cast<std::underlying_type_t<BGEstimatorType>>(BGEstimatorType::GMG));
        ImGui::SameLine();
        ImGui::RadioButton("GSOC", &type, static_cast<std::underlying_type_t<BGEstimatorType>>(BGEstimatorType::GSOC));
        ImGui::SameLine();
        ImGui::RadioButton("LSBP", &type, static_cast<std::underlying_type_t<BGEstimatorType>>(BGEstimatorType::LSBP));
        ImGui::SameLine();
        ImGui::RadioButton("MOG", &type, static_cast<std::underlying_type_t<BGEstimatorType>>(BGEstimatorType::MOG));
        ImGui::SameLine();
        ImGui::RadioButton("Cuda MOG", &type, static_cast<std::underlying_type_t<BGEstimatorType>>(BGEstimatorType::CUDA_MOG));
        ImGui::SameLine();
        ImGui::RadioButton("Cuda MOG2", &type, static_cast<std::underlying_type_t<BGEstimatorType>>(BGEstimatorType::CUDA_MOG2));
        options.type = static_cast<BGEstimatorType>(type);
    }

    void gui_show_estirmator_options(){
    
        if (ImGui::CollapsingHeader("Estimator Options", ImGuiTreeNodeFlags_None))
        {
            
            switch(options.type){
                case BGEstimatorType::CNT:  { gui_options_CNT();    break; }
                case BGEstimatorType::GMG:  { gui_options_GMG();    break; }
                case BGEstimatorType::GSOC: { gui_options_GSOC();   break; }
                case BGEstimatorType::LSBP: {break;}
                case BGEstimatorType::MOG:  {break;}
                default:{break;}
                
            }
        }
        
    }

public:

    BackgroundEstimatorWidget(std::shared_ptr<BackgroundEstimator> background,  bool hidden = false)
        :
        background_est_{background},
        options{background_est_ -> get_options_ref()}
    {
        image_viewer_bg_ = std::make_shared<ImageViewer>();
        image_viewer_mask_ = std::make_shared<ImageViewer>();

        set_hidden_state(hidden);

        image_thread_ = std::thread([this](){
                while(true){
                    std::unique_lock lock(background_est_ -> get_mtx_update());
                    background_est_ -> get_cv_update().wait(lock);
                    try{
                        frame_bg_ = background_est_ -> get_background();
                    }catch(std::exception &e){
                        frame_bg_ = cv::Mat();
                    }

                    try{
                        frame_mask_ = background_est_ -> get_mask();
                        cv::cvtColor(frame_mask_, frame_mask_, cv::ColorConversionCodes::COLOR_GRAY2RGB);
                    }catch(std::exception &e){
                        frame_mask_ = cv::Mat();
                    }
                    
                    
                }
            });
    }

    

    void gui() override {
        ImGui::Begin("Background Estimator");

        if(ImGui::Button("Initialize")){
            background_est_ -> init();
        }

        ImGui::SameLine();
        if(ImGui::Button("Preview Background")){
            image_viewer_bg_ -> toggle_enabled();
        }

        ImGui::SameLine();
        if(ImGui::Button("Preview Mask")){
            image_viewer_mask_ -> toggle_enabled();
        }

        gui_switch_estimator();
        gui_show_estirmator_options();

        ImGui::BeginChild("Background");
        image_viewer_bg_ -> show_image(frame_bg_);
        image_viewer_mask_ -> show_image(frame_mask_);
        ImGui::EndChild();

        ImGui::End();
    }

};

} // namespace Traffic