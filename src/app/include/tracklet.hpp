#pragma once
#include "kalmanEstimator.hpp"
#include "detection.hpp"

namespace Traffic{

class Tracklet{
private:
	static std::mutex mtx_id_;
	static uint64_t lowest_id_;
	static uint32_t MAX_PREDICTED_FRAMES;

	uint64_t id_;

	Measurement measurement_;
	Detection current_detection_;
	// Detection previous_detection_;

    std::unique_ptr<KalmanEstimator> kalman_;

	bool allowed_updates_ = false;


protected:
	void assign_id(){
		std::unique_lock<std::mutex> lock(mtx_id_);
		lowest_id_ ++;
		id_ = lowest_id_;
	}

public:
	Tracklet(KalmanOptions options, Detection detection)
	{
		// previous_detection_ = detection;
		options.inititial_measurement.x = detection.box.x;
		options.inititial_measurement.y = detection.box.y;
		current_detection_ = detection;

		kalman_ = std::make_unique<KalmanEstimator>(options);
		assign_id();
	}

	uint64_t get_id(){ return id_; }
	bool is_allowed_to_update(){ return allowed_updates_; }

	Detection get_updated_detecton(){
		return current_detection_;
	}

	bool should_terminate(){
		return (kalman_ -> get_prediction_counter()) > MAX_PREDICTED_FRAMES;
	}

	void allow_updates(){
		allowed_updates_ = true;
	}

	cv::Point2d get_center(){
		cv::Point2d center;
		center.x = current_detection_.box.x + current_detection_.box.width/2;
		center.x = current_detection_.box.y + current_detection_.box.height/2;
		return center;
	}

	void update(Detection detection){
		// previous_detection_ = current_detection_;
		current_detection_ = detection;

		measurement_.x = current_detection_.box.x;
		measurement_.y = current_detection_.box.y;

		kalman_ -> update(measurement_);

		Measurement corrected = kalman_ -> get_corrected_measurement();

		current_detection_.box.x = corrected.x;
		current_detection_.box.y = corrected.y;
	}

	void visualize(cv::Mat &output_frame){

		auto box = current_detection_.box;
		cv::Scalar color = cv::Scalar(100,255,0);

		std::string s = "T:" + std::to_string(id_);
		cv::Size textSize = cv::getTextSize(s, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
		cv::Rect textBox(box.x, box.y - 40, textSize.width + 10, textSize.height + 20);

		cv::rectangle(output_frame, textBox, color, cv::FILLED);
		cv::putText(output_frame, s, cv::Point(box.x + 5, box.y - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);

		cv::rectangle(output_frame, box, color);
	}
};

} // namespace Traffic