#pragma once

#include "tracklet.hpp"
#include "Hungarian.h"
#include "hungarian_data.hpp"
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

		for(int i = 0; i< tracklets_.size(); i++){
			if(tracklets_[i].get() == nullptr){
				continue;
			}

			if(tracklets_[i]-> get_id() == id){
				std::cout << "trying to delete " << id << "size: " << tracklets_.size() << std::endl;
				tracklets_[i].reset();
				std::cout << "reset the pointer" << std::endl;
				tracklets_.erase(tracklets_.begin() + i);
				i--;
				break;
			}

		}
	}


	HungarianData calculate_assignments(const std::vector<Detection> &detections){

		HungarianData hungarian_data;
		auto& iou_matrix = hungarian_data.iou_matrix;
		auto& assignments = hungarian_data.assignments;
		auto& costs = hungarian_data.costs;

		if(detections.empty()){ return hungarian_data; }

		for(auto const &detection : detections){
			std::vector<double> iou_row;
			for(auto const& tracklet : tracklets_){
				cv::Rect tracklet_box = tracklet -> get_updated_detecton().box;
				
				iou_row.push_back(IoU::calculate_iou(tracklet_box, detection.box));
			}

			iou_matrix.push_back(iou_row);
		}
		// std::cout << "MATRIX: {\n";
		// for ( const auto &row : iou_matrix )
		// {
			// for ( const auto &s : row ) std::cout << s << ' ';
			// std::cout << std::endl;
		// }
		// std::cout << "};\n";
		
		HungAlgo_.Solve(iou_matrix, assignments);
		// std::cout << "Assignments |\n";

		for (unsigned int x = 0; x < assignments.size(); x++){
			// std::cout << "Detection " << x << ", assigned tracker: " << assignments[x];
			float iou = -1;
			if( assignments[x] >= 0){
				iou = iou_matrix[x][assignments[x]];
				
			}
			// std::cout << ", IoU = " << iou << " | pos: | " << detections[x].box.x << " ; " << detections[x].box.y;
			// std::cout << std::endl;
			costs.push_back(iou);
		}

		for (unsigned int x = 0; x < costs.size(); x++){
			// std::cout << "Detection " << x << ", cost : " << costs[x] << std::endl;
		}

		return hungarian_data;
	}

	void update_tracklets(HungarianData hungarian_data, const std::vector<Detection> &detections){
		std::map<uint32_t, Detection> update_map;
		auto &assignments = hungarian_data.assignments;
		auto &costs = hungarian_data.costs;
		
		for (unsigned int detection_idx = 0; detection_idx < assignments.size(); detection_idx++){
			int tracker_idx = assignments[detection_idx];
			auto &detection = detections[detection_idx];
			float cost = costs[detection_idx];

			if(tracker_idx >= 0){
				auto &tracklet = tracklets_[tracker_idx];
				bool iou_threshold_pass = (0 <= cost) && (cost <= 0.5);
				// std::cout << "Tracker " << tracklet -> get_id() << " associated cost " << cost << " PASS: " << iou_threshold_pass << std::endl;
				if(iou_threshold_pass){
					std::cout << "Mapping tracker: " << tracklet -> get_id() << " with detecion at: " << detection.box.x  <<  "  ; "<< detection.box.y << std::endl;
					update_map[tracklet -> get_id()] = detection;
				}
			}else{
				KalmanOptions options;
				options.fps = 60;
				options.inititial_measurement.x = (float)detection.box.x;
				options.inititial_measurement.y = (float)detection.box.y;
				auto new_tracklet = std::make_shared<Tracklet>(options, detection);
				tracklets_.push_back(new_tracklet);
				std::cout << "Created new tracker: " << new_tracklet -> get_id() << " with detecion no: " << detection_idx << std::endl;
			}
		}

		for(const auto& tracklet : tracklets_){
			if(! tracklet -> is_allowed_to_update()){
				// std::cout << "Allowing tracklet: " << tracklet -> get_id() << " to be updated " << std::endl;
				tracklet -> allow_updates();
			}else{
				auto &detection = update_map[tracklet -> get_id()];
				

				tracklet -> update(detection);
				
			}
		}

		// for(const auto& tracklet : tracklets_){
		// 	if(tracklet -> should_terminate()){
		// 		std::cout << "Destroying tracklet: " << tracklet -> get_id() << std::endl;
		// 		remove_tracklet(tracklet -> get_id());
		// 	}
		// }

		for(int i = 0; i< tracklets_.size(); i++){
			if(tracklets_[i] -> should_terminate()){
				std::cout << "trying to delete " << i << "size: " << tracklets_.size() << std::endl;
				tracklets_[i].reset();
				std::cout << "reset the pointer" << std::endl;
				tracklets_.erase(tracklets_.begin() + i);
				i--;
				break;
			}

		}

		for(const auto& tracklet : tracklets_){
			Detection det = tracklet -> get_updated_detecton();
			std::cout << "Updated tracker: " << tracklet -> get_id() << " to position: |x: " << det.box.x << " | y:  " << det.box.y << std::endl;
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
		std::cout << "\n---------------------------------\n";
		auto hungarian_data = calculate_assignments(detections);
		update_tracklets(hungarian_data, detections);
	}

	std::vector<std::shared_ptr<Tracklet>> get_tracklets(){
		return tracklets_;
	}


	void reset(){
		tracklets_ = std::vector<std::shared_ptr<Tracklet>>();
		Tracklet::reset_ids();
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