#pragma once
#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <deque>

#include "utils.hpp"

std::vector<std::string> IMAGE_FORMATS {"jpeg", "jpg", "png"};

class FrameProvider{
private:
    bool ready_ = false;
    bool is_video_ = false;

    std::string path_ = "./";

    std::unique_ptr<cv::VideoCapture> video_capture_;
    std::deque<std::string> videos_;
    std::deque<std::string> images_;

    void set_video_mode(bool mode = false){ is_video_ = mode; }

    void set_video_capture(std::string path){
        video_capture_ = std::make_unique<cv::VideoCapture>(path);
        ready_ = video_capture_ -> isOpened();
    };

    FrameProvider(std::string path)
        :path_(path)
    {
        
    }

public:


    static FrameProvider VideoFrameProvider(std::string files_path){
        auto frame_provider = FrameProvider(files_path);
        frame_provider.set_video_mode(true);
        frame_provider.set_video_capture(files_path);

        return frame_provider;
    }

    static FrameProvider ImageFrameProvider(std::string files_path){
        auto frame_provider = FrameProvider(files_path);
        frame_provider.set_video_mode(false);
        
        std::vector<std::string> imgs = utils::get_files_by_extensions(files_path, IMAGE_FORMATS);
        // images_ = std::deque(imgs.begin(), imgs.end());
            
        return frame_provider;
    }

    cv::Mat get_next_frame(){
        cv::Mat frame;
        if(is_video_){
            *video_capture_.get() >> frame;
        }else{
            frame = cv::imread(images_[0]);
            images_.pop_front();
        }
        return frame;
    }

};
