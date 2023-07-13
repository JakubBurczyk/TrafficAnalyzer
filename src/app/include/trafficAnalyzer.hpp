#pragma once
#include "Core/Application.hpp"
#include "Core/Instrumentor.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <glad/glad.h>
#include <imgui.h>


#include "objectDetector.hpp"

namespace Traffic{


class TrafficAnalyzer{
private:

    std::shared_ptr<ObjectDetector> detector_;

protected:


public:

    TrafficAnalyzer(std::shared_ptr<ObjectDetector> detector):
        detector_{detector}
    {

    }


    std::shared_ptr<ObjectDetector> get_object_detector(){ return detector_; };
    
};

} // namespace Traffic