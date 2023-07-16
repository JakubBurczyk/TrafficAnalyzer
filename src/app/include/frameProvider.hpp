#pragma once
#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <deque>
#include <algorithm>

#include "utils.hpp"
// #include "imageViewer.hpp"

std::vector<std::string> IMAGE_FORMATS {"jpeg", "jpg", "png"};

#define FP_DEBUG_NAME "FrameProvider | "

class FrameProvider{
private:

    bool ready_ = false;
    bool video_mode_ = false;

    int frame_cnt_ = 0;

    std::string path_ = "./";

    cv::VideoCapture video_capture_;
    std::deque<std::string> videos_;
    std::deque<std::string> images_;

public:

    

    FrameProvider(std::string path)
        :path_(path)
    {
    }

    bool is_ready(){ return ready_; }
    int get_frame_number() { return frame_cnt_; }
    std::string get_path_() { return path_; }

    void set_video_mode(bool mode = false){
        APP_DEBUG(FP_DEBUG_NAME "Setting video mode to: {}", mode);
        video_mode_ = mode; 
    }
    void set_path(std::string path){
        APP_DEBUG(FP_DEBUG_NAME "Setting path to: {}", path);
        path_ = path;
    }

    void start(){
        frame_cnt_ = 0;
        APP_DEBUG(FP_DEBUG_NAME "Initializing | Path: {} | Video Mode: {}" , path_, video_mode_);
        if(video_mode_){
            ready_ = video_capture_.open(path_);
            if(ready_){
                APP_DEBUG(FP_DEBUG_NAME "Video file opened" );
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

    void stop(){
        APP_DEBUG(FP_DEBUG_NAME "Stopping FrameProvider");
        ready_ = false;
        if(video_mode_){
            video_capture_.release();
        }else{
            images_ = std::deque<std::string>{};
        }
    }

    cv::Mat get_next_frame(){
        cv::Mat frame;
        if(video_mode_){
            video_capture_ >> frame;
        }else if(!images_.empty()){
            frame = cv::imread(images_[0]);
            images_.pop_front();
        }
        return frame;
    }
    
};
