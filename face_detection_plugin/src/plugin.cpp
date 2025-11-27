#include "plugin.h"
#include "engine.h"
#include <string> // Bổ sung thư viện string

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Plugin::doObtainEngine()
{
    return new Engine(this);
}

std::string Plugin::manifestString() const
{
    // --- KHẮC PHỤC LỖI PARSING BẰNG CÁCH GHÉP CHUỖI C++ ---
    // Phương pháp này loại bỏ mọi lỗi do ký tự ẩn trong R"json(...)"
    std::string manifest = "{";
    manifest += "\"id\": \"face.detection.plugin\",";
    manifest += "\"name\": \"Face Detection Plugin\",";
    manifest += "\"description\": \"Face Detection using YOLOv8 ONNX\",";
    manifest += "\"version\": \"1.0.0\",";
    manifest += "\"vendor\": \"Custom\",";
    manifest += "\"typeLibrary\": {";
    manifest += "\"objectTypes\": [";
    manifest += "{";
    manifest += "\"id\": \"face.detection.object\","; // ID PHẢI KHỚP
    manifest += "\"name\": \"Face\",";
    manifest += "\"attributes\": []";
    manifest += "}";
    manifest += "]";
    manifest += "}";
    manifest += "}";
    return manifest;
}

} // namespace face_detection_plugin

// Entry point
extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin()
{
    return new face_detection_plugin::Plugin();
}