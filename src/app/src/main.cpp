/*
 * Copyright (c) 2022 Martin Helmut Fieber <info@martin-fieber.se>
 */

#define SDL_MAIN_HANDLED

#include "trafficAnalyzerApp.hpp"
#include "Core/Log.hpp"

using namespace Traffic;

int main() {
	try {
	APP_PROFILE_BEGIN_SESSION_WITH_FILE("App", "profile.json");

	{
		APP_PROFILE_SCOPE("Test scope");

		auto frames		= std::make_shared<FrameProvider>("");
		auto detector 	= std::make_shared<ObjectDetector>(frames);
		auto analyzer 	= std::make_shared<TrafficAnalyzer>(detector);

		TrafficAnalyzerApp app("App", analyzer);
		app.run();
	}

	APP_PROFILE_END_SESSION();

	}catch (std::exception& e) {
		APP_ERROR("Main process terminated with: {}", e.what());
	}

	return 0;
}
