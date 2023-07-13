#pragma once

#include "detection.hpp"

class IInference{
private:

public:
    virtual ~IInference() = default;

    virtual std::vector<Detection> runInference(const cv::Mat &input) = 0;
};