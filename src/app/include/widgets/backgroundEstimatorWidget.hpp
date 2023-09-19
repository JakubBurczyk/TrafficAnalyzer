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
    
    std::shared_ptr<ImGui::FileBrowser> fileDialog;

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
        static bool compensate_camera_GSOC = false;
        ImGui::Checkbox("GSOC | Compensate camera", &compensate_camera_GSOC);
        if(compensate_camera_GSOC){
            options.GSOC.mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_LK;
        }else{
            options.GSOC.mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
        }

        ImGui::InputInt("GSOC | n Samples", &options.GSOC.nSamples);
        ImGui::InputInt("GSOC | Hits Threshold", &options.GSOC.hitsThreshold);
        ImGui::InputFloat("GSOC | Replace Rate", &options.GSOC.replaceRate);
        ImGui::InputFloat("GSOC | Propagation Rate", &options.GSOC.propagationRate);
        ImGui::InputFloat("GSOC | Alpha", &options.GSOC.alpha);
        ImGui::InputFloat("GSOC | Beta", &options.GSOC.beta);
        ImGui::InputFloat("GSOC | Blinking Suppression Decay", &options.GSOC.blinkingSupressionDecay);
        ImGui::InputFloat("GSOC | Blinking Suppression Multiplier", &options.GSOC.blinkingSupressionMultiplier);
        ImGui::InputFloat("GSOC | Noise Removal Threshold BG", &options.GSOC.noiseRemovalThresholdFacBG);
        ImGui::InputFloat("GSOC | Noise Removal Threshold FG", &options.GSOC.noiseRemovalThresholdFacFG); 
    }

    void gui_options_LSBP(){
        static bool compensate_camera_LSBP = false;
        ImGui::Checkbox("LSBP | Compensate camera", &compensate_camera_LSBP);
        if(compensate_camera_LSBP){
            options.GSOC.mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_LK;
        }else{
            options.GSOC.mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
        }

        ImGui::InputInt("LSBP | n Samples", &options.LSBP.nSamples);
        ImGui::InputInt("LSBP | Radius", &options.LSBP.LSBPRadius);
        ImGui::InputInt("LSBP | Threshold", &options.LSBP.LSBPthreshold);
        ImGui::InputInt("LSBP | Min Count", &options.LSBP.minCount);
        ImGui::InputFloat("LSBP | Tlower", &options.LSBP.Tlower);
        ImGui::InputFloat("LSBP | Tupper", &options.LSBP.Tupper);
        ImGui::InputFloat("LSBP | Tinc", &options.LSBP.Tinc);
        ImGui::InputFloat("LSBP | Tdec", &options.LSBP.Tdec); 
        ImGui::InputFloat("LSBP | Rscale", &options.LSBP.Rscale);
        ImGui::InputFloat("LSBP | Rincdec", &options.LSBP.Rincdec); 
        ImGui::InputFloat("LSBP | Noise Removal Threshold BG", &options.LSBP.noiseRemovalThresholdFacBG);
        ImGui::InputFloat("LSBP | Noise Removal Threshold FG", &options.LSBP.noiseRemovalThresholdFacFG); 
    }

    void gui_options_MOG(){
        ImGui::InputInt("MOG | history", &options.MOG.history);
        ImGui::InputInt("MOG | nMixtures", &options.MOG.nmixtures);
        ImGui::InputDouble(" MOG | Background Ratio", &options.MOG.backgroundRatio);
        ImGui::InputDouble(" MOG | Noise Sigma", &options.MOG.noiseSigma);
    }

    void gui_options_CUDA_MOG(){
        ImGui::InputInt("CUDA MOG | history", &options.CUDA_MOG.history);
        ImGui::InputInt("CUDA MOG | nMixtures", &options.CUDA_MOG.nmixtures);
        ImGui::InputDouble(" CUDA MOG | Background Ratio", &options.CUDA_MOG.backgroundRatio);
        ImGui::InputDouble(" CUDA MOG | Noise Sigma", &options.CUDA_MOG.noiseSigma);
    }

    void gui_options_CUDA_MOG_2(){
        ImGui::Checkbox("CUDA MOG 2 | Detect Shadows", &options.CUDA_MOG_2.detectShadows);
        ImGui::InputInt("CUDA MOG 2 | history", &options.CUDA_MOG_2.history);
        ImGui::InputDouble(" CUDA MOG 2 | var Threshold", &options.CUDA_MOG_2.varThreshold);
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
                case BGEstimatorType::CNT:      { gui_options_CNT();        break; }
                case BGEstimatorType::GMG:      { gui_options_GMG();        break; }
                case BGEstimatorType::GSOC:     { gui_options_GSOC();       break; }
                case BGEstimatorType::LSBP:     { gui_options_LSBP();       break; }
                case BGEstimatorType::MOG:      { gui_options_MOG();        break; }
                case BGEstimatorType::CUDA_MOG: { gui_options_CUDA_MOG();   break; }
                case BGEstimatorType::CUDA_MOG2:{ gui_options_CUDA_MOG_2(); break; }
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

        if(ImGui::Button("Select Save Directory")){
            fileDialog = std::make_shared<ImGui::FileBrowser>(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_SelectDirectory);
            fileDialog -> Open();
        }


        static std::string results_path = "./";
        static char results_filename[512] = "bg_est_result";
        ImGui::InputTextWithHint("Filename", "filename",results_filename,512);
        ImGui::SameLine();

        if(ImGui::Button("Save Estimator Results")){
            background_est_ -> save_mask(results_path + "/" + results_filename + "_mask.png");
            background_est_ -> save_frame(results_path + "/" + results_filename + "_frame.png");
        }

        gui_switch_estimator();
        gui_show_estirmator_options();

        ImGui::BeginChild("Background");
        image_viewer_bg_ -> show_image(frame_bg_);
        image_viewer_mask_ -> show_image(frame_mask_);
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