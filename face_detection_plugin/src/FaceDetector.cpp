#include "FaceDetector.h"
#include <iostream>

FaceDetector::FaceDetector(const std::string& modelPath) {
    try {
        net = cv::dnn::readNetFromONNX(modelPath);
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    } catch (const cv::Exception& e) {
        std::cerr << "FaceDetector Error: " << e.what() << std::endl;
    }
}

std::vector<DetectionResult> FaceDetector::detect(const cv::Mat& image, float confThreshold, float nmsThreshold) {
    std::vector<DetectionResult> finalResults;
    if (image.empty()) return finalResults;

    cv::Mat blob;
    cv::dnn::blobFromImage(image, blob, 1.0 / 255.0, inputSize, cv::Scalar(), true, false);
    
    net.setInput(blob);

    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    // Xử lý Output YOLOv8 (Transpose [1, 5, 8400] -> [1, 8400, 5])
    cv::Mat output = outputs[0];
    int rows = output.size[2];
    int dimensions = output.size[1];
    
    output = output.reshape(1, dimensions);
    cv::Mat t_output = output.t();
    
    float* data = (float*)t_output.data;

    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    std::vector<int> classIds;

    float x_factor = (float)image.cols / inputSize.width;
    float y_factor = (float)image.rows / inputSize.height;

    for (int i = 0; i < rows; ++i) {
        float confidence = data[4]; // Score (Giả sử class Face ở index 4)

        if (confidence >= confThreshold) {
            float cx = data[0];
            float cy = data[1];
            float w = data[2];
            float h = data[3];

            int left = int((cx - 0.5 * w) * x_factor);
            int top = int((cy - 0.5 * h) * y_factor);
            int width = int(w * x_factor);
            int height = int(h * y_factor);

            boxes.push_back(cv::Rect(left, top, width, height));
            confidences.push_back(confidence);
            classIds.push_back(0);
        }
        data += dimensions;
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    for (int idx : indices) {
        DetectionResult res;
        res.box = boxes[idx];
        res.confidence = confidences[idx];
        res.classId = classIds[idx];
        finalResults.push_back(res);
    }

    return finalResults;
}