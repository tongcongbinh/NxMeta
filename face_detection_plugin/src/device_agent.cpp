#include "device_agent.h"
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/kit/debug.h>
#include <nx/kit/utils.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <chrono>
#include <algorithm>

#if defined(_WIN32)
    #include <windows.h>
#endif

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

// Helper lấy đường dẫn
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
    
    // Kiểm tra model (chỉ log 1 lần)
    std::ifstream f(m_modelPath.c_str());
    if (f.good()) NX_PRINT << "[FacePlugin] Model OK: " << m_modelPath;
    else NX_PRINT << "[FacePlugin] ERROR: Model not found!";

    m_faceDetector = std::make_unique<FaceDetector>(m_modelPath);
}

DeviceAgent::~DeviceAgent() {}

nx::sdk::Uuid DeviceAgent::trackIdByIndex(int index)
{
    while (index >= (int)m_trackIds.size())
        m_trackIds.push_back(UuidHelper::randomUuid());

    return m_trackIds[index];
}

std::string DeviceAgent::manifestString() const
{
    // QUAN TRỌNG: Trả về rỗng để Client dùng typeLibrary bên plugin.cpp
    return R"json({
        "supportedTypes": [
            {
                "objectTypeId": "face.detection.object",
                "attributes": []
            }
        ]
    })json";
}

// Hàm convert YUV -> BGR
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
    if (stepY == w) memcpy(yMat.data, dataY, w * h);
    else for (int i = 0; i < h; i++) memcpy(yMat.data + i * w, dataY + i * stepY, w);

    cv::Mat uMat(h / 2, w / 2, CV_8UC1);
    cv::Mat vMat(h / 2, w / 2, CV_8UC1);
    for (int i = 0; i < h / 2; i++) {
        memcpy(uMat.data + i * (w / 2), dataU + i * stepU, w / 2);
        memcpy(vMat.data + i * (w / 2), dataV + i * stepV, w / 2);
    }

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
    // --- FRAME SKIPPING: Giảm tải CPU ---
    static int frameCounter = 0;
    frameCounter++;
    if (frameCounter % 5 != 0) return true; // Xử lý 1 frame, bỏ 4 frame

    cv::Mat image = convertYUVtoBGR(videoFrame);
    if (image.empty()) return true;

    // Detect (ngưỡng 0.4)
    std::vector<DetectionResult> results = m_faceDetector->detect(image, 0.4f, 0.4f);
    if (results.empty()) return true;

    // --- GỬI METADATA ---
    auto metadataPacket = makePtr<ObjectMetadataPacket>();
    metadataPacket->setTimestampUs(videoFrame->timestampUs());
    
    // Duration 200ms (đủ để che lấp khoảng trống 5 frame)
    metadataPacket->setDurationUs(200000); 

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& det = results[i];
        auto objectMetadata = makePtr<ObjectMetadata>();
        
        // ID PHẢI KHỚP VỚI FILE plugin.cpp
        objectMetadata->setTypeId("face.detection.object"); 
        objectMetadata->setConfidence(det.confidence);

        // Set Track ID để client hiển thị bounding box
        objectMetadata->setTrackId(trackIdByIndex((int)i));

        float x = (float)det.box.x / image.cols;
        float y = (float)det.box.y / image.rows;
        float w = (float)det.box.width / image.cols;
        float h = (float)det.box.height / image.rows;

        // Valid tọa độ
        if (x < 0) x = 0; if (y < 0) y = 0;
        if (x + w > 1.0f) w = 1.0f - x;
        if (y + h > 1.0f) h = 1.0f - y;

        objectMetadata->setBoundingBox(Rect(x, y, w, h));
        metadataPacket->addItem(objectMetadata.get());
    }

    pushMetadataPacket(metadataPacket.releasePtr());
    return true;
}

void DeviceAgent::doSetNeededMetadataTypes(nx::sdk::Result<void>*, const nx::sdk::analytics::IMetadataTypes*) {}

} // namespace face_detection_plugin