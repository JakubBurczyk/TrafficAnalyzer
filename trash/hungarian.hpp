#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

class HungarianSolver{
private:

    

public:
    static void solve(cv::Mat weights){
    }


    static void reduce_rows(cv::Mat &weights){
        cv::Mat mins;
        cv::reduce(weights, mins, 1, cv::ReduceTypes::REDUCE_MIN);

        std::cout << "Mins = " << std::endl << " "  << mins << std::endl << std::endl;

        for (int r = 0; r < weights.rows; ++r) {
            weights.row(r) = weights.row(r) - mins.row(r).at<float>(0);
        }
    }

    static void reduce_cols(cv::Mat &weights){
        cv::Mat mins;
        cv::reduce(weights, mins, 0, cv::ReduceTypes::REDUCE_MIN);

        std::cout << "Mins = " << std::endl << " "  << mins << std::endl << std::endl;

        for (int c = 0; c < weights.cols; ++c) {
            weights.col(c) = weights.col(c) - mins.col(c).at<float>(0);
        }
    }

};