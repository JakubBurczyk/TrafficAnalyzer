#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>

#include "iou.hpp"

namespace Traffic{

struct Measurement{
    float x = -1;
    float y = -1;
};

struct KalmanOptions{
    uint32_t fps = 30;
    float measurement_error = 1;
    Measurement inititial_measurement;
};

class KalmanEstimator{
private:

    KalmanOptions options_;
    float dt_ = 0;
    
    cv::KalmanFilter kalman_ = cv::KalmanFilter(4, 2, 4);

    uint32_t pure_prediction_coutner = 0;

protected:

    static cv::Mat prepare_transition_matrix(float dt);
    static cv::Mat prepare_control_matrix(float dt);
    static cv::Mat prepare_measurement_matrix(Measurement m);
    static cv::Mat prepare_measurement_error_matrix(float x_variance, float y_variance);
    static cv::Mat prepare_process_covariance_matrix(float dt);
    static cv::Mat prepare_measurement(Measurement m);

    void init_kalman_();

    void set_dt(uint32_t fps){
        dt_ = 1.0f/(float)fps;
    }

    void increment_prediction_counter(){
        pure_prediction_coutner++;
    }

    void reset_prediction_counter(){
        pure_prediction_coutner = 0;
    }

    void predict(){
        // std::cout << "KALMAN PREDICTION STEP\n"; 
        kalman_.predict();
        increment_prediction_counter();
    }

public:

    KalmanEstimator(KalmanOptions options)
    :
        options_{options}
    {
        set_dt(options_.fps);
        init_kalman_();

    }

    uint32_t get_prediction_counter(){ return pure_prediction_coutner; }

    void update(Measurement m){        
        predict();
        if(m.x < 0 || m.y < 0) { return; }
        // std::cout << "KALMAN CORRECTION STEP\n"; 
        kalman_.correct(prepare_measurement(m));
        reset_prediction_counter();
    }

    Measurement get_corrected_measurement(){
        // std::cout << "GETTING CORRECTED MEASUREMENT\n"; 
        Measurement result;
        result.x = kalman_.statePost.at<float>(0,0);
        result.y = kalman_.statePost.at<float>(1,1);
        // std::cout << "GOT CORRECTED MEASUREMENT\n"; 
        return result;
    }
    
};

} // namespace Traffic