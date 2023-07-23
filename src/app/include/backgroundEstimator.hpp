#pragma once
#include <string>
#include <filesystem>
#include <deque>
#include <algorithm>
#include <condition_variable>

#include <opencv2/opencv.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/cudabgsegm.hpp>
#include <opencv2/bgsegm.hpp>

#include "backgroundEstimator_data.hpp"

namespace Traffic{

class BackgroundEstimator{
private:
    bool ready_ = false;
    
    BGEstimatorOptions options_;

    cv::Ptr<cv::BackgroundSubtractor> background_sub_;
    cv::Mat fg_mask_;
    cv::cuda::GpuMat fg_mask_gpu_;
    cv::cuda::GpuMat frame_gpu_;
    cv::Mat frame_;

    cv::cuda::GpuMat bg_gpu_;

    std::condition_variable cv_update_;
    std::mutex mtx_notify_update_;

public:

    BackgroundEstimator(){
    }

    bool is_ready(){ return ready_; };
    std::condition_variable& get_cv_update(){ return cv_update_; }
    std::mutex& get_mtx_update(){ return mtx_notify_update_; }
    BGEstimatorOptions &get_options_ref() { return options_; }

    void save_mask(std::string filename){
        cv::imwrite(filename, fg_mask_);
    }

    void save_frame(std::string filename){
        cv::imwrite(filename, frame_);
    }

    void update(cv::Mat &frame){
        if(options_.CUDA){
            frame_ = frame.clone();
            frame_gpu_.upload(frame);
            background_sub_ -> apply(frame_gpu_, fg_mask_gpu_);
            fg_mask_ = cv::Mat(fg_mask_gpu_);
            
        }else{
            background_sub_ -> apply(frame, fg_mask_);
        }
        
        cv_update_.notify_all();
    }

    cv::Mat get_background(){
        cv::Mat bg;
        if(options_.CUDA){
            background_sub_ -> getBackgroundImage(bg_gpu_);
            bg = cv::Mat(bg_gpu_);
        }else{
            background_sub_ -> getBackgroundImage(bg);
        }   
        
        return bg;
    }

    cv::Mat get_mask(){
        return fg_mask_;
    }

    void create_CNT(){
        options_.CUDA = false;
        background_sub_ = cv::bgsegm::createBackgroundSubtractorCNT(    options_.CNT.minPixelStability,
                                                                        options_.CNT.useHistory,
                                                                        options_.CNT.maxPixelStability,
                                                                        options_.CNT.isParallel
                                                                    );
    }

    void create_GMG(){
        options_.CUDA = false;
        background_sub_ = cv::bgsegm::createBackgroundSubtractorGMG(    options_.GMG.initializationFrames,
                                                                        options_.GMG.decisionThreshold
                                                                    );
    }

    void create_GSOC(){
        options_.CUDA = false;
        background_sub_ = cv::bgsegm::createBackgroundSubtractorGSOC(   options_.GSOC.mc,
                                                                        options_.GSOC.nSamples,
                                                                        options_.GSOC.replaceRate,
                                                                        options_.GSOC.propagationRate,
                                                                        options_.GSOC.hitsThreshold,
                                                                        options_.GSOC.alpha,
                                                                        options_.GSOC.beta,
                                                                        options_.GSOC.blinkingSupressionDecay,
                                                                        options_.GSOC.blinkingSupressionMultiplier,
                                                                        options_.GSOC.noiseRemovalThresholdFacBG,
                                                                        options_.GSOC.noiseRemovalThresholdFacFG
                                                                    );
    }

    void create_LSBP(){
        options_.CUDA = false;
        background_sub_ = cv::bgsegm::createBackgroundSubtractorLSBP(   options_.LSBP.mc,
                                                                        options_.LSBP.nSamples,
                                                                        options_.LSBP.LSBPRadius,
                                                                        options_.LSBP.Tlower,
                                                                        options_.LSBP.Tupper,
                                                                        options_.LSBP.Tinc,
                                                                        options_.LSBP.Tdec,
                                                                        options_.LSBP.Rscale,
                                                                        options_.LSBP.Rincdec,
                                                                        options_.LSBP.noiseRemovalThresholdFacBG,
                                                                        options_.LSBP.noiseRemovalThresholdFacFG,
                                                                        options_.LSBP.LSBPthreshold,
                                                                        options_.LSBP.minCount
                                                                    );
    }

    void create_MOG(){
        options_.CUDA = false;
        background_sub_ = cv::bgsegm::createBackgroundSubtractorMOG(    options_.MOG.history,
                                                                        options_.MOG.nmixtures,
                                                                        options_.MOG.backgroundRatio,
                                                                        options_.MOG.noiseSigma  
                                                                    );
    }

    void create_CUDA_MOG(){
        options_.CUDA = true;
        background_sub_ = cv::cuda::createBackgroundSubtractorMOG();
    }

    void create_CUDA_MOG2(){
        options_.CUDA = true;
        background_sub_ = cv::cuda::createBackgroundSubtractorMOG2(2000);
    }

    bool init(){
        ready_ = false;

        switch(options_.type){
            case BGEstimatorType::CNT:{
                create_CNT();
                break;
            }
            case BGEstimatorType::GMG:{
                create_GMG();
                break;
            }
            case BGEstimatorType::GSOC:{
                create_GSOC();
                break;
            }
            case BGEstimatorType::LSBP:{
                create_LSBP();
                break;
            }
            case BGEstimatorType::MOG:{
                create_MOG();
                break;
            }

            case BGEstimatorType::CUDA_MOG:{
                create_CUDA_MOG();
                break;
            }

            case BGEstimatorType::CUDA_MOG2:{
                create_CUDA_MOG2();
                break;
            }

            default:{
                return false;
                break;
            }
            
        }

        ready_ = true;

        return ready_;
    };

};

} // namespace Traffic