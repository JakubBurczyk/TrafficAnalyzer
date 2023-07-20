#pragma once

#include "Core/Application.hpp"
#include "Core/Instrumentor.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <imgui.h>

#include "frameProvider.hpp"
#include "objectDetector.hpp"
#include "backgroundEstimator.hpp"

namespace Traffic{

class TrafficAnalyzer{
private:
    std::mutex mtx_run_;

    bool analyzer_initialized_ = false;
    bool background_est_initialized_ = false;

    bool run_ = false;
    bool previous_run_ = false;

    std::shared_ptr<ObjectDetector> detector_;
    std::shared_ptr<FrameProvider> frame_provider_;
    std::shared_ptr<BackgroundEstimator> background_est_;

    cv::Mat frame_;

    std::thread analyzer_thread_;
    std::thread background_est_thread_;

    std::condition_variable cv_update_;
    std::mutex mtx_notify_update_;
    
protected:
    bool start(bool &thread_prev_initialized, std::function<void(void)> join_callback);
    void run();

    void create_analyzer_thread();
    void create_background_est_thread();

    void join_analyzer_thread();
    void join_background_est_thread();

    bool run_detector(bool run);
    bool run_background_est(bool run);
    bool advance_frame();

    bool update_analyzer(){
        bool result = false;

        result = advance_frame();
        result = run_detector(result);

        return result;
    }

    bool update_background_est(){
        bool result = false;
        for(int i = 0; i<10; i++){
            result = advance_frame();
        }
        
        if(result){
            background_est_ -> update(frame_);
        }
        return result;
    }   


public:

    TrafficAnalyzer(std::shared_ptr<FrameProvider> frame_provider, 
                    std::shared_ptr<ObjectDetector> detector,
                    std::shared_ptr<BackgroundEstimator> background);

    std::shared_ptr<FrameProvider> get_frame_provider(){ return frame_provider_; }
    std::shared_ptr<ObjectDetector> get_object_detector(){ return detector_; }
    std::shared_ptr<BackgroundEstimator> get_background_estimator(){ return background_est_; }

    bool is_running();

    bool start_analyzer();
    bool start_background_est();

    bool stop();
    void reset_source();
    
};

} // namespace Traffic
