#pragma once
#include "kalmanEstimator.hpp"
#include "detection.hpp"

namespace Traffic{

class Tracklet{
private:
	static std::mutex mtx_id_;
	static uint64_t lowest_id_;

	uint64_t id_;

	Measurement measurement_;
	Detection current_detection_;
	Detection previous_detection_;

    std::unique_ptr<KalmanEstimator> kalman_;

protected:
	void assign_id(){
		std::unique_lock<std::mutex> lock(mtx_id_);
		lowest_id_ ++;
		id_ = lowest_id_;
	}

public:
	Tracklet(KalmanOptions options)
	{
		kalman_ = std::make_unique<KalmanEstimator>(options);
		assign_id();
	}

	void update(Detection detection){
		previous_detection_ = detection;
		current_detection_ = detection;

		measurement_.x = current_detection_.box.x;
		measurement_.y = current_detection_.box.y;
		kalman_ -> update(measurement_);
	}
};

} // namespace Traffic