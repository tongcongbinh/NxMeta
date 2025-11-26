#include "FaceDetector.h"
#include <iostream>

FaceDetector::FaceDetector(const std::string& modelPath) {
    try {
        std::cout << "Loading ONNX model from: " << modelPath << std::endl;
        net = cv::dnn::readNetFromONNX(modelPath);
        
        // Dùng CPU cho ổn định (nếu có CUDA thì đổi sau)
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    } catch (const cv::Exception& e) {
        std::cerr << "FaceDetector Error: " << e.what() << std::endl;
    }
}

std::vector<DetectionResult> FaceDetector::detect(const cv::Mat& image, float confThreshold, float nmsThreshold) {
    std::vector<DetectionResult> finalResults;
    if (image.empty()) return finalResults;

    // 1. Preprocess: Resize & Normalize
    // YOLO thường dùng scale 1/255.0, swapRB=true (nếu input là BGR thì chuyển thành RGB)
    cv::Mat blob;
    cv::dnn::blobFromImage(image, blob, 1.0 / 255.0, inputSize, cv::Scalar(), true, false);
    
    net.setInput(blob);

    // 2. Inference
    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    // 3. Post-process (YOLOv8 Logic)
    // Output YOLOv8 gốc thường là: [Batch, Channels, Anchors] -> [1, 5, 8400] (với 1 class face)
    // 5 Channels gồm: cx, cy, w, h, score
    cv::Mat output = outputs[0];
    
    // Transpose để chuyển thành [1, 8400, 5] cho dễ duyệt
    // output.size[1] là số channels (5), output.size[2] là số anchors (8400)
    int rows = output.size[2]; 
    int dimensions = output.size[1];
    
    // Reshape về 2D: [5, 8400] sau đó Transpose thành [8400, 5]
    output = output.reshape(1, dimensions); 
    cv::Mat t_output = output.t();
    
    float* data = (float*)t_output.data;

    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    std::vector<int> classIds;

    // Hệ số scale để map lại bounding box về kích thước ảnh gốc
    float x_factor = (float)image.cols / inputSize.width;
    float y_factor = (float)image.rows / inputSize.height;

    for (int i = 0; i < rows; ++i) {
        // Với YOLOv8 1 class:
        // data[0]=cx, data[1]=cy, data[2]=w, data[3]=h, data[4]=score
        float confidence = data[4]; 

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
            classIds.push_back(0); // Class Face = 0
        }
        // Di chuyển con trỏ đến hàng tiếp theo
        data += dimensions;
    }

    // 4. Non-Maximum Suppression (NMS) để loại bỏ box trùng lặp
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