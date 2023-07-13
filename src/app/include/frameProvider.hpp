#pragma once
#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <deque>

#include "utils.hpp"

std::vector<std::string> IMAGE_FORMATS {"jpeg", "jpg", "png"};

#define FP_DEBUG_NAME "FrameProvider | "

class FrameProvider{
private:

    bool ready_ = false;
    bool video_mode_ = false;

    std::string path_ = "./";

    std::unique_ptr<cv::VideoCapture> video_capture_;
    std::deque<std::string> videos_;
    std::deque<std::string> images_;

    void set_video_capture(std::string path){
        APP_DEBUG(FP_DEBUG_NAME "Setting video capture to: {}", path);
        video_capture_ = std::make_unique<cv::VideoCapture>(path);
    };

public:

    FrameProvider(std::string path)
        :path_(path)
    {
    }

    bool is_ready(){ return ready_; }

    void set_video_mode(bool mode = false){
        APP_DEBUG(FP_DEBUG_NAME "Setting video mode to: {}", mode);
        video_mode_ = mode; 
    }
    void set_path(std::string path){
        APP_DEBUG(FP_DEBUG_NAME "Setting path to: {}", path);
        path_ = path;
    }

    void init(){
        APP_DEBUG(FP_DEBUG_NAME "Initializing | Path: {} | Video Mode: {}" , path_, video_mode_);
        if(video_mode_){
            set_video_capture(path_);
            ready_ = video_capture_ -> isOpened();
            if(ready_){
                APP_DEBUG(FP_DEBUG_NAME "Video file opened" , images_.size());
            }
        }
        else{
            auto imgs = utils::get_files_by_extensions(path_,IMAGE_FORMATS);
            images_ = std::deque(imgs.begin(), imgs.end());
            ready_ = !images_.empty();
            if(ready_){
                APP_DEBUG(FP_DEBUG_NAME "Detected [{}] images" , images_.size());
            }
        }
        
        APP_DEBUG(FP_DEBUG_NAME "Ready state: {}" , ready_);
    }

    cv::Mat get_next_frame(){
        cv::Mat frame;
        if(video_mode_){
            *video_capture_.get() >> frame;
        }else if(!images_.empty()){
            frame = cv::imread(images_[0]);
            images_.pop_front();
        }
        return frame;
    }

};
