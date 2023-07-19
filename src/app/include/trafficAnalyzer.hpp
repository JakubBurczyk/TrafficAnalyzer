#pragma once
#include "Core/Application.hpp"
#include "Core/Instrumentor.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <imgui.h>


#include "objectDetector.hpp"

namespace Traffic{


class TrafficAnalyzer{
private:
    std::mutex mtx_run_;
    bool run_ = false;

    std::shared_ptr<ObjectDetector> detector_;
    std::shared_ptr<FrameProvider> frame_provider_;

    cv::Mat frame_;

    std::thread analyzer_thread_;

    std::condition_variable cv_update_;
    std::mutex mtx_notify_update_;
    
protected:


public:

    TrafficAnalyzer(std::shared_ptr<FrameProvider> frame_provider, std::shared_ptr<ObjectDetector> detector)
    :
        frame_provider_{frame_provider}
        ,detector_{detector}
    {

    }

    bool is_running(){
        std::unique_lock<std::mutex> lock(mtx_run_);
        return run_;
    }

    void run_bg(){
        if(!run_){
            run_ = true;

            analyzer_thread_ = std::thread([this](){
                while(true){
                    std::cout << "Loop\n"; 
                    if(!is_running()){ return; }{}

                    update();
                }
            });
        }
        
    }

    void join(){
        analyzer_thread_.join();
    }

    bool start(){
        std::unique_lock<std::mutex> lock(mtx_run_);
        bool previous_run_ = run_;
        return !previous_run_;
    }

    bool stop(){
        std::unique_lock<std::mutex> lock(mtx_run_);
        bool previous_run_ = run_;
        run_ = false;
        std::cout << "STOPPING" << std::endl;
        return previous_run_;
    }

    bool run_detector(){
        bool result = detector_ -> is_ready();

        if(result){
            detector_ -> detect(frame_);
        }

        return result;
    }

    bool advance_frame(){
        bool result = frame_provider_ -> next_frame();
        if(result){
            std::cout << "Getting frame" << std::endl;
            frame_ = frame_provider_ -> get_frame();
        }
        return result;
    }

    bool update(){
        std::cout << "Update\n";
        bool result = false;

        result = advance_frame();
        result = run_detector();

        return result;
        
    }


    std::shared_ptr<FrameProvider> get_frame_provider(){ return frame_provider_; };
    std::shared_ptr<ObjectDetector> get_object_detector(){ return detector_; };
    
};

} // namespace Traffic