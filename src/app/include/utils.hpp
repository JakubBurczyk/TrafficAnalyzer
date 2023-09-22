#pragma once
#include <opencv2/opencv.hpp>
#include <string_view>
#include <filesystem>
#include <chrono>
#include <algorithm>

#define UTILS_DEBUG_NAME "Utils | "
namespace utils{

// static bool endsWith(std::string_view str, std::string_view suffix)
// {
//     return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
// }

// static bool startsWith(std::string_view str, std::string_view prefix)
// {
//     return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
// }

static std::string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H:%M:%S");
    return ss.str();
}

static std::vector<std::string> get_files_by_extensions(std::string path, std::vector<std::string> extensions){
    std::vector<std::string> files;

    for (const auto & entry : std::filesystem::directory_iterator(path)){

            bool valid_entry = false;
            std::string path;
            
            for(auto &format : extensions){
                path = entry.path();
                if(path.ends_with(format)){
                    valid_entry = true;
                    break;
                }
            }

            if(!valid_entry) continue;

            files.push_back(path);

    }

    return files;
}

static cv::Mat cvMatSignum(cv::Mat src)
{
    cv::Mat z = cv::Mat::zeros(src.size(), src.type()); 
    cv::Mat a = (z < src) & 1;
    cv::Mat b = (src < z) & 1;

    cv::Mat dst;
    addWeighted(a,1.0,b,-1.0,0.0,dst, CV_32F);
    return dst;
}

class LineSegment{
public:
    cv::Point2d p1, p2;
    double slope = 999;
    double intercept = 0;
    bool valid = true;

    LineSegment(cv::Point2d p1_, cv::Point2d p2_){
        p1 = p1_;
        p2 = p2_;
        if(p1.x != p2.x){
            slope = (p2.y - p1.y) / (p2.x - p1.x);
            intercept = p1.y - slope * p1.x;
            
        }else{
            // valid = false;
        }
        
    }

    bool is_valid(){
        return valid;
    }
};

static double orientation(const cv::Point2d &a, const cv::Point2d &b, const cv::Point2d &c) {
	float val = (b.y - a.y)*(c.x - b.x) - (b.x - a.x)*(c.y - b.y);
	if(val == 0) {
		return 0;
	}
	return (val > 0) ? 1 : 2; // 1 : clockwise , 2 : counterclockwise
}

static bool onSegment(const cv::Point2d &a, const cv::Point2d &c, const cv::Point2d &b) {
	if((c.x <= std::max(a.x, b.x) && c.x >= std::min(a.x, b.x)) && (c.y <= std::max(a.y, b.y) && c.y >= std::min(a.y, b.y))) {
		return true;
	}
	return false;
}

static bool doSegmentIntersect(std::shared_ptr<LineSegment> s1, std::shared_ptr<LineSegment> s2) {
    auto a = s1 -> p1;
    auto b = s1 -> p2;
    auto c = s2 -> p1;
    auto d = s2 -> p2;

	double o1 = orientation(a, b, c);
	double o2 = orientation(a, b, d);
	double o3 = orientation(c, d, a);
	double o4 = orientation(c, d, b);

    bool intersects = false;

	if(o1 != o2 && o3 != o4) intersects = true;
	else if(o1 == 0 && onSegment(a, c, b)) intersects = false;
	else if(o2 == 0 && onSegment(a, d, b)) intersects = false;
	else if(o3 == 0 && onSegment(c, a, d)) intersects = false;
	else if(o4 == 0 && onSegment(c, b, d)) intersects = false;

	return intersects;
}

// static std::vector<std::string> get_files(std::string path){
//     std::vector<std::string> files;

//     for (const auto & entry : std::filesystem::directory_iterator(path)){
//             files.push_back(path);
//     }

//     return files;
// }


}//namespace utils