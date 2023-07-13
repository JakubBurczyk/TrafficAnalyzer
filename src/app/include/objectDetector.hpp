#pragma once

#include "detection.hpp"
#include "iInference.hpp"
#include "frameProvider.hpp"

namespace Traffic{

class ObjectDetector{
private:
    std::shared_ptr<IInference> inf;
    std::shared_ptr<FrameProvider> frame_provider_;

public:

    ObjectDetector(std::shared_ptr<FrameProvider> frame_provider):
        frame_provider_{frame_provider}
    {
        
    }

    std::vector<Detection> detect(const cv::Mat &input) {
        return inf -> runInference(input);
    };

    std::shared_ptr<FrameProvider> get_frame_provider(){ return frame_provider_; };

};

} // namespace Traffic