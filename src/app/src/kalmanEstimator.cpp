#include "kalmanEstimator.hpp"

namespace Traffic{

cv::Mat KalmanEstimator::prepare_transition_matrix(float dt){
    float transition_matrix_vectorized[4][4] = {    {1, 0, dt,  0  },
                                                    {0, 1, 0,   dt },
                                                    {0, 0, 1,   0  },
                                                    {0, 0, 0,   1  } };

    return cv::Mat(4,4, CV_32FC1, transition_matrix_vectorized);
}

cv::Mat KalmanEstimator::prepare_control_matrix(float dt){
    float control_matrix_vectorized[4][1] = {   {dt * dt / 2},
                                                {dt * dt / 2},
                                                {dt         },
                                                {dt         } };

    return cv::Mat(4,1, CV_32FC1, control_matrix_vectorized);
}

cv::Mat KalmanEstimator::prepare_measurement_matrix(Measurement m){
    float measurement_matrix_vectorized[2][4] = {   {m.x,   0,      0,  0},
                                                    {0,     m.y,    0,  0} };

    return cv::Mat(2,4, CV_32FC1, measurement_matrix_vectorized);
}
cv::Mat KalmanEstimator::prepare_measurement(Measurement m){
    float measurement_vectorized[2][1] = {  {m.x},
                                            {m.y} };

    return cv::Mat(2,1, CV_32FC1, measurement_vectorized);
}

cv::Mat KalmanEstimator::prepare_measurement_error_matrix(float x_variance, float y_variance){
    float measurement_error_matrix_vectorized[2][2] = { {x_variance, 0},
                                                        {0, y_variance} };

    return cv::Mat(2,2, CV_32FC1, measurement_error_matrix_vectorized);
}

cv::Mat KalmanEstimator::prepare_process_covariance_matrix(float dt){

    float dt2 = dt * dt;
    float dt3 = dt2 * dt;
    float dt4 = dt3 * dt;

    float process_covariance_matrix_vectorized[4][4] = {    {dt4/4, 0,      dt3/2,  0       },
                                                            {0,     dt4/4,  0,      dt3/2   },
                                                            {dt3/2, 0,      dt2,    0       },
                                                            {0,     dt3/2,  0,      dt2     } };

    return cv::Mat(4,4, CV_32FC1, process_covariance_matrix_vectorized);
}

void KalmanEstimator::init_kalman_(){
    kalman_.transitionMatrix = prepare_transition_matrix(dt_);
    kalman_.controlMatrix = prepare_control_matrix(dt_);
    kalman_.measurementMatrix = prepare_measurement_matrix(Measurement({1, 1}));
    kalman_.measurementNoiseCov = prepare_measurement_error_matrix(options_.measurement_error, options_.measurement_error);
    kalman_.processNoiseCov = prepare_process_covariance_matrix(dt_);
    // kalman_.statePre = prepare_measurement_matrix(options_.inititial_measurement);
    // kalman_.statePost = prepare_measurement_matrix(options_.inititial_measurement);
}

} // namespace Traffic