#include "plugin.h"
#include "engine.h"
#include <nx/kit/json.h>

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Plugin::doObtainEngine()
{
    return new Engine();
}

std::string Plugin::manifestString() const
{
    return R"json({
        "id": "face.detection.plugin",
        "name": "Face Detection Plugin",
        "description": "Plugin nhận diện khuôn mặt YOLO ONNX",
        "version": "1.0.0",
        "vendor": "My Company"
    })json";
}

} // namespace face_detection_plugin

extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin()
{
    return new face_detection_plugin::Plugin();
}