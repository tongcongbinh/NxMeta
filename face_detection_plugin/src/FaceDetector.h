#pragma once
#include <opencv2/opencv.hpp>
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
    std::vector<DetectionResult> detect(const cv::Mat& image, float confThreshold = 0.5f, float nmsThreshold = 0.4f);

private:
    cv::dnn::Net net;
    const cv::Size inputSize = cv::Size(640, 640);
};