#pragma once
#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <deque>
#include <algorithm>
#include <condition_variable>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/cudabgsegm.hpp>
#include <opencv2/bgsegm.hpp>
namespace Traffic{

enum BGEstimatorType{
    CNT,
    GMG,
    GSOC,
    LSBP,
    MOG,

};

struct BGEstimatorCNTOptions{
    int minPixelStability = 15;
    bool useHistory = true;
    int maxPixelStability = 15*60;
    bool isParallel = true;
};

struct BGEstimatorGMGOptions{
    int initializationFrames=120;
    double decisionThreshold=0.8;
};

struct BGEstimatorGSOCOptions{
    int mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
    int nSamples = 20;
    float replaceRate = 0.003f;
    float propagationRate = 0.01f;
    int hitsThreshold = 32;
    float alpha = 0.01f;
    float beta = 0.0022f;
    float blinkingSupressionDecay = 0.1f;
    float blinkingSupressionMultiplier = 0.1f;
    float noiseRemovalThresholdFacBG = 0.0004f;
    float noiseRemovalThresholdFacFG = 0.0008f;
};

struct BGEstimatorLSBPOptions{
    int mc = cv::bgsegm::LSBPCameraMotionCompensation::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
    int nSamples = 20;
    int LSBPRadius = 16;
    float Tlower = 2.0f;
    float Tupper = 32.0f;
    float Tinc = 1.0f;
    float Tdec = 0.05f;
    float Rscale = 10.0f;
    float Rincdec = 0.005f;
    float noiseRemovalThresholdFacBG = 0.0004f;
    float noiseRemovalThresholdFacFG = 0.0008f;
    int LSBPthreshold = 8;
    int minCount = 2;
};

struct BGEstimatorMOGOptions{
    int history=200;
    int nmixtures=5;
    double backgroundRatio=0.7;
    double noiseSigma=0;
};

struct BGEstimatorOptions{
    BGEstimatorType type = BGEstimatorType::MOG;;
    BGEstimatorCNTOptions CNT;
    BGEstimatorGMGOptions GMG;
    BGEstimatorGSOCOptions GSOC;
    BGEstimatorLSBPOptions LSBP;
    BGEstimatorMOGOptions MOG;
};


class BackgroundEstimator{
private:
    bool ready_ = false;
    
    BGEstimatorOptions options_;

    cv::Ptr<cv::BackgroundSubtractor> background_sub_;
    cv::Mat fg_mask_;

    std::condition_variable cv_update_;
    std::mutex mtx_notify_update_;

public:

    BackgroundEstimator(){
    }

    bool is_ready(){ return ready_; };
    std::condition_variable& get_cv_update(){ return cv_update_; }
    std::mutex& get_mtx_update(){ return mtx_notify_update_; }
    BGEstimatorOptions &get_options_ref() { return options_; }


    void update(cv::Mat &frame){
        background_sub_ -> apply(frame, fg_mask_);
        cv_update_.notify_all();
    }

    cv::Mat get_background(){
        cv::Mat bg;
        background_sub_ -> getBackgroundImage(bg);
        return bg;
    }

    cv::Mat get_mask(){
        return fg_mask_;
    }

    void create_CNT(){
        background_sub_ = cv::bgsegm::createBackgroundSubtractorCNT(    options_.CNT.minPixelStability,
                                                                        options_.CNT.useHistory,
                                                                        options_.CNT.maxPixelStability,
                                                                        options_.CNT.isParallel
                                                                    );
    }

    void create_GMG(){
        background_sub_ = cv::bgsegm::createBackgroundSubtractorGMG(    options_.GMG.initializationFrames,
                                                                        options_.GMG.decisionThreshold
                                                                    );
    }

    void create_GSOC(){
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
        background_sub_ = cv::bgsegm::createBackgroundSubtractorMOG(    options_.MOG.history,
                                                                        options_.MOG.nmixtures,
                                                                        options_.MOG.backgroundRatio,
                                                                        options_.MOG.noiseSigma  
                                                                    );
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