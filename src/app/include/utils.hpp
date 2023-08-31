#pragma once
#include <opencv2/opencv.hpp>
#include <string_view>
#include <filesystem>
#include <chrono>

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


// static std::vector<std::string> get_files(std::string path){
//     std::vector<std::string> files;

//     for (const auto & entry : std::filesystem::directory_iterator(path)){
//             files.push_back(path);
//     }

//     return files;
// }


}//namespace utils