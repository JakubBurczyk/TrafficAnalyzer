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

    std::vector<Detection> detections_prev_;
    std::vector<Detection> detections_;

    std::condition_variable cv_detections_;
    std::mutex mtx_notify_detections_;

    cv::Mat frame_;

public:

    ObjectDetector()
    {
        
    }

    bool is_ready(){ return ready_; }
    bool get_CUDA_mode() { return CUDA_; }

    const std::string& get_model_path(){ return onnx_path_; }
    std::vector<Detection> get_detections(){ return detections_; }
    std::condition_variable& get_cv_detections(){ return cv_detections_; }
    std::mutex& get_mtx_detections(){ return mtx_notify_detections_; }


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

    void detect(const cv::Mat &frame) {
        frame_ = frame.clone();
        detections_prev_ = detections_;
        detections_ = inf -> runInference(frame_);
        cv_detections_.notify_all();
    };

    cv::Mat visualize(){
        cv::Mat visualization_frame = frame_.clone();
        for(auto &detection : detections_){
            cv::Rect box = detection.box;
            cv::Scalar color = cv::Scalar(0,0,255);

            cv::rectangle(visualization_frame, box, color, 2);
        }
        return visualization_frame;
    }

};

} // namespace Traffic