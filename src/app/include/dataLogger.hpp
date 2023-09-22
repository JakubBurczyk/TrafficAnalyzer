#pragma once

#include <mutex>
#include <filesystem>
#include <map>
#include <fstream>

#include "utils.hpp"
#include "json.hpp"

namespace fs = std::filesystem;

namespace Traffic{

struct LoggerOptions
{
    bool stamp_filenames = true;
};

struct LogOptions
{
    uint32_t id;
    std::string filename;
    fs::path path;
    bool append = true;
    
};


class Log{
private:
    LogOptions options_;
    std::mutex file_mtx_;
    std::fstream file_;

    std::ios_base::openmode flags_ = std::fstream::in | std::fstream::out | std::fstream::trunc;

protected:

    void set_append_mode(bool mode){
        flags_ = std::fstream::out | std::fstream::in ;
        flags_ = flags_ | mode ? std::fstream::app : std::fstream::trunc;
    }

    bool is_open(){
        return file_.is_open();
    }

    nlohmann::json construct_entry(nlohmann::json data){
        nlohmann::json entry;
        entry["timestamp_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        entry["data"] = data;
        return entry;
    }

    std::string entry_to_string(nlohmann::json entry){
        return entry.dump(-1);
    }

public:

    Log(LogOptions options)
    :
    options_{options}
    {
        options_.path.replace_filename(options_.filename);
        options_.path.replace_extension("json");
        set_append_mode(options_.append);
    }

    ~Log(){
        stop();
    }


    bool start(){
        std::unique_lock<std::mutex> lck(file_mtx_);
        file_.open(options_.path,flags_);
        return is_open();
    }

    bool stop(){
        std::unique_lock<std::mutex> lck(file_mtx_);

        if(is_open()){
            std::cout << "Logfile " << options_.path << " is open and being closed" << std::endl;
            // file_.close();
        }
        
    }

    bool log(nlohmann::json data){
        std::unique_lock<std::mutex> lck(file_mtx_);
        bool status = is_open();
        
        if(status){

            std::string entry = entry_to_string(construct_entry(data));
            // std::cout << "Logging data: " << entry << std::endl;
            try{
                file_ << entry << std::endl;
            }
            catch(std::exception &e){

            }
            
        }

        return status;
    }

    

};

class DataLogger{
private:

    LoggerOptions options_;
    
    uint32_t free_id_ = 0;
    std::mutex mtx_id_;
    
    std::map<uint32_t, std::shared_ptr<Log>> logfiles_;

protected:

    uint32_t take_next_id(){
        std::unique_lock<std::mutex> lck(mtx_id_);
        auto assigned = free_id_;
        free_id_ += 1;
        return assigned;
    }

public:

    DataLogger(LoggerOptions options)
    :
    options_{options}    
    {

    }

    uint32_t add_logfile(std::string filename, std::string path){
        LogOptions logOpts;
        logOpts.id = this -> take_next_id();
        logOpts.filename = filename;

        if(options_.stamp_filenames){
            logOpts.filename = logOpts.filename + "_" + utils::return_current_time_and_date();
        }

        
        logOpts.path = path;
        logfiles_[logOpts.id] = std::make_shared<Log>(logOpts);

        return logOpts.id;
    }

    bool log(uint32_t file, nlohmann::json data){
        return logfiles_[file] -> log(data);
    }

    void start(){
        for(auto p : logfiles_){
            p.second -> start();
        }
    }

    void stop(){
        std::cout << "DL | Closing logfiles" << std::endl;
        for(auto p : logfiles_){
            p.second -> stop();
        }
        std::cout << "DL | Closed logfiles" << std::endl;
    }

};

}// namespace Traffic