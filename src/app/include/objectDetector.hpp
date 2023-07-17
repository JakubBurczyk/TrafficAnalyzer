#pragma once

#include "detection.hpp"
#include "inference.h"
#include "frameProvider.hpp"

namespace Traffic{

#define OD_DEBUG_NAME "ObjectDetector | "

class ObjectDetector{
private:
    std::string onnx_path_ = "NONE";
    bool CUDA_ = false;
    bool ready_ = false;
    std::shared_ptr<IInference> inf;
    std::shared_ptr<FrameProvider> frame_provider_;

    std::vector<Detection> detections_prev_;
    std::vector<Detection> detections_;



public:

    ObjectDetector(std::shared_ptr<FrameProvider> frame_provider):
        frame_provider_{frame_provider}
    {
        
    }

    bool is_ready(){ return ready_; }
    bool get_CUDA_mode() { return CUDA_; }
    std::shared_ptr<FrameProvider> get_frame_provider(){ return frame_provider_; }
    const std::string& get_model_path(){ return onnx_path_; }
    std::vector<Detection> get_detections(){ return detections_; }

    void set_onnx_path(std::string path){ onnx_path_ = path; }
    void toggle_CUDA_mode_(){ CUDA_ = !CUDA_; }

    bool init(){
        ready_ = false;
        try{
            inf = std::make_shared<Inference>(onnx_path_, cv::Size(800, 480), "classes.txt", CUDA_);
            APP_DEBUG(OD_DEBUG_NAME "Initialized inference | onnx: {}", onnx_path_);
            ready_ = true;
        }catch(std::exception &e){
            APP_ERROR(OD_DEBUG_NAME "Error inititalizing inference | {}", e.what());
        }
        return ready_;
    }

    void detect(cv::Mat &frame) {
        
        detections_prev_ = detections_;
        APP_DEBUG(OD_DEBUG_NAME "Running NN [{}x{}]",frame.cols, frame.rows);
        detections_ = inf -> runInference(frame);
        APP_DEBUG(OD_DEBUG_NAME "Detected {} objects", detections_.size());
    };

    cv::Mat visualize(const cv::Mat &frame){
        APP_DEBUG(OD_DEBUG_NAME "Running visualization");
        cv::Mat visualization_frame = frame.clone();
        for(auto &detection : detections_){
            cv::Rect box = detection.box;
            cv::Scalar color = detection.color;

            cv::rectangle(visualization_frame, box, color, 2);
        }
        APP_DEBUG(OD_DEBUG_NAME "Visualized {} objects", detections_.size());
        return visualization_frame;
    }



};

} // namespace Traffic