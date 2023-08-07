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
	KalmanOptions options_;

	bool allowed_updates_ = false;
	

protected:
	void assign_id(){
		std::unique_lock<std::mutex> lock(mtx_id_);
		lowest_id_ ++;
		id_ = lowest_id_;
	}

public:
	Tracklet(KalmanOptions options, Detection detection)
	:
		options_{options}
	{
		// previous_detection_ = detection;
		options_.inititial_measurement.x = (float)detection.box.x;
		options_.inititial_measurement.y = (float)detection.box.y;
		current_detection_ = detection;

		kalman_ = std::make_unique<KalmanEstimator>(options_);
		assign_id();
	}

	~Tracklet(){
		kalman_.reset();
	}

	static void reset_ids(){
		std::unique_lock<std::mutex> lock(mtx_id_);
		lowest_id_ = 0;
	}

	uint64_t get_id(){ return id_; }
	bool is_allowed_to_update(){ return allowed_updates_; }

	Detection get_updated_detecton() const{
		return current_detection_;
	}

	bool should_terminate(){
		return (kalman_ -> get_prediction_counter()) > MAX_PREDICTED_FRAMES;
	}

	void allow_updates(){
		allowed_updates_ = true;
	}
	

	cv::Point2d get_center() const{
		cv::Point2d center;
		center.x = current_detection_.box.x + current_detection_.box.width/2;
		center.x = current_detection_.box.y + current_detection_.box.height/2;
		return center;
	}
	
	Measurement get_corrected_measurement(bool centered=false) const{
		Measurement m = kalman_ -> get_corrected_measurement();

		if(centered){
			cv::Point2d center = get_center();
			m.x = center.x;
			m.y = center.y;
		}

		return m;
	}


	void update(Detection detection){
		// previous_detection_ = current_detection_;
		current_detection_ = detection;

		measurement_.x = (float)current_detection_.box.x;
		measurement_.y = (float)current_detection_.box.y;
		
		// std::cout 	<< "Updating tracker " << id_
		// 			<< " located | x = " << kalman_ -> get_corrected_measurement().x
		// 			<< " | y = " << kalman_ -> get_corrected_measurement().y
		// 			<< " | with detection at | x = " << measurement_.x
		// 			<< " | y = " << measurement_.y 
		// 			<< std::endl;

		kalman_ -> update(measurement_);

		Measurement corrected = kalman_ -> get_corrected_measurement();

		// std::cout 	<< "Updated tracker " << id_
		// 			<< " corrected to | x = " << corrected.x
		// 			<< " | y = " << corrected.y
		// 			<< std::endl;

		current_detection_.box.x = corrected.x;
		current_detection_.box.y = corrected.y;

		// std::cout << "\n###############################\n\n";
	}

	void visualize(cv::Mat &output_frame){

		Measurement m = kalman_ -> get_corrected_measurement();
		auto box = current_detection_.box;
		box.x = m.x;
		box.y = m.y;

		// std::cout 	<< "Visualizing tracker " << id_
		// 			<< " located | x = " << get_updated_detecton().box.x
		// 			<< " | y = " << get_updated_detecton().box.y
		// 			<< std::endl;

		cv::Scalar color = cv::Scalar(100,255,0);
		double font_scale = 0.75;
		std::string s = "T:" + std::to_string(id_);
		cv::Size textSize = cv::getTextSize(s, cv::FONT_HERSHEY_DUPLEX, font_scale, 1, 0);
		cv::Rect textBox(box.x, box.y - textSize.height, textSize.width, textSize.height);

		cv::rectangle(output_frame, textBox, color, cv::FILLED);
		cv::putText(output_frame, s, cv::Point(box.x, box.y), cv::FONT_HERSHEY_DUPLEX, font_scale, cv::Scalar(0, 0, 0), 1, 0);

		cv::rectangle(output_frame, box, color);
	}
};

} // namespace Traffic