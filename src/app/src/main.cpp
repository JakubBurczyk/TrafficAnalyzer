/*
 * Copyright (c) 2022 Martin Helmut Fieber <info@martin-fieber.se>
 */

#define SDL_MAIN_HANDLED

#include "trafficAnalyzerApp.hpp"
#include "Core/Log.hpp"

#include "Hungarian.h"

using namespace Traffic;


int main() {
	try {
	APP_PROFILE_BEGIN_SESSION_WITH_FILE("App", "profile.json");
	gl3wInit();
	{
		APP_PROFILE_SCOPE("Test scope");

		auto frames		= std::make_shared<FrameProvider>(std::string("./"));
		auto frame_prep = std::make_shared<FramePreprocessor>();
		auto detector 	= std::make_shared<ObjectDetector>();
		auto background = std::make_shared<BackgroundEstimator>();
		auto tracker 	= std::make_shared<TrafficTracker>();
		auto trajectory = std::make_shared<TrajectoryGenerator>();
		auto analyzer 	= std::make_shared<TrafficAnalyzer>(frames,
															frame_prep,
															detector,
															background,
															tracker,
															trajectory
															);

		TrafficAnalyzerApp app("App", analyzer);
		app.run();
	}

	APP_PROFILE_END_SESSION();

	}catch (std::exception& e) {
		std::cout << "APP EXCEPTION: " << e.what();
		APP_ERROR("Main process terminated with: {}", e.what());
	}
	// KalmanOptions options;
	// Detection det;

	// std::vector<std::shared_ptr<Tracklet>> tracklets;
	
	// auto t = std::make_shared<Tracklet>(options,det);

	// tracklets.push_back(t);
	// tracklets[0].reset();

	// std::cout << "reset done \n";

	// tracklets.erase(tracklets.begin() + 0);


	// std::cout << "EOF\n";
	return 0;
}
