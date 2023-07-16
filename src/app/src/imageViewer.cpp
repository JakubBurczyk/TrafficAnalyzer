#include "imageViewer.hpp"

bool ImageViewer::initialized_ = false;
std::mutex ImageViewer::mtx_init_;