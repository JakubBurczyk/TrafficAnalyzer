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
		auto analyzer 	= std::make_shared<TrafficAnalyzer>(frames,
															frame_prep,
															detector,
															background,
															tracker);

		TrafficAnalyzerApp app("App", analyzer);
		app.run();
	}

	APP_PROFILE_END_SESSION();

	}catch (std::exception& e) {
		APP_ERROR("Main process terminated with: {}", e.what());
	}

	return 0;
}
