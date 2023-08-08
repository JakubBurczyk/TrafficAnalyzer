#pragma once

#include <condition_variable>

#include "tracklet.hpp"

namespace Traffic{

struct TrajectoryOptions{

    
    uint32_t frame_width = 1920;
    uint32_t frame_height = 1080;
    uint32_t fps = 60;
    float dt = 1/fps;

    uint32_t smoothing_radius = 10;


    bool generate_absolute_velocity = true;
    bool generate_separated_velocity = true;

    bool smoothing = false;

};

class TrajectoryGenerator{
private:
    TrajectoryOptions options_;

    std::mutex mtx_trajectories_;
    std::condition_variable cv_update_trajectories;

    cv::Mat presence_count_channel_;
    cv::Mat velocity_abs_channel_;
    cv::Mat velocity_x_channel_;
    cv::Mat velocity_y_channel_;

protected:
    cv::Mat create_empty_channel(){ return cv::Mat(options_.frame_height, options_.frame_width, CV_32F); }

    void prepare_channels(){
        presence_count_channel_ = create_empty_channel();
        velocity_abs_channel_ = create_empty_channel();
        velocity_x_channel_ = create_empty_channel();
        velocity_y_channel_ = create_empty_channel();
    }

    void add_to_channel(cv::Point2d p, cv::Mat &channel, float value){
        float& channel_point_ref = channel.at<float>(p);
        channel_point_ref = channel_point_ref + value;
    }

    void add_to_channel(cv::Rect rect, cv::Mat &channel, float value){
        cv::Mat added = create_empty_channel();
        cv::rectangle(added, rect, cv::Scalar(value));
        cv::add(channel, added, channel);
    }

    cv::Mat average_channel(cv::Mat& channel){
        // @todo THIS MIGHT BE FUCKING BROKEN DUE TO FLOATING POINT DIV = inf/nan/0
        // cv::setUseOptimized(false); could resolve that as a last ditch effort
        cv::Mat averaged = channel / presence_count_channel_;
    }

    void apply_presence_count(std::shared_ptr<Tracklet> tracklet){
        float count_increment = 1.0f;
        auto center = tracklet -> get_center();
        add_to_channel(tracklet -> get_center(), presence_count_channel_, count_increment);
    }

    void apply_absolute_velocity(std::shared_ptr<Tracklet> tracklet){
        float velocity = tracklet -> get_corrected_measurement().velocity();

        add_to_channel(tracklet -> get_center(), velocity_abs_channel_, velocity);
    }

    void apply_separated_velocities(std::shared_ptr<Tracklet> tracklet){
        Measurement m = tracklet -> get_corrected_measurement();
        cv::Point2d p = tracklet -> get_center();
        add_to_channel(p, velocity_x_channel_, m.v_x);
        add_to_channel(p, velocity_y_channel_, m.v_y);
    }

    void apply_tracklets_to_channels(std::vector<std::shared_ptr<Tracklet>> tracklets){
        for(const auto &tracklet : tracklets){
            apply_presence_count(tracklet);

            if(options_.generate_absolute_velocity){
                apply_absolute_velocity(tracklet);
            }

            if(options_.generate_separated_velocity){
                apply_separated_velocities(tracklet);
            }

        }
    }


public:

    TrajectoryGenerator()
    {
        prepare_channels();
    }

    std::mutex& get_mtx_update(){ return mtx_trajectories_; }
    std::condition_variable& get_cv_update() { return cv_update_trajectories; }
    TrajectoryOptions& get_options_ref(){ return options_; }

    void update(std::vector<std::shared_ptr<Tracklet>> tracklets){
        std::unique_lock<std::mutex> lock(mtx_trajectories_);
        // std::cout << "Trajectory Generator | update" << std::endl;
        apply_tracklets_to_channels(tracklets);
        // std::cout << "Trajectory Generator | FINISHED update" << std::endl;
        cv_update_trajectories.notify_all();
    }

    

    void generate_heatmaps(){
        

    }

    cv::Mat generate_presence_heatmap(){
        std::cout << "Generating presence heatmap | locking trajectories\n";
        std::unique_lock<std::mutex> lock(mtx_trajectories_);

        cv::Mat heatmap = presence_count_channel_.clone();
        cv::Mat binary = heatmap.clone();

        
        // binary.convertTo(binary,CV_8UC1);
        // cv::threshold(binary, binary, 0.0 , 1.0, cv::THRESH_BINARY);
        // binary = binary * 255;
        // cv::normalize(heatmap, heatmap, 0, 255, cv::NormTypes::NORM_MINMAX, CV_8UC1);
        // cv::cvtColor(binary, binary, cv::COLOR_GRAY2RGB);
        
        int dilation_elem = 0;
        int dilation_size = 5;
        int const max_elem = 2;
        int const max_kernel_size = 21;

        std::cout << "Struct element\n";
        cv::Mat element = getStructuringElement( cv::MorphShapes::MORPH_ELLIPSE,
            cv::Size( 2*dilation_size + 1, 2*dilation_size+1 ),
            cv::Point( dilation_size, dilation_size ) );

        std::cout << "Dilation\n";
        cv::dilate(heatmap, heatmap, element);
        // std::cout << "Median blur\n";
        // cv::medianBlur(heatmap, heatmap, 3);


        std::cout << "Normalizing presences\n";
        cv::normalize(heatmap, heatmap, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        cv::GaussianBlur(heatmap, heatmap, cv::Size(11,11), 20, 20);
        
        std::cout << "Converting to BGR\n";
        
        
        cv::cvtColor(heatmap, heatmap, cv::COLOR_GRAY2BGR);
        std::cout << "Applying colormap\n";
        cv::applyColorMap(heatmap, heatmap, cv::COLORMAP_JET);
        std::cout << "Generating presence heatmap\n";
        return heatmap;

    }
    

};

} //namespace Traffic