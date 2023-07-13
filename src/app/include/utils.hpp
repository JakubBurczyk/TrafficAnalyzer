#pragma once

#include <string_view>
#include <filesystem>


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

    APP_DEBUG(UTILS_DEBUG_NAME "Loaded: {} files", files.size());
    return files;
}

// static std::vector<std::string> get_files(std::string path){
//     std::vector<std::string> files;

//     for (const auto & entry : std::filesystem::directory_iterator(path)){
//             files.push_back(path);
//     }

//     return files;
// }


}//namespace utils