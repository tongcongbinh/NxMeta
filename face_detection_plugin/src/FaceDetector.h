#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>

struct DetectionResult {
    cv::Rect box;
    float confidence;
    int classId;
};

class FaceDetector {
public:
    FaceDetector(const std::string& modelPath);
    // Hàm detect trả về danh sách kết quả
    std::vector<DetectionResult> detect(const cv::Mat& image, float confThreshold = 0.5f, float nmsThreshold = 0.4f);

private:
    cv::dnn::Net net;
    // Kích thước input model (YOLO thường là 640x640 hoặc 320x320 tùy model bạn train)
    const cv::Size inputSize = cv::Size(640, 640); 
};