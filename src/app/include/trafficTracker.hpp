#pragma once

#include "tracklet.hpp"
#include "Hungarian.h"

/*

	vector< vector<double> > costMatrix = { {1,2},{2,1},{0,3} };

	HungarianAlgorithm HungAlgo;
	vector<int> assignment;

	double cost = HungAlgo.Solve(costMatrix, assignment);

	for (unsigned int x = 0; x < costMatrix.size(); x++)
		std::cout << x << ", assigned job:" << assignment[x] << "\t";

	std::cout << "\ncost: " << cost << std::endl;

*/
namespace Traffic{

class TrafficTracker{
private:
	std::mutex mtx_notify_tracks_;
	std::condition_variable cv_tracks_;

	HungarianAlgorithm HungAlgo_;
	
	
	std::vector<std::shared_ptr<Tracklet>> tracklets_;
	cv::Mat frame_;
protected:

	void remove_tracklet(uint64_t id){
		int idx = 0;
		for(auto const& tracklet : tracklets_){
			if(tracklet -> get_id() == id){
				tracklets_.erase(tracklets_.begin() + idx);
				break;
			}
			idx++;
		}
	}


	std::vector<int> calculate_assignments(const std::vector<Detection> &detections){
		std::vector<std::vector<double>> iou_matrix;

		for(auto const &detection : detections){
			std::vector<double> iou_row;
			for(auto const& tracklet : tracklets_){
				cv::Rect tracklet_box = tracklet -> get_updated_detecton().box;
				
				iou_row.push_back(IoU::calculate_iou(tracklet_box, detection.box));
			}

			iou_matrix.push_back(iou_row);
		}
		std::cout << "MATRIX: {\n";
		for ( const auto &row : iou_matrix )
		{
			for ( const auto &s : row ) std::cout << s << ' ';
			std::cout << std::endl;
		}
		std::cout << "};\n";
		std::vector<int> assignments;
		HungAlgo_.Solve(iou_matrix, assignments);
		return assignments;
	}

	void update_tracklets(const std::vector<int> &assignments, const std::vector<Detection> &detections){
		std::map<uint32_t, Detection> update_map;

		for (unsigned int detection_idx = 0; detection_idx < assignments.size(); detection_idx++){
			int tracker_idx = assignments[detection_idx];
			auto &detection = detections[detection_idx];
			

			if(tracker_idx >= 0){
				auto &tracklet = tracklets_[tracker_idx];
				// std::cout << "Mapping traecker: " << tracklet -> get_id() << " with detecion no: " << detection_idx << std::endl;
				update_map[tracklet -> get_id()] = detection;

			}else{
				KalmanOptions options;
				options.fps = 60;
				options.inititial_measurement.x = detection.box.x;
				options.inititial_measurement.y = detection.box.y;
				auto new_tracklet = std::make_shared<Tracklet>(options, detection);
				tracklets_.push_back(new_tracklet);
				std::cout << "Creating new tracker: " << new_tracklet -> get_id() << " with detecion no: " << detection_idx << std::endl;
			}
		}

		for(const auto& tracklet : tracklets_){
			if(! tracklet -> is_allowed_to_update()){
				std::cout << "Allowing tracklet: " << tracklet -> get_id() << " to be updated " << std::endl;
				tracklet -> allow_updates();
			}else{
				tracklet -> update(update_map[tracklet -> get_id()]);
				if(tracklet -> should_terminate()){
					std::cout << "Destroying tracklet: " << tracklet -> get_id() << std::endl;
					remove_tracklet(tracklet -> get_id());
				}
			}
		}

		cv_tracks_.notify_all();
			
	}

public:

	TrafficTracker(){

	}

	std::mutex& get_mtx_tracks() { return mtx_notify_tracks_; }
	std::condition_variable& get_cv_tracks(){ return cv_tracks_; }

	void update(std::vector<Detection> detections, cv::Mat frame){
		frame_ = frame;
		auto assignments = calculate_assignments(detections);
		update_tracklets(assignments, detections);
	}

	std::vector<std::shared_ptr<Tracklet>> get_tracklets(){
		return tracklets_;
	}

	cv::Mat visualize_tracklets(){
		cv::Mat visualization_frame = frame_.clone();
		for(const auto& tracklet : tracklets_){
			tracklet -> visualize(visualization_frame);
		}

		return visualization_frame;
	}

};

} // namepsace Traffic