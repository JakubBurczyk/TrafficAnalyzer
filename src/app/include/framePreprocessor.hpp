#pragma once
#include <condition_variable>

#include <opencv2/opencv.hpp>

#include "detection.hpp"

class FramePreprocessor{
private:
    bool ready_ = false;

    cv::Mat masked_frame_;
    cv::Mat masked_bg_;

    cv::Mat frame_mask_;
    cv::Mat bg_mask_;

    std::condition_variable cv_frame_;
    std::mutex mtx_notify_frame_;

public:

    FramePreprocessor(){

    }

    bool is_ready(){return ready_; }
    std::condition_variable& get_cv_new_frame(){ return cv_frame_; };
    std::mutex& get_mtx_new_frame(){ return mtx_notify_frame_; };
    cv::Mat get_masked_frame(){ return masked_frame_; }
    
    void set_mask(cv::Mat &mask){
        ready_ = true;
        cv::cvtColor(mask, frame_mask_, cv::ColorConversionCodes::COLOR_GRAY2BGR);
    }

    cv::Mat invert_mask(cv::Mat &mask){
        cv::Mat inverted;
        cv::bitwise_not(mask, inverted);
        return inverted;
    }

    cv::Mat mask_from_detections(std::vector<Detection> detections, cv::Size size){
        cv::Mat mask = cv::Mat(size, CV_8UC1);
        mask.setTo(cv::Scalar(255,255,255));

        for(auto &detection : detections){
            cv::Rect box = detection.box;
            cv::Scalar color = cv::Scalar(0,0,0);

            cv::rectangle(mask, box, color, -1);
        }
        return mask;
    }

    cv::Mat mask_frame(cv::Mat &frame){
        if(frame_mask_.empty()) { return frame; }

        cv::bitwise_and(frame, frame_mask_, masked_frame_);
        cv_frame_.notify_all();
        return masked_frame_;
    }

    cv::Mat mask_frame(cv::Mat &frame, cv::Mat &mask){
        if(mask.empty()) { return frame; }

        cv::cvtColor(mask, frame_mask_, cv::ColorConversionCodes::COLOR_GRAY2BGR);
        cv::bitwise_and(frame, frame_mask_, masked_frame_);
        cv_frame_.notify_all();
        return masked_frame_;
    }

    void remove_mask(){
        frame_mask_ = cv::Mat();
    }

};