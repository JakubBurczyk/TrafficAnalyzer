#pragma once
#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <deque>
#include <algorithm>
#include <condition_variable>
#include "utils.hpp"
// #include "imageViewer.hpp"

std::vector<std::string> IMAGE_FORMATS {"jpeg", "jpg", "png"};

#define FP_DEBUG_NAME "FrameProvider | "

class FrameProvider{
private:
    
    

    bool ready_ = false;
    bool video_mode_ = false;

    int frame_cnt_ = 0;

    std::string path_ = "";
    bool path_initialized_ = false;

    cv::VideoCapture video_capture_;
    std::deque<std::string> videos_;
    std::deque<std::string> images_;

    cv::Mat frame_;

    std::chrono::system_clock::time_point last_frame_read_;
    std::chrono::milliseconds ms_since_last_frame_{0};

public:
    std::condition_variable cv_frame_;
    std::mutex mtx_notify_frame_;

    FrameProvider(std::string path)
        :path_(path)
    {
    }

    bool is_ready(){ return ready_; }
    bool has_path_() { return path_initialized_; }

    int get_frame_number() { return frame_cnt_; }
    std::string get_path_() { return path_; }
    std::chrono::milliseconds get_ms_since_read(){ return ms_since_last_frame_; }

    const cv::Mat& get_const_frame_ref(){ return frame_; }
    cv::Mat& get_frame_ref(){ return frame_; }
    cv::Mat get_frame(){ return frame_; }

    void set_video_mode(bool mode = false){
        APP_DEBUG(FP_DEBUG_NAME "Setting video mode to: {}", mode);
        video_mode_ = mode; 
    }
    void set_path(std::string path){
        APP_DEBUG(FP_DEBUG_NAME "Setting path to: {}", path);
        path_ = path;
        path_initialized_ = true;
    }

    void start(){
        stop();
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

        //Initialize first frame
        if(ready_ ){
            next_frame();
        }
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

    bool next_frame(){
        if(video_mode_){
            video_capture_ >> frame_;
        }else if(!images_.empty()){
            frame_ = cv::imread(images_[0]);
            images_.pop_front();
        }
        
        if(!frame_.empty()){
            frame_cnt_++;
            auto now = std::chrono::system_clock::now();
            auto elapsed = now - last_frame_read_;
            last_frame_read_ = now;
            ms_since_last_frame_ = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
            
        }
        cv_frame_.notify_all();
        return !frame_.empty();
    }

    
    
};
