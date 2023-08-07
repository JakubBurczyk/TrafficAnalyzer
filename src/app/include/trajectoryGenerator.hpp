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
    cv::Mat create_empty_channel(){ return cv::Mat(options_.frame_width, options_.frame_height, CV_32F); }

    void prepare_channels(){
        presence_count_channel_ = create_empty_channel();
        velocity_abs_channel_ = create_empty_channel();
        velocity_x_channel_ = create_empty_channel();
        velocity_y_channel_ = create_empty_channel();
    }

    void add_to_channel(cv::Point2d p, cv::Mat &channel, float value){
        auto channel_point_ref = channel.at<float>(p);
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

    void apply_presence_count(std::shared_ptr<const Tracklet> tracklet){
        float count_increment = 1;
        add_to_channel(tracklet -> get_center(), presence_count_channel_, count_increment);
    }

    void apply_absolute_velocity(std::shared_ptr<const Tracklet> tracklet){
        float velocity = tracklet -> get_corrected_measurement().velocity();

        add_to_channel(tracklet -> get_center(), velocity_abs_channel_, velocity);
    }

    void apply_separated_velocities(std::shared_ptr<const Tracklet> tracklet){
        Measurement m = tracklet -> get_corrected_measurement();
        cv::Point2d p = tracklet -> get_center();
        add_to_channel(p, velocity_x_channel_, m.v_x);
        add_to_channel(p, velocity_y_channel_, m.v_y);
    }

    void apply_tracklets_to_channels(std::vector<std::shared_ptr<const Tracklet>> tracklets){
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

    TrajectoryGenerator(TrajectoryOptions options)
    :
        options_{options}
    {
        prepare_channels();
    }

    std::mutex& get_mtx_update(){ return mtx_trajectories_; }
    std::condition_variable& get_cv_update() { return cv_update_trajectories; }
    
    void update(std::vector<std::shared_ptr<const Tracklet>> tracklets){
        std::unique_lock<std::mutex> lock(mtx_trajectories_);

        apply_tracklets_to_channels(tracklets);

        cv_update_trajectories.notify_all();
    }

    void generate_heatmaps(){
        std::unique_lock<std::mutex> lock(mtx_trajectories_);

    }
    

};

} //namespace Traffic