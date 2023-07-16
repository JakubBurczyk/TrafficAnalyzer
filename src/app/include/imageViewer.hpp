#pragma once

#include <opencv2/opencv.hpp>
#include <SDL.h>

#include "GL/gl3w.h"
#include "imgui.h"

class ImageViewer{
private:
    static bool initialized_;
    static std::mutex mtx_init_;

    GLuint texture_;
    float width_ = 0;
    float height_ = 0;

    void set_image(cv::Mat &frame){
        width_ = frame.cols;
        height_ = frame.rows;

        glGenTextures( 1, &texture_ );
        glBindTexture( GL_TEXTURE_2D, texture_ );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_BGR
                    , GL_UNSIGNED_BYTE, frame.data );
 
    }

public:

    ImageViewer(){
        std::unique_lock<std::mutex>(ImageViewer::mtx_init_);
        if(!ImageViewer::initialized_){
            gl3wInit();
        }
    }
    static void init(){
        
    }

    void show_image(cv::Mat &frame){
        set_image(frame);
        ImGui::Image( reinterpret_cast<void*>( static_cast<intptr_t>( texture_ ) ), ImVec2( width_, height_ ) );
    } 

};


