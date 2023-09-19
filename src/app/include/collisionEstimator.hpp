#pragma once

#include <condition_variable>

#include "tracklet.hpp"
#include "utils.hpp"

namespace Traffic{

struct CollistionOptions{
    double future_millis = 2000;
    double input_fps = 60;
};

class Collider{
private:
    std::shared_ptr<Tracklet> tracklet;
    std::shared_ptr<utils::LineSegment> trajectory_seg;
    bool reversed_trajectory = false;
    bool collides_ = false;
    std::vector<cv::Point2d> intersection_points_;
    uint64_t id_;
public:
    Collider(std::shared_ptr<Tracklet> tracklet, const CollistionOptions &options){
        Measurement m = tracklet -> get_corrected_measurement(true);
        cv::Point2d p1;
        cv::Point2d p2;
        p1.x = m.x;
        p1.y = m.y;
        // std::cout << "mv_x: " << m.v_x << " | mv_y: " << m.v_y << " | ";
        p2.x = m.x + m.v_x * options.future_millis  / 1000;
        p2.y = m.y + m.v_y * options.future_millis  / 1000;
        
        if(p1.x < p2.x){
            reversed_trajectory = true;
            std::swap(p1,p2);
        }

        id_ = tracklet -> get_id();

        trajectory_seg = std::make_shared<utils::LineSegment>(p1,p2);
        // std::cout << "p1: " << trajectory_seg -> p1.x << "; " << trajectory_seg ->  p1.y << " | p2: "<< trajectory_seg -> p2.x << "; " << trajectory_seg -> p2.y << std::endl;
    }

    std::shared_ptr<utils::LineSegment> get_trajectory_seg() const { return trajectory_seg; }
    bool is_valid() const { return  trajectory_seg -> is_valid(); }
    uint64_t get_id() const  { return id_; }

    cv::Point2d get_intersection_point(const Collider &collider){
        auto collider_slope = collider.get_trajectory_seg() -> slope;
        auto collider_intercept = collider.get_trajectory_seg() -> intercept;
        cv::Point2d intersection_point;

        if(collider_slope == trajectory_seg -> slope){
            //find closest point to collider on either segment
        }
        else{
            double nom =  collider_intercept - trajectory_seg -> intercept;
            double denom = trajectory_seg -> slope - collider_slope;

            intersection_point.x = nom/denom;
            intersection_point.y = trajectory_seg -> slope * intersection_point.x + trajectory_seg -> intercept;
        }

        std::cout << "Intersection of " << this -> get_id()
        << " and " << collider.get_id() << "located at: "
        << intersection_point.x << " ; " << intersection_point.y << std::endl;
        return intersection_point;
    };

    bool check_collision(const Collider &collider){
        if(this -> get_id() == collider.get_id() ) { return false; }
        if(!is_valid() || !collider.is_valid()) { std::cout << "erly terminate\n";return false; }

        // std::cout << "Checking collision" << std::endl;
        bool collision = utils::doSegmentIntersect(trajectory_seg, collider.get_trajectory_seg());
        collides_ = collides_ || collision;

        if(collision){
            intersection_points_.push_back(get_intersection_point(collider));
        }
//         std::cout << "IDS: " << this -> get_id() << " : " << collider.get_id() <<  " ";
// std::cout << "{{" << trajectory_seg -> p1.x << "," << trajectory_seg ->  p1.y << "},{"
//     << trajectory_seg -> p2.x << "," << trajectory_seg -> p2.y << "}}" << " WITH "
//      << "{{" << collider.get_trajectory_seg() -> p1.x << "," << collider.get_trajectory_seg() ->  p1.y << "},{"
//     << collider.get_trajectory_seg() -> p2.x << "," << collider.get_trajectory_seg() -> p2.y << "}} = " <<  collides_ << std::endl;

        return collides_;
    }

    void visualize(cv::Mat &output_frame) const {
        auto p1 = reversed_trajectory ? get_trajectory_seg() -> p2 : get_trajectory_seg() -> p1;
        auto p2 = reversed_trajectory ? get_trajectory_seg() -> p1 : get_trajectory_seg() -> p2;

        cv::Scalar color; 
        // = collides_ ? cv::Scalar(255,145,0) : cv::Scalar(0,255,200);
        std::cout << "Visu host: " << this -> get_id() << " collides = " << collides_ << std::endl;
        if(collides_){
            color = cv::Scalar(0,145,255);
        }else{
            color = cv::Scalar(200,255,0);
        }


		cv::arrowedLine(output_frame, p1, p2, color,2);

		double font_scale = 0.75;
		std::string s = "T:" + std::to_string(this -> get_id());
		cv::Size textSize = cv::getTextSize(s, cv::FONT_HERSHEY_DUPLEX, font_scale, 1, 0);
		cv::Rect textBox(p1.x, p1.y - textSize.height, textSize.width, textSize.height);

        cv::rectangle(output_frame, textBox, color, cv::FILLED);
		cv::putText(output_frame, s, cv::Point(p1.x, p1.y), cv::FONT_HERSHEY_DUPLEX, font_scale, cv::Scalar(0, 0, 0), 1, 0);
	
        for(const auto& intersection : intersection_points_){
            cv::circle(output_frame,intersection,5,cv::Scalar(0,0,255),2);
        }
    }

};

class CollisionEstimator{
private:
    CollistionOptions options_;

    std::mutex mtx_collisions_;
    std::condition_variable cv_update_collisions_;
    std::vector<Collider> colliders_;
    cv::Mat frame_;
public:

    CollisionEstimator(){};

    CollistionOptions& get_options_ref(){return options_; }
    std::mutex& get_mtx_update(){ return mtx_collisions_; }
    std::condition_variable& get_cv_update() { return cv_update_collisions_; }

    void update(std::vector<std::shared_ptr<Tracklet>> tracklets, cv::Mat frame){
        std::unique_lock<std::mutex> lock(mtx_collisions_);
        frame_ = frame;
        colliders_ = std::vector<Collider>();

        for(auto &tracklet : tracklets){
            colliders_.push_back(Collider(tracklet, options_));
        }
    
        // std::cout << "starting collision checks len: " << colliders_.size() << std::endl;
        for(auto &host : colliders_){
            // std::cout << "Host id: " << host.get_id() << std::endl;
            for(auto &collider : colliders_){
                host.check_collision(collider);
            }
        }
        
        cv_update_collisions_.notify_all();
    }

    cv::Mat visualize_colliders(){
        cv::Mat visualization_frame = frame_.clone();
		for(const auto& collider : colliders_){
			collider.visualize(visualization_frame);
		}
        std::cout << "---------------------\n";
		return visualization_frame;
	}
    
};

} // namespace Traffic