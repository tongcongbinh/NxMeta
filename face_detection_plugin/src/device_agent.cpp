#include "device_agent.h"

#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_track_best_shot_packet.h>
#include <nx/kit/debug.h>
#include <nx/kit/utils.h>

#if defined(_WIN32)
    #include <windows.h>
#endif

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

// Hàm tiện ích lấy đường dẫn DLL để load model
std::string getPluginDir() {
#if defined(_WIN32)
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR) &getPluginDir, &hm) == 0)
    {
        return "";
    }
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
    m_faceDetector = std::make_unique<FaceDetector>(m_modelPath);
}

DeviceAgent::~DeviceAgent()
{
}

std::string DeviceAgent::manifestString() const
{
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

bool DeviceAgent::pushCompressedVideoFrame(const nx::sdk::analytics::ICompressedVideoPacket* videoPacket)
{
    std::string codec = videoPacket->codec();
    // Chuyển về chữ thường
    std::transform(codec.begin(), codec.end(), codec.begin(), [](unsigned char c){ return std::tolower(c); });
    static int frameCount = 0;
    if (frameCount++ % 100 == 0) {
        NX_PRINT << "[MyFacePlugin] Codec dang nhan: " << codec;
    }
    // Nếu là H.264/H.265/HEVC... -> Bỏ qua ngay lập tức
    if (codec != "mjpeg" && codec != "jpeg") {
        // (Tùy chọn) In log 1 lần để biết server đang nhận cái gì
        static bool logged = false;
        if (!logged) {
            NX_PRINT << "[MyFacePlugin] Bo qua codec khong ho tro: " << codec;
            logged = true;
        }
        return true; 
    }
    const char* data = videoPacket->data();
    int size = videoPacket->dataSize();
    
    std::vector<char> imgData(data, data + size);
    cv::Mat frame = cv::imdecode(imgData, cv::IMREAD_COLOR);

    if (frame.empty()) return true;

    // Gọi AI
    std::vector<DetectionResult> results = m_faceDetector->detect(frame);

    if (results.empty()) return true;

    auto metadataPacket = makePtr<ObjectMetadataPacket>();
    metadataPacket->setTimestampUs(videoPacket->timestampUs());
    metadataPacket->setDurationUs(0);

    for (const auto& det : results) {
        auto objectMetadata = makePtr<ObjectMetadata>();
        
        objectMetadata->setTypeId("face.detection.object");
        objectMetadata->setConfidence(det.confidence);

        // Tính tọa độ Relative
        float x = (float)det.box.x / frame.cols;
        float y = (float)det.box.y / frame.rows;
        float w = (float)det.box.width / frame.cols;
        float h = (float)det.box.height / frame.rows;

        // Clip tọa độ (Sửa 1.0 -> 1.0f để sửa warning C4244)
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