#include "plugin.h"
#include "engine.h"

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Plugin::doObtainEngine()
{
    return new Engine(this);
}

std::string Plugin::manifestString() const
{
    // THÊM: typeLibrary vào đây
    return R"json({
        "id": "face.detection.plugin",
        "name": "Face Detection Plugin",
        "description": "Face Detection using YOLOv8 ONNX",
        "version": "1.0.0",
        "vendor": "Custom",
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

} // namespace face_detection_plugin

// Giữ nguyên đoạn extern "C" ở cuối file
extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin()
{
    return new face_detection_plugin::Plugin();
}