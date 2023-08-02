#pragma once

#include <vector>

class HungarianData
{
public:
    std::vector<std::vector<double>> iou_matrix;
    std::vector<int> assignments;
    std::vector<float> costs;
};
