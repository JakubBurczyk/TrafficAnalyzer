#pragma once

#include <GL/gl.h>

#include "imgui.h"
#include "dataLogger.hpp"
#include "widget.hpp"

namespace Traffic{

class DataLoggerWidget : public Widget{
private:
    std::shared_ptr<DataLogger> logger_;
public:

    void gui()
};

} // namespace Traffic