#include "trafficAnalyzer.hpp"

namespace Traffic{

TrafficAnalyzer::TrafficAnalyzer(   std::shared_ptr<FrameProvider> frame_provider,
                                    std::shared_ptr<FramePreprocessor> frame_preprocessor,
                                    std::shared_ptr<ObjectDetector> detector,
                                    std::shared_ptr<BackgroundEstimator> background)
:
    frame_provider_{frame_provider},
    frame_preprocessor_{frame_preprocessor},
    detector_{detector},
    background_est_{background}
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

void TrafficAnalyzer::create_analyzer_thread()
{
    std::cout << "Creating analyzer thread" << std::endl;
    analyzer_thread_ = std::thread([this](){
        while(true){
            if(!is_running()){ break; }

            update_analyzer();
        }
        std::cout << "Analyzer thread done\n";
        return;
    });
}

void TrafficAnalyzer::create_background_est_thread()
{
    std::cout << "Creating background estimator thread" << std::endl;
    background_est_thread_ = std::thread([this](){
        while(true){
            if(!is_running()){ break; }

            update_background_est();
        }
        std::cout << "Background estimator  thread done\n";
        return;
    });
}

void TrafficAnalyzer::join_analyzer_thread()
{        
    if(analyzer_thread_.joinable()){
        std::cout << "Joining analyzer thread" << std::endl;
        analyzer_thread_.join();
    }
}

void TrafficAnalyzer::join_background_est_thread()
{        
    if(background_est_thread_.joinable()){
        std::cout << "Joining analyzer thread" << std::endl;
        background_est_thread_.join();
    }
}

bool TrafficAnalyzer::start(bool &thread_prev_initialized, std::function<void(void)> join_callback){
    bool running = is_running();

    if(running){
        stop();
    }else if(thread_prev_initialized){
        join_callback();
    }

    return !running;
}

bool TrafficAnalyzer::start_analyzer()
{
    start(  analyzer_initialized_,
            std::bind(&TrafficAnalyzer::join_analyzer_thread, this));
    
    analyzer_initialized_ = true;

    run();
    create_analyzer_thread();

    return !previous_run_;
}

bool TrafficAnalyzer::start_background_est()
{
    start(  background_est_initialized_,
            std::bind(&TrafficAnalyzer::join_background_est_thread, this));

    background_est_initialized_ = true;

    run();
    create_background_est_thread();

    return !previous_run_;
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

bool TrafficAnalyzer::run_detector(bool run)
{
    if(!run){ return false;}

    bool result = detector_ -> is_ready();

    if(result){
        detector_ -> detect(frame_);
    }

    return result;
}

bool TrafficAnalyzer::run_background_est(bool run)
{
    if(!run){ return false;}
    bool result = false;
    
    background_est_ -> update(frame_);
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