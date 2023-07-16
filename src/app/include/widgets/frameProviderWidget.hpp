#pragma once
#include "imgui.h"
#include "frameProvider.hpp"
#include "widget.hpp"
#include "imfilebrowser.h"
#include <thread>
#include <chrono>

#include "GL/gl3w.h"
#include <SDL.h>

namespace Traffic{

class FrameProviderWidget : public Widget{

private:

    std::shared_ptr<FrameProvider> frame_provider_;
    std::shared_ptr<ImGui::FileBrowser> fileDialog;

    bool from_directory_ = false;
    bool video_ = false;
    bool display_frames = false;
    
public:

    FrameProviderWidget(std::shared_ptr<FrameProvider> frame_provider, bool hidden = false)
    :
        frame_provider_{frame_provider}
    {
        
        set_hidden_state(hidden);
    }

    void gui() override {
        ImGui::Begin("Image/Video Source");
    
        static int selection = 0;
        ImGui::RadioButton("Images",&selection, 0); ImGui::SameLine();
        ImGui::RadioButton("Video", &selection, 1);
        video_ = selection;

        ImGui::SameLine();
        if(video_){
            if(ImGui::Button("Select File")){
                fileDialog = std::make_shared<ImGui::FileBrowser>();
                set_file_types();
                fileDialog -> Open();
            }
        }else{
            if(ImGui::Button("Select Directory")){
                fileDialog = std::make_shared<ImGui::FileBrowser>(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_SelectDirectory);
                set_file_types();
                fileDialog -> Open();
            }
        }

        if(ImGui::Button("Show frames")){
            frame_provider_ -> start();
            gl3wInit();
            display_frames = true;
        }

        
        if(frame_provider_ -> is_ready() && display_frames){
            cv::Mat frame = frame_provider_ -> get_next_frame();
            GLuint texture;
            
            if(frame.empty()){
                frame_provider_ -> stop();
            }
            else{

                // ImGui::Begin("Frames");
                APP_DEBUG(FP_DEBUG_NAME "Displaying frame {}", frame_provider_ -> frame_cnt_);
                frame_provider_ -> frame_cnt_++;

                // std::this_thread::sleep_for(std::chrono::milliseconds(16));
                cv::imwrite("./saves/test_"+ std::to_string(frame_provider_ -> frame_cnt_)+".png", frame);
                glGenTextures( 1, &texture );
                glBindTexture( GL_TEXTURE_2D, texture );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_BGR
                            , GL_UNSIGNED_BYTE, frame.data );

                ImGui::Image( reinterpret_cast<void*>( static_cast<intptr_t>( texture ) ), ImVec2( frame.cols, frame.rows ) );
                // frame_provider_ ->show_frame(frame);
            }
            
            // ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
        }
        

        ImGui::End();
        
        if(fileDialog){
    
            fileDialog -> Display();
            if(fileDialog -> HasSelected())
            {
                std::string path = fileDialog -> GetSelected().string();
                frame_provider_ -> set_path(path);
                frame_provider_ -> set_video_mode(video_);
                fileDialog -> ClearSelected();
            }
        }

    }

    void set_file_types(){
        if(video_){
            fileDialog -> SetTypeFilters({".mkv", ".mp4", ".avi"});
        }
        else{
            fileDialog -> SetTypeFilters({".jpeg", ".jpg", ".png"});
        }
    }

};

} // namespace Traffic