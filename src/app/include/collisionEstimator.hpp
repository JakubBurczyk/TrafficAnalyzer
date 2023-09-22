#pragma once

#include <iomanip>
#include <sstream>
#include <condition_variable>
#include<limits>
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
    Measurement state_;
    cv::Rect bb_;

    double time_to_intersect_ = std::numeric_limits<double>::infinity();
    double pet_ = std::numeric_limits<double>::infinity();

public:
    Collider(std::shared_ptr<Tracklet> tracklet, const CollistionOptions &options){
        state_ = tracklet -> get_corrected_measurement(true);
        bb_ = tracklet -> get_bb();
        cv::Point2d p1;
        cv::Point2d p2;
        p1.x = state_.x;
        p1.y = state_.y;
        // std::cout << "mv_x: " << m.v_x << " | mv_y: " << m.v_y << " | ";
        p2.x = state_.x + state_.v_x * options.future_millis  / 1000;
        p2.y = state_.y + state_.v_y * options.future_millis  / 1000;
        
        double distance_x = p2.x - p1.x;
        double distance_y = p2.y - p1.y;
        double distance = std::sqrt(distance_x * distance_x + distance_y * distance_y);

        if(p1.x < p2.x){
            reversed_trajectory = true;
            std::swap(p1,p2);
        }

        id_ = tracklet -> get_id();

        trajectory_seg = std::make_shared<utils::LineSegment>(p1,p2);
        // std::cout << "p1: " << trajectory_seg -> p1.x << "; " << trajectory_seg ->  p1.y << " | p2: "<< trajectory_seg -> p2.x << "; " << trajectory_seg -> p2.y << std::endl;
        // std::cout << " distance: " <<  distance << " velocity " << state_.velocity() << " Time " << distance / state_.velocity() << std::endl;
    }

    std::shared_ptr<utils::LineSegment> get_trajectory_seg() const { return trajectory_seg; }
    bool is_valid() const { return  trajectory_seg -> is_valid(); }
    uint64_t get_id() const  { return id_; }
    Measurement get_state() const { return state_; }

    double get_tti() const { return time_to_intersect_; }
    double get_pet() const { return pet_; }
    void set_tti(double value) { time_to_intersect_ = value; }
    void set_pet(double value) { pet_ = value; }

    

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

        // std::cout << "Intersection of " << this -> get_id()
        // << " and " << collider.get_id() << "located at: "
        // << intersection_point.x << " ; " << intersection_point.y << std::endl;
        return intersection_point;
    };

    double calculate_tt_intersect(cv::Point2d target){
        
        Measurement m = this -> get_state();
        double tti = std::numeric_limits<double>::infinity();

        double distance_x = m.x - target.x;
        double distance_y = m.y - target.y;
        double distance = std::sqrt(distance_x * distance_x + distance_y * distance_y);
        double velocity = m.velocity();

        if(m.velocity() > 0.5){
            
            tti = distance / m.velocity();
            // std::cout << " | tti: " << tti << std::endl;
        }

        return tti;
    }

    void print_id(){
        // std::cout << "HOST_ID: " << this -> get_id() << " ";
    }

    double calculate_object_length(){
        // std::cout << "object len" << std::endl;
        print_id();
        // std::cout << "slope = " << trajectory_seg -> slope << " | ";
        double alpha = atan(std::abs(trajectory_seg->slope));
        double beta = M_PI_2 - alpha;
        
        // std::cout << "beta " << beta;
        // std::cout << "getting bb" << std::endl;
        double width = bb_.width;
        double height = bb_.height;
        // std::cout << "w: " << width << " | h: " << height << std::endl; 
        // std::cout << "precalc trig"<< std::endl;
        double cos_beta = cos(beta);
        double sin_beta = sin(beta);
        double cos_beta2 = cos_beta * cos_beta;
        double sin_beta2 = sin_beta * sin_beta;

        double denom = (cos_beta2 - sin_beta2);

        // std::cout << "calc values" << std::endl;
        double l_x = (width * cos_beta - height * sin_beta) / denom;
        double l_y = (-1 * width * sin_beta + height * cos_beta ) / denom;
        // std::cout << "object len done" << std::endl;
        return std::max(l_x, l_y);
    }

    double calculate_tt_leave(cv::Point2d target){
        // std::cout << "calc ttl" << std::endl;
        double leave_distance = calculate_object_length();
        
        double ttl = leave_distance / get_state().velocity();
        // std::cout << "host: " << this -> get_id() << " leave distance = " << leave_distance << " | vel: = " << get_state().velocity() << "| ttl: " << ttl << std::endl;
        // std::cout << "calc ttl done" << std::endl;
        return ttl;
    }

    bool check_collision(Collider &collider){
        if(this -> get_id() == collider.get_id() ) { return false; }
        if(!is_valid() || !collider.is_valid()) { return false; }

        // std::cout << "Checking collision" << std::endl;
        bool collision = utils::doSegmentIntersect(trajectory_seg, collider.get_trajectory_seg());
        collides_ = collides_ || collision;

        if(collision){
            cv::Point2d intersection = get_intersection_point(collider);
            intersection_points_.push_back(intersection);
            
            double tti = this -> calculate_tt_intersect(intersection);
            
            if(tti < this -> get_tti()){
                this -> set_tti(tti);
                
                double collider_tti = collider.calculate_tt_intersect(intersection);
                double collider_ttl = collider.calculate_tt_leave(intersection);

                double pet = tti - collider_tti - collider_ttl/2;

                if(pet > 0){
                    // double t2 = collider.calculate_tt_leave(intersection);
                    // double pet = t2 - t1;
                    this -> set_pet(pet);

                    // std::cout << "HOST: " <<  this -> get_id() << " with: " << collider.get_id()  << " | TTI: " << tti << " | PET: " << pet << std::endl;
                    // << " | TTI: " << tti << " | T1: " << t1 << " | T2: " << t2 << " | PET: " << pet << std::endl;
                }

                // this -> set_pet(std::abs(tti - collider_tti));
                // std::cout << "set_tti to: " << this -> get_tti() << std::endl;
            }
            
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
        // std::cout << "Visu host: " << this -> get_id() << " collides = " << collides_ << std::endl;

        if(collides_){
            color = cv::Scalar(0,145,255);
        }else{
            color = cv::Scalar(200,255,0);
        }


		cv::arrowedLine(output_frame, p1, p2, color,2);

		double font_scale = 0.75;

        std::stringstream stream;

        stream << std::fixed << std::setprecision(2) << this -> get_tti();
        std::string tti_str = this -> get_tti() != std::numeric_limits<double>::infinity() ? " | TTI: " + stream.str() + "s" : "";

        stream.str("");
        stream.clear();
        stream << std::fixed << std::setprecision(2) << this -> get_pet();
        std::string pet_str = this -> get_pet() != std::numeric_limits<double>::infinity() ? " | PET: " + stream.str() + "s" : "";

		std::string s = "T:" + std::to_string(this -> get_id()) + tti_str + pet_str;
		cv::Size textSize = cv::getTextSize(s, cv::FONT_HERSHEY_DUPLEX, font_scale, 1, 0);
		cv::Rect textBox(p1.x, p1.y - textSize.height, textSize.width, textSize.height);

        cv::rectangle(output_frame, textBox, color, cv::FILLED);
		cv::putText(output_frame, s, cv::Point(p1.x, p1.y), cv::FONT_HERSHEY_DUPLEX, font_scale, cv::Scalar(0, 0, 0), 1, 0);
	
        for(const auto& intersection : intersection_points_){
            
            // std::stringstream stream;
            // stream << "PET:" << std::fixed << std::setprecision(2) << this -> get_pet();
            
            cv::circle(output_frame,intersection,5,cv::Scalar(0,0,255),2);
            // cv::Size textSize = cv::getTextSize(stream.str(), cv::FONT_HERSHEY_DUPLEX, font_scale, 1, 0);
            // cv::Rect textBox(intersection.x, intersection.y + textSize.height, textSize.width, textSize.height);
            // cv::rectangle(output_frame, textBox, color, cv::FILLED);
            // cv::putText(output_frame, stream.str(), intersection + cv::Point2d(0, 2 * textSize.height), cv::FONT_HERSHEY_DUPLEX, font_scale, cv::Scalar(0, 0, 0), 1, 0);
            
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
        // std::cout << "---------------------\n";
		return visualization_frame;
	}
    
};

} // namespace Traffic