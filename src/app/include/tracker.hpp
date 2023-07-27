#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>

#include "iou.hpp"
#include "Hungarian.h"

/*

	vector< vector<double> > costMatrix = { {1,2},{2,1},{0,3} };

	HungarianAlgorithm HungAlgo;
	vector<int> assignment;

	double cost = HungAlgo.Solve(costMatrix, assignment);

	for (unsigned int x = 0; x < costMatrix.size(); x++)
		std::cout << x << ", assigned job:" << assignment[x] << "\t";

	std::cout << "\ncost: " << cost << std::endl;

*/

namespace Traffic{

struct TrackerOptions{
    uint32_t fps = 30;
    float measurement_error = 1;
};

class Tracker{
private:

    TrackerOptions options_;
    float dt_ = 0;
    
    cv::KalmanFilter kalman_;

protected:

    static cv::Mat prepare_transition_matrix(float dt);
    static cv::Mat prepare_control_matrix(float dt);
    static cv::Mat prepare_measurement_matrix();
    static cv::Mat prepare_measurement_error_matrix(float x_variance, float y_variance);
    static cv::Mat prepare_process_covariance_matrix(float dt);

    void init_kalman_();

    void set_dt(uint32_t fps){
        dt_ = 1.0f/(float)fps;
    }

public:

    Tracker(TrackerOptions options)
    :
        options_{options}
    {
        set_dt(options_.fps);
        init_kalman_();
    }
    
};

} // namespace Traffic