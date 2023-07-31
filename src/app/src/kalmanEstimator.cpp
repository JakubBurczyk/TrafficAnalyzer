#include "kalmanEstimator.hpp"

namespace Traffic{

cv::Mat KalmanEstimator::prepare_transition_matrix(float dt){
    float transition_matrix_vectorized[4][4] = {    {1.0f,  0.0f,   dt,     0.0f    },
                                                    {0.0f,  1.0f,   0.0f,   dt      },
                                                    {0.0f,  0.0f,   1.0f,   0.0f    },
                                                    {0.0f,  0.0f,   0.0f,   1.0f    } };

    return cv::Mat(4,4, CV_32F, transition_matrix_vectorized);
}

cv::Mat KalmanEstimator::prepare_control_matrix(float dt){
    float control_matrix_vectorized[4][1] = {   {dt * dt / 2.0f },
                                                {dt * dt / 2.0f },
                                                {dt             },
                                                {dt             } };

    return cv::Mat(4,1, CV_32F, control_matrix_vectorized);
}

cv::Mat KalmanEstimator::prepare_measurement_matrix(Measurement m){
    float measurement_matrix_vectorized[2][4] = {   {1.0f,  0.0f,   0.0f,   0.0f},
                                                    {0.0f,  1.0f,   0.0f,   0.0f} };

    return cv::Mat(2,4, CV_32F, measurement_matrix_vectorized);
}
cv::Mat KalmanEstimator::prepare_measurement(Measurement m){
    float measurement_vectorized[2][1] = {  {m.x },
                                            {m.y } };

    cv::Mat prepared = cv::Mat(2,1, CV_32F, measurement_vectorized);
    std::cout << "PREPARED MEASUREMENT = \n " << prepared << "\n";
    return prepared;
}

cv::Mat KalmanEstimator::prepare_measurement(Measurement m, Measurement initial_state){
    float measurement_vectorized[2][1] = {  {m.x - initial_state.x},
                                            {m.y - initial_state.y} };

    cv::Mat prepared = cv::Mat(2,1, CV_32F, measurement_vectorized);
    // std::cout << "PREPARED MEASUREMENT = \n " << prepared << "\n\n";
    return prepared;
}

cv::Mat KalmanEstimator::prepare_state(Measurement m){

    std::cout << "initial m = "<< m.x << std::endl;
    float state_matrix_vectorized[4][1] = {     {2137.0f    },
                                                {4488.0f    },
                                                {0.0f   },  
                                                {0.0f   } };
    cv::Mat prepared = cv::Mat(4,1, CV_16F, state_matrix_vectorized);
    std::cout << "INITIALIZING STATE TO  = \n " << prepared << "\n\n";
    return prepared;
}


cv::Mat KalmanEstimator::prepare_measurement_error_matrix(float x_variance, float y_variance){
    float measurement_error_matrix_vectorized[2][2] = { {x_variance, 0.0f},
                                                        {0.0f, y_variance} };

    return cv::Mat(2,2, CV_32F, measurement_error_matrix_vectorized);
}

cv::Mat KalmanEstimator::prepare_process_covariance_matrix(float dt){

    float dt2 = dt * dt;
    float dt3 = dt2 * dt;
    float dt4 = dt3 * dt;

    float process_covariance_matrix_vectorized[4][4] = {    {dt4/4.0f,  0.0f,       dt3/2.0f,   0.0f        },
                                                            {0.0f,      dt4/4.0f,   0.0f,       dt3/2.0f    },
                                                            {dt3/2.0f,  0.0f,       dt2,        0.0f        },
                                                            {0.0f,      dt3/2.0f,   0.0f,       dt2         } };

    return cv::Mat(4,4, CV_32F, process_covariance_matrix_vectorized);
}



void KalmanEstimator::init_kalman_(){
    std::cout << "Initializing Kalman Filter\n";
    kalman_.init(4,2,0, CV_32F);
    // kalman_.transitionMatrix = prepare_transition_matrix(dt_);
    // kalman_.controlMatrix = prepare_control_matrix(dt_);
    // kalman_.measurementMatrix = prepare_measurement_matrix(Measurement({1.0f, 1.0f}));
    // kalman_.measurementNoiseCov = prepare_measurement_error_matrix(options_.measurement_error, options_.measurement_error);
    // kalman_.processNoiseCov = prepare_process_covariance_matrix(dt_);
    // kalman_.statePre = prepare_state(options_.inititial_measurement);
    // kalman_.statePost = prepare_state(options_.inititial_measurement);

    kalman_.transitionMatrix.at<float>(0,0) = 1.0f;
    kalman_.transitionMatrix.at<float>(1,1) = 1.0f;
    kalman_.transitionMatrix.at<float>(2,2) = 1.0f;
    kalman_.transitionMatrix.at<float>(3,3) = 1.0f;
    kalman_.transitionMatrix.at<float>(0,2) = dt_;
    kalman_.transitionMatrix.at<float>(1,3) = dt_;

    kalman_.measurementMatrix.at<float>(0,0) = 1.0f;
    kalman_.measurementMatrix.at<float>(1,1) = 1.0f;

    kalman_.statePre.at<float>(0,0) = options_.inititial_measurement.x;
    kalman_.statePre.at<float>(1,0) = options_.inititial_measurement.y;

    // std::cout << "INITIALIZED PREDICTION TO:\n" << kalman_.statePre << "\n------------------\n";
    // std::cout << "INITIALIZED TRANSITION TO:\n" << kalman_.transitionMatrix << "\n------------------\n";
    // std::cout << "INITIALIZED MEASUREMENT TO:\n" << kalman_.measurementMatrix << "\n------------------\n";
}

} // namespace Traffic