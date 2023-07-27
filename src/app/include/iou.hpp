#pragma once
#include <cstdlib>
#include <opencv2/opencv.hpp>

class IoU{

public:
    static double calculate_iou(const cv::Rect &r1, const cv::Rect &r2){
        double iou = -1;

        int x = std::abs(r1.x - r2.x);
        int y = std::abs(r1.y - r2.y);

        int width  = r1.width * (r1.x <= r2.x) + r2.width * (r1.x > r2.x);
        int height  = r1.height * (r1.y <= r2.y) + r2.height * (r1.y > r2.y);


        int intersection_width = (width - x);
        int intersection_height = (height - y);
        int intersection_area = intersection_width * intersection_height;
        int union_area = r1.area() + r2.area() - intersection_area;

        

        if(intersection_width > 0 && intersection_height >0)
        {
            iou = (double)intersection_area / (double)union_area;
        }
        
        return iou;
    }

};