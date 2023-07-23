#pragma once
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/cudabgsegm.hpp>
#include <opencv2/bgsegm.hpp>

namespace Traffic{

enum class BGEstimatorType : int32_t {
    CNT = 1,
    GMG = 2,
    GSOC = 3,
    LSBP = 4,
    MOG = 5,
    CUDA_MOG = 6,
    CUDA_MOG2 = 7

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

struct BGEstimatorCUDAMOGOptions{
    int history = 200;
    int nmixtures = 5;
    double backgroundRatio = 0.7;
    double noiseSigma = 0;
};

struct BGEstimatorCUDAMOG2Options{
    int history = 500;
    double varThreshold = 16;
    bool detectShadows = true;
};

struct BGEstimatorOptions{
    BGEstimatorType type = BGEstimatorType::MOG;
    BGEstimatorCNTOptions CNT;
    BGEstimatorGMGOptions GMG;
    BGEstimatorGSOCOptions GSOC;
    BGEstimatorLSBPOptions LSBP;
    BGEstimatorMOGOptions MOG;
    BGEstimatorCUDAMOGOptions CUDA_MOG;
    BGEstimatorCUDAMOG2Options CUDA_MOG_2;
    bool CUDA = false;
};

} // namespace Traffic