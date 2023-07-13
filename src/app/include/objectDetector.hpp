#pragma once

#include "detection.hpp"
#include "inference.h"
#include "frameProvider.hpp"

namespace Traffic{

#define OD_DEBUG_NAME "FrameProvider | "

class ObjectDetector{
private:
    std::string onnx_path_ = "";

    std::shared_ptr<IInference> inf;
    std::shared_ptr<FrameProvider> frame_provider_;

public:

    ObjectDetector(std::shared_ptr<FrameProvider> frame_provider):
        frame_provider_{frame_provider}
    {
        
    }

    std::shared_ptr<FrameProvider> get_frame_provider(){ return frame_provider_; };

    std::vector<Detection> detect(const cv::Mat &input) {
        return inf -> runInference(input);
    };

    void set_onnx_path(std::string &path){
        onnx_path_ = path;
    }

    bool init(){
        bool result = false;
        try{
            inf = std::make_shared<Inference>(onnx_path_, cv::Size(800, 480), "classes.txt", 1);
            APP_DEBUG(OD_DEBUG_NAME "Initialized inference | onnx: {}", onnx_path_);
            result = true;
        }catch(std::exception &e){
            APP_ERROR(OD_DEBUG_NAME "Error inititalizing inference | {}", e.what());
        }
        return result;
    }

};

} // namespace Traffic