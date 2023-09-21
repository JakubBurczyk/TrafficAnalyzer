#pragma once
#include <optional>
#include <condition_variable>

#include "tracklet.hpp"
#include "utils.hpp"

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
    int heatmap_type = cv::COLORMAP_TURBO;

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

    void initialize(){
        prepare_channels();
    }

    void prepare_channels(){
        presence_count_channel_ = create_empty_channel();
        velocity_abs_channel_ = create_empty_channel();
        velocity_x_channel_ = create_empty_channel();
        velocity_y_channel_ = create_empty_channel();
    }

    void add_to_channel(cv::Point2d p, cv::Mat &channel, float value){
        try{
            if(p.x < 0 || p.x > options_.frame_width || p.y < 0 || p.y > options_.frame_height){ return; }
            // std::cout << "TrajectoryGenerator | Reading ref to channel at (" << p.x <<" ; "<< p.y << ")" << std::endl;
            float& channel_point_ref = channel.at<float>(p);
            // float ch_value = channel.at<float>(p);
            // std::cout << "TrajectoryGenerator | adding values" << std::endl;
            // std::cout << "TrajectoryGenerator | Adding value = "<< value << "to value " << channel_point_ref << std::endl;
            channel_point_ref = channel_point_ref + value;
            // channel.at<float>(p) = ch_value + value;
            // std::cout << "TrajectoryGenerator | Adding to channel done" << std::endl;
        }
        catch(std::exception &e){

        }
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
        return averaged;
    }

    void apply_presence_count(std::shared_ptr<Tracklet> tracklet){
        // std::cout << "TrajectoryGenerator | Applying presence" << std::endl;
        float count_increment = 1.0f;
        auto center = tracklet -> get_center();
        add_to_channel(tracklet -> get_center(), presence_count_channel_, count_increment);
        // std::cout << "TrajectoryGenerator | Applying presence done" << std::endl;
    }

    void apply_absolute_velocity(std::shared_ptr<Tracklet> tracklet){
        // std::cout << "TrajectoryGenerator | Applying abs vel" << std::endl;
        float velocity = tracklet -> get_corrected_measurement().velocity();

        add_to_channel(tracklet -> get_center(), velocity_abs_channel_, velocity);
        // std::cout << "TrajectoryGenerator | Applying abs vel done" << std::endl;
    }

    void apply_separated_velocities(std::shared_ptr<Tracklet> tracklet){
        // std::cout << "TrajectoryGenerator | Applying separated vel" << std::endl;
        Measurement m = tracklet -> get_corrected_measurement();
        cv::Point2d p = tracklet -> get_center();
        add_to_channel(p, velocity_x_channel_, m.v_x);
        add_to_channel(p, velocity_y_channel_, m.v_y);
        // std::cout << "TrajectoryGenerator | Applying separated vel done" << std::endl;
    }

    void apply_tracklets_to_channels(std::vector<std::shared_ptr<Tracklet>> tracklets){
        // std::cout << "TrajectoryGenerator | Applying tracklets" << std::endl;
        for(const auto &tracklet : tracklets){
            apply_presence_count(tracklet);

            if(options_.generate_absolute_velocity){
                apply_absolute_velocity(tracklet);
            }

            if(options_.generate_separated_velocity){
                apply_separated_velocities(tracklet);
            }

        }
        // std::cout << "TrajectoryGenerator | Applying tracklets done" << std::endl;
    }

    cv::Mat color_heatmap(cv::Mat heatmap, int color = -1){
        cv::cvtColor(heatmap, heatmap, cv::COLOR_GRAY2BGR);
        if(color == -1){
            color = options_.heatmap_type;
        }
        cv::applyColorMap(heatmap, heatmap, color);
        return heatmap;
    }

    cv::Mat dilate_channel(cv::Mat heatmap){
        int dilation_size = 5;
        cv::Mat element = getStructuringElement( cv::MorphShapes::MORPH_ELLIPSE,
            cv::Size( 2*dilation_size + 1, 2*dilation_size+1 ),
            cv::Point( dilation_size, dilation_size ) );

        cv::dilate(heatmap, heatmap, element);
        return heatmap;
    }

    cv::Mat directional_heatmap(cv::Mat &channel){
        cv::Mat positive_mask;
        cv::Mat negative_mask;
        cv::Mat positive_heatmap;
        cv::Mat negative_heatmap;

        cv::Mat heatmap = average_channel(channel);

        cv::normalize((heatmap > 0), positive_mask, 0, 1, cv::NORM_MINMAX, CV_32FC1);
        cv::normalize((heatmap < 0), negative_mask, 0, 1, cv::NORM_MINMAX, CV_32FC1);

        cv::multiply(heatmap, positive_mask, positive_heatmap);
        cv::multiply(cv::abs(heatmap), negative_mask, negative_heatmap);

        cv::normalize(positive_heatmap, positive_heatmap, 0, 127, cv::NORM_MINMAX, CV_8UC1);
        cv::normalize(negative_heatmap, negative_heatmap, 0, 127, cv::NORM_MINMAX, CV_8UC1);

        positive_heatmap = dilate_channel(positive_heatmap);
        negative_heatmap = dilate_channel(negative_heatmap);

        cv::GaussianBlur(positive_heatmap, positive_heatmap, cv::Size(11,11), 20, 20);
        cv::GaussianBlur(negative_heatmap, negative_heatmap, cv::Size(11,11), 20, 20);

        positive_heatmap.convertTo(positive_heatmap, CV_8SC1);
        negative_heatmap.convertTo(negative_heatmap, CV_8SC1);

        heatmap =  positive_heatmap - negative_heatmap;
        heatmap.convertTo(heatmap, CV_8SC1);

        return heatmap;
    }


