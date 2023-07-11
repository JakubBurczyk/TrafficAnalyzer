/*
 * Copyright (c) 2022 Martin Helmut Fieber <info@martin-fieber.se>
 */

#define SDL_MAIN_HANDLED

#include "trafficAnalyzerApp.hpp"
#include "Core/Log.hpp"

int main() {
  try {
    APP_PROFILE_BEGIN_SESSION_WITH_FILE("App", "profile.json");

    {
      APP_PROFILE_SCOPE("Test scope");

      auto detector = std::make_shared<ObjectDetector>(ObjectDetector());

      TrafficAnalyzer app("App", detector);
      
      app.run();
    }

    APP_PROFILE_END_SESSION();
  } catch (std::exception& e) {
    APP_ERROR("Main process terminated with: {}", e.what());
  }

  return 0;
}
