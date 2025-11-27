#include "engine.h"
#include "device_agent.h"

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Engine::Engine(Plugin* plugin):
    nx::sdk::analytics::Engine(false, plugin->instanceId()), // false để tắt log debug thừa
    m_plugin(plugin)
{
}

Engine::~Engine() {}

void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
{
    *outResult = new DeviceAgent(deviceInfo);
}

std::string Engine::manifestString() const
{
    // CHỈ CẤU HÌNH VIDEO STREAM
    return R"json({
        "capabilities": "needUncompressedVideoFrames_yuv420",
        "streamTypeFilter": "uncompressedVideo"
    })json";
}

} // namespace face_detection_plugin