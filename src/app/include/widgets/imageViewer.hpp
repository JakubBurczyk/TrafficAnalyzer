#pragma once

#include <opencv2/opencv.hpp>
#include <SDL.h>

#include "GL/gl3w.h"
#include "imgui.h"
#include "imfilebrowser.h"
#include "utils.hpp"

class ImageViewer{
private:
    static bool initialized_;
    static int active_instances_;
    static std::mutex mtx_init_;
    static std::mutex mtx_show_;

    bool enabled_ = false;

    GLuint texture_;
    std::string name ="";
    std::mutex mtx_tex;

    float width_ = 0;
    float height_ = 0;
    
    int scale_w_ = 10;
    int scale_h_ = 10;

    float INT_SCALE_FACTOR = 10;

    std::shared_ptr<ImGui::FileBrowser> fileDialog;
    std::string save_path = "./";
    char filename[512] = "Image";
public:

    ImageViewer(){
        std::unique_lock<std::mutex>(ImageViewer::mtx_init_);
        if(!ImageViewer::initialized_){
            gl3wInit();
        }
        active_instances_++;

        glGenTextures( 1, &texture_ );

        name = "[" + std::to_string(texture_) + "]";

        fileDialog = std::make_shared<ImGui::FileBrowser>(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_SelectDirectory);
    }

    bool is_enabled(){ return enabled_; }

    void toggle_enabled(){ enabled_ = !enabled_; }

    void set_width_scale(double scale) { scale_w_ = scale; }
    void set_height_scale(double scale) { scale_h_ = scale; }

    int &get_width_scale_ref(){ return scale_w_; };
    int &get_height_scale_ref(){ return scale_h_; };

    void set_image(cv::Mat &frame){
        glBindTexture( GL_TEXTURE_2D, texture_ );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

        cv::Mat resized_;
        cv::resize(frame, resized_, cv::Size(), scale_w_/INT_SCALE_FACTOR, scale_h_/INT_SCALE_FACTOR, cv::INTER_LINEAR);
        
        width_ = resized_.cols;
        height_ = resized_.rows;

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_BGR
                    , GL_UNSIGNED_BYTE, resized_.data );
 
    }

    void show_image(cv::Mat &frame){
        
        std::unique_lock<std::mutex>(ImageViewer::mtx_show_);
        
        if(!enabled_){
            ImGui::Text("Image preview disabled");
        }else{

            if(ImGui::Button(("Select Save Directory" + name).c_str())){
                fileDialog -> Open();
            }

            if(fileDialog){
                fileDialog -> Display();
                if(fileDialog -> HasSelected())
                {
                    save_path = fileDialog -> GetSelected().string() + "/";
                    fileDialog -> ClearSelected();
                }
            }

            ImGui::SameLine();
            
            ImGui::PushItemWidth(150);
            ImGui::InputTextWithHint(("Filename " + name).c_str(), "filename",filename,512);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            if(ImGui::Button(("Save Image "+ name).c_str())){
                std::string date = utils::return_current_time_and_date();
                std::string path = save_path + std::string(filename) +  "_" + date + ".png";
                std::cout << "Saving image to: " << path << std::endl;
                cv::imwrite(path,cv::Mat(frame));
            }

            if(ImGui::CollapsingHeader(("Image Controls" + name).c_str(), ImGuiTreeNodeFlags_None))
            {
                ImGui::PushItemWidth(500);
                ImGui::SliderInt(("Width scale" + name).c_str(), &scale_w_, 1, 10);
                ImGui::SliderInt(("Heightd scale" + name).c_str() , &scale_h_, 1, 10);
            }
            ImGui::PopItemWidth();
            if(frame.empty()){
                ImGui::Text("Image is empty");
            }else{
                set_image(frame);
                ImGui::Image( reinterpret_cast<void*>( static_cast<intptr_t>( texture_ ) ), ImVec2( width_, height_ ) );
            }
        }

            
        
    } 

};