public:

    TrajectoryGenerator()
    {
        initialize();
    }

    std::mutex& get_mtx_update(){ return mtx_trajectories_; }
    std::condition_variable& get_cv_update() { return cv_update_trajectories; }
    TrajectoryOptions& get_options_ref(){ return options_; }

    void set_fps(uint32_t value){options_.fps = value;}
    void set_width(uint32_t value){ options_.frame_width = value; }
    void set_height(uint32_t value){ options_.frame_height = value; }

    void update(std::vector<std::shared_ptr<Tracklet>> tracklets){
        try{
            // std::cout << "TrajectoryGenerator | Update" << std::endl;
            std::unique_lock<std::mutex> lock(mtx_trajectories_);
            apply_tracklets_to_channels(tracklets);
            cv_update_trajectories.notify_all();
            // std::cout << "TrajectoryGenerator | Update done" << std::endl;
        }
        catch(std::exception &e){

        }
    }

    void reset(){
        initialize();
    }


    cv::Mat generate_presence_heatmap(){
        std::unique_lock<std::mutex> lock(mtx_trajectories_);

        cv::Mat heatmap = presence_count_channel_.clone();
        
        // binary.convertTo(binary,CV_8UC1);
        // cv::threshold(binary, binary, 0.0 , 1.0, cv::THRESH_BINARY);
        // binary = binary * 255;
        // cv::normalize(heatmap, heatmap, 0, 255, cv::NormTypes::NORM_MINMAX, CV_8UC1);
        // cv::cvtColor(binary, binary, cv::COLOR_GRAY2RGB);
                // std::cout << "Median blur\n";
        // cv::medianBlur(heatmap, heatmap, 3);

        cv::GaussianBlur(heatmap, heatmap, cv::Size(11,11), 20, 20);
        cv::normalize(heatmap, heatmap, 0, 255, cv::NORM_MINMAX, CV_8UC1);

        heatmap = dilate_channel(heatmap);
        heatmap = color_heatmap(heatmap);

        return heatmap;
    }

    cv::Mat generate_avg_speed_heatmap(){
        std::unique_lock<std::mutex> lock(mtx_trajectories_);

        cv::Mat heatmap = average_channel(velocity_abs_channel_);

        cv::normalize(heatmap, heatmap, 0, 255, cv::NORM_MINMAX, CV_8UC1);

        heatmap = dilate_channel(heatmap);

        cv::GaussianBlur(heatmap, heatmap, cv::Size(11,11), 20, 20);
        
        heatmap = color_heatmap(heatmap);

        return heatmap;
    }

    cv::Mat generate_x_speed_heatmap(){
        std::unique_lock<std::mutex> lock(mtx_trajectories_);
        cv::Mat heatmap = directional_heatmap(velocity_x_channel_);

        cv::normalize(heatmap, heatmap, 0, 255, cv::NORM_MINMAX, CV_8UC1);

        heatmap = color_heatmap(heatmap);

        return heatmap;
    }

    cv::Mat generate_y_speed_heatmap(){
        std::unique_lock<std::mutex> lock(mtx_trajectories_);
        cv::Mat heatmap = directional_heatmap(velocity_y_channel_);

        cv::normalize(heatmap, heatmap, 0, 255, cv::NORM_MINMAX, CV_8UC1);

        heatmap = color_heatmap(heatmap);

        return heatmap;
    }
    
};

} //namespace Traffic