#include "device_agent.h"

#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/kit/debug.h>
#include <nx/kit/utils.h>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Thêm thư viện fstream để thay thế fopen
#include <fstream> 

#if defined(_WIN32)
    #include <windows.h>
#endif

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

// Helper lấy đường dẫn DLL
std::string getPluginDir() {
#if defined(_WIN32)
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR) &getPluginDir, &hm) == 0) return "";
    GetModuleFileNameA(hm, path, sizeof(path));
    std::string fullPath(path);
    return fullPath.substr(0, fullPath.find_last_of("\\/"));
#else
    return ".";
#endif
}

DeviceAgent::DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo):
    ConsumingDeviceAgent(deviceInfo, /*enableOutput*/ true)
{
    m_modelPath = getPluginDir() + "/model.onnx";
    NX_PRINT << "[FacePlugin] KHOI TAO. Model path: " << m_modelPath;
    
    // SỬA: Dùng ifstream để kiểm tra file (tránh warning C4996)
    std::ifstream f(m_modelPath.c_str());
    if (f.good()) {
        NX_PRINT << "[FacePlugin] Tim thay file model!";
    } else {
        NX_PRINT << "[FacePlugin] LOI: Khong tim thay file model.onnx!";
    }

    m_faceDetector = std::make_unique<FaceDetector>(m_modelPath);
}

DeviceAgent::~DeviceAgent() {}

std::string DeviceAgent::manifestString() const {
    return R"json({
        "typeLibrary": {
            "objectTypes": [
                {
                    "id": "face.detection.object",
                    "name": "Face",
                    "attributes": []
                }
            ]
        }
    })json";
}

// Hàm convert YUV -> BGR (giữ nguyên logic đã sửa trước đó)
cv::Mat convertYUVtoBGR(const nx::sdk::analytics::IUncompressedVideoFrame* frame) {
    int w = frame->width();
    int h = frame->height();
    
    const uint8_t* dataY = (const uint8_t*)frame->data(0);
    const uint8_t* dataU = (const uint8_t*)frame->data(1);
    const uint8_t* dataV = (const uint8_t*)frame->data(2);

    int stepY = frame->lineSize(0);
    int stepU = frame->lineSize(1);
    int stepV = frame->lineSize(2);

    cv::Mat yMat(h, w, CV_8UC1);
    // Copy Y plane
    if (stepY == w) {
        memcpy(yMat.data, dataY, w * h);
    } else {
        for (int i = 0; i < h; i++) memcpy(yMat.data + i * w, dataY + i * stepY, w);
    }

    cv::Mat uMat(h / 2, w / 2, CV_8UC1);
    cv::Mat vMat(h / 2, w / 2, CV_8UC1);

    // Copy U, V planes
    for (int i = 0; i < h / 2; i++) {
        memcpy(uMat.data + i * (w / 2), dataU + i * stepU, w / 2);
        memcpy(vMat.data + i * (w / 2), dataV + i * stepV, w / 2);
    }

    // Ghép thành YUV I420 continuous
    cv::Mat yuv(h + h / 2, w, CV_8UC1);
    memcpy(yuv.data, yMat.data, w * h);
    memcpy(yuv.data + w * h, uMat.data, (w * h) / 4);
    memcpy(yuv.data + w * h + (w * h) / 4, vMat.data, (w * h) / 4);

    cv::Mat bgr;
    cv::cvtColor(yuv, bgr, cv::COLOR_YUV2BGR_I420);
    return bgr;
}

bool DeviceAgent::pushUncompressedVideoFrame(const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame)
{
    cv::Mat image = convertYUVtoBGR(videoFrame);
    if (image.empty()) return true;

    // Detect
    std::vector<DetectionResult> results = m_faceDetector->detect(image, 0.4f, 0.4f);

    if (results.empty()) return true;

    // In log detection
    static int logCounter = 0;
    if (logCounter++ % 30 == 0) { // Log mỗi 30 detection để đỡ spam
        NX_PRINT << "[FacePlugin] Phat hien: " << results.size() << " khuon mat.";
    }

    auto metadataPacket = makePtr<ObjectMetadataPacket>();
    metadataPacket->setTimestampUs(videoFrame->timestampUs());
    metadataPacket->setDurationUs(0);

    for (const auto& det : results) {
        auto objectMetadata = makePtr<ObjectMetadata>();
        objectMetadata->setTypeId("face.detection.object");
        objectMetadata->setConfidence(det.confidence);

        float x = (float)det.box.x / image.cols;
        float y = (float)det.box.y / image.rows;
        float w = (float)det.box.width / image.cols;
        float h = (float)det.box.height / image.rows;

        if (x < 0) x = 0; if (y < 0) y = 0;
        if (x + w > 1.0f) w = 1.0f - x;
        if (y + h > 1.0f) h = 1.0f - y;

        objectMetadata->setBoundingBox(Rect(x, y, w, h));
        metadataPacket->addItem(objectMetadata.get());
    }

    pushMetadataPacket(metadataPacket.releasePtr());
    return true;
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* /*outValue*/,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
{
}

} // namespace face_detection_plugin