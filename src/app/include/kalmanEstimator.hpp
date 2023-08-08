#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>

#include "iou.hpp"

namespace Traffic{

struct Measurement{
    float x = -1;
    float y = -1;
    float v_x = 0;
    float v_y = 0;

    float velocity(){
        return std::sqrt(v_x * v_x + v_y * v_y);
    }
};

struct KalmanOptions{
    uint32_t fps = 60;
    float measurement_error = 1;
    Measurement inititial_measurement;
};

class KalmanEstimator{
private:
    bool pure_prediction_  = false;

    KalmanOptions options_;
    float dt_ = 0;
    
    cv::KalmanFilter kalman_;

    uint32_t pure_prediction_coutner = 0;

protected:

    static cv::Mat prepare_transition_matrix(float dt);
    static cv::Mat prepare_control_matrix(float dt);
    static cv::Mat prepare_measurement_matrix(Measurement m);
    static cv::Mat prepare_measurement_error_matrix(float x_variance, float y_variance);
    static cv::Mat prepare_process_covariance_matrix(float dt);
    static cv::Mat prepare_measurement(Measurement m);
    static cv::Mat prepare_measurement(Measurement m, Measurement initial_state);
    static cv::Mat prepare_state(Measurement m);

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
        // std::cout << "Kalman PRE = \n " << kalman_.statePre << "\n";
        increment_prediction_counter();
    }

    void correct(const Measurement &m){
        if(m.x < 0 || m.y < 0) {
            pure_prediction_ = true;
            
            kalman_.statePost = kalman_.statePre.clone();

            float x_predicted = kalman_.statePost.at<float>(0);
            float y_predicted = kalman_.statePost.at<float>(1);

            kalman_.statePost.at<float>(0) = x_predicted * (x_predicted > 0);
            kalman_.statePost.at<float>(1) = y_predicted * (y_predicted > 0);

            // std::cout<< "No correction applied! |" <<  x_predicted << " | " << y_predicted<<"\n";
            return;
        }
        //reset velocities after re-ack of detection
        if(pure_prediction_){
            pure_prediction_ = false;
            kalman_.statePost.at<float>(3) = 0;
            kalman_.statePost.at<float>(4) = 0;
        }

        cv::Mat measurement_matrix = cv::Mat(2,1,CV_32F,0.0);
        measurement_matrix.at<float>(0,0) = m.x;
        measurement_matrix.at<float>(1,0) = m.y;
        auto correction = kalman_.correct(measurement_matrix);

        reset_prediction_counter();
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
        // std::cout << "\n---------------------------------\nMeasurement | x: " << m.x << " | y: " << m.x << " | \n"; 
        predict();
        correct(m);

        // cv::Mat measurement_matrix = prepare_measurement(m,options_.inititial_measurement);
        
        
        // std::cout << "-----------------------------------------------------\n";
        // std::cout << "Kalman CORRECTION = \n " << correction << "\n";
        // std::cout << "-----------------------------------------------------\n";
        // std::cout << "Kalman PRE = \n " << kalman_.statePre << "\n";
        // std::cout << "-----------------------------------------------------\n";
        // std::cout << "Kalman POST = \n " << kalman_.statePost << "\n\n";
        
    }

    Measurement get_corrected_measurement(){

        Measurement result;
        result.x = kalman_.statePost.at<float>(0,0);
        result.y = kalman_.statePost.at<float>(1,0);
        result.v_x = kalman_.statePost.at<float>(2,0);
        result.v_y = kalman_.statePost.at<float>(3,0);
        return result;
    }
    
};

} // namespace Traffic