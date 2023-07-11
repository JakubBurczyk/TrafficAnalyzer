#pragma once

#include "detection.hpp"
#include "iInference.hpp"

class ObjectDetector{
private:
    std::shared_ptr<IInference> inf;

public:

    std::vector<Detection> detect(const cv::Mat &input) {
        return inf -> runInference(input);
    };

};