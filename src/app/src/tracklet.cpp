#include "tracklet.hpp"

namespace Traffic{

uint64_t Tracklet::lowest_id_ = 0;
std::mutex Tracklet::mtx_id_ = std::mutex();
uint32_t Tracklet::MAX_PREDICTED_FRAMES = 60;
} // namespace Traffic