#include "imageViewer.hpp"

bool ImageViewer::initialized_ = false;
std::mutex ImageViewer::mtx_init_;
std::mutex ImageViewer::mtx_show_;
int ImageViewer::active_instances_ = 0;