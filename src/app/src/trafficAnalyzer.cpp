#include "trafficAnalyzer.hpp"

namespace Traffic{

TrafficAnalyzer::TrafficAnalyzer(   std::shared_ptr<FrameProvider>          frame_provider,
                                    std::shared_ptr<FramePreprocessor>      frame_preprocessor,
                                    std::shared_ptr<ObjectDetector>         detector,
                                    std::shared_ptr<BackgroundEstimator>    background,
                                    std::shared_ptr<TrafficTracker>         tracker,
                                    std::shared_ptr<TrajectoryGenerator>    trajectory,
                                    std::shared_ptr<CollisionEstimator>     collision
                                    )
:
    frame_provider_{frame_provider},
    frame_preprocessor_{frame_preprocessor},
    detector_{detector},
    background_est_{background},
    tracker_{tracker},
    trajectory_{trajectory},
    collision_{collision}
{

}

bool TrafficAnalyzer::is_running(){
    std::unique_lock<std::mutex> lock(mtx_run_);
    return run_;
}

void TrafficAnalyzer::run(){
    std::unique_lock<std::mutex> lock(mtx_run_);
    run_ = true;
}


void TrafficAnalyzer::reset_source(){
    stop();
    frame_provider_ -> reset();
}

bool TrafficAnalyzer::stop()
{
    std::unique_lock<std::mutex> lock(mtx_run_);
    run_ = false;
    std::cout << "STOPPING" << std::endl;
    return previous_run_;
}

void TrafficAnalyzer::start_processor(PROCESSING_TYPE type){
    bool running = is_running();
    auto &processor = processors_[type];

    if(running){
        stop();
        
    }else{
        run();
    }

    join_processor(type);
    create_processor(type);
}

void TrafficAnalyzer::create_processor(PROCESSING_TYPE type){
    auto &processor = processors_[type];
    std::function update_callback = processor_updates_[type];
    processor.initialized = true;
    processor.thr = std::thread([this, update_callback](){
        while(true){
            if(!is_running()){ break; }
            update_callback();
        }
        return;
    });
}

void TrafficAnalyzer::join_processor(PROCESSING_TYPE type){
        auto &processor = processors_[type];
        if(processor.initialized){
            if(processor.thr.joinable()){
                processor.thr.join();
            }
        }
    }

bool TrafficAnalyzer::update_detector(bool run)
{
    if(!run){ return false;}

    bool result = detector_ -> is_ready();

    if(result){
        detector_ -> detect(frame_);
    }

    return result;
}

bool TrafficAnalyzer::update_background_est(bool run)
{
    if(!run){ return false;}
    bool result = false;
    
    background_est_ -> update(frame_);
    result = true;

    return result;
}

bool TrafficAnalyzer::update_trajectory(bool run){
    if(!run){ return false;}
    bool result = false;

    trajectory_ -> update(tracker_ -> get_tracklets());
    return result;
}

bool TrafficAnalyzer::update_tracker(){

    bool result = false;

    cv::Mat frame = detector_ -> visualize();
    tracker_ -> update(detector_ -> get_detections(), frame);
    result = true;
    
    return result;
}

bool TrafficAnalyzer::update_collisions(){
    bool result = false;

    collision_ -> update(tracker_ -> get_tracklets(), frame_);

    result = true;
    return result;
}



bool TrafficAnalyzer::advance_frame()
{
    bool result = frame_provider_ -> next_frame();
    if(result){
        frame_ = frame_provider_ -> get_frame();
    }
    return result;
}

bool TrafficAnalyzer::mask_frame(){
    frame_ = frame_preprocessor_ -> mask_frame(frame_);

    return true;
}


} // namespace Traffic