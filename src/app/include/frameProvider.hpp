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
    
    bool running_ = false;
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

    std::condition_variable cv_frame_;
    std::mutex mtx_notify_frame_;

public:
    

    FrameProvider(std::string path)
        :path_(path)
    {
    }

    bool is_ready(){ return ready_; }
    bool is_running() { return running_; }
    bool has_path_() { return path_initialized_; }

    int get_frame_number() { return frame_cnt_; }
    std::string get_path_() { return path_; }
    std::chrono::milliseconds get_ms_since_read(){ return ms_since_last_frame_; }

    const cv::Mat& get_const_frame_ref(){return frame_; }
    cv::Mat& get_frame_ref(){ return frame_; }
    cv::Mat get_frame(){ return frame_; }

    std::condition_variable& get_cv_new_frame(){ return cv_frame_; };
    std::mutex& get_mtx_new_frame(){ return mtx_notify_frame_; };

    void set_video_mode(bool mode = false){
        video_mode_ = mode; 
    }
    void set_path(std::string path){
        path_ = path;
        path_initialized_ = true;
    }


    void start(){
        if(running_){ return; }
        std::cout << "FRAME PROVIDER STARTED\n";
        
        frame_cnt_ = 0;
        if(video_mode_){
            ready_ = video_capture_.open(path_);
        }
        else{
            auto imgs = utils::get_files_by_extensions(path_,IMAGE_FORMATS);
            images_ = std::deque(imgs.begin(), imgs.end());
            ready_ = !images_.empty();
        }

        //Initialize first frame
        if(ready_ ){
            running_ = true;
            next_frame();
        }
    }

    void pause(){
        running_ = false;
    }

    void reset(){
        stop();
        start();
        
    }

    void stop(){
        pause();
        
        if(video_mode_){
            video_capture_.release();
            ready_ = video_capture_.isOpened();
        }else{
            images_ = std::deque<std::string>{};
            ready_ = images_.size() == 0;
        }
    }

    bool next_frame(){
        if(!running_){ return false; }

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
