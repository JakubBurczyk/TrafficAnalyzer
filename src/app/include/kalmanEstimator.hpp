#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>

#include "iou.hpp"

namespace Traffic{

struct Measurement{
    float x = 0;
    float y = 0;
};

struct KalmanOptions{
    uint32_t fps = 30;
    float measurement_error = 1;
    Measurement inititial_detection;
};

class KalmanEstimator{
private:

    KalmanOptions options_;
    float dt_ = 0;
    
    cv::KalmanFilter kalman_;

protected:

    static cv::Mat prepare_transition_matrix(float dt);
    static cv::Mat prepare_control_matrix(float dt);
    static cv::Mat prepare_measurement_matrix(Measurement m);
    static cv::Mat prepare_measurement_error_matrix(float x_variance, float y_variance);
    static cv::Mat prepare_process_covariance_matrix(float dt);


    void init_kalman_();

    void set_dt(uint32_t fps){
        dt_ = 1.0f/(float)fps;
    }

    Measurement get_corrected_measurement(){
        Measurement result;
        result.x = kalman_.statePost.at<float>(0,0);
        result.y = kalman_.statePost.at<float>(1,1);
        return result;
    }

public:

    KalmanEstimator(KalmanOptions options)
    :
        options_{options}
    {
        set_dt(options_.fps);
        init_kalman_();

    }

    void update(Measurement m){
        kalman_.predict();
        kalman_.correct(prepare_measurement_matrix(m));
    }
    
};

} // namespace Traffic