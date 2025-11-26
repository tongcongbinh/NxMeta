#include "engine.h"
#include "device_agent.h"

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Engine::Engine(Plugin* plugin):
    // enableOutput=true để hiện log debug
    nx::sdk::analytics::Engine(true, plugin->instanceId()),
    m_plugin(plugin)
{
}

Engine::~Engine()
{
}

void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
{
    *outResult = new DeviceAgent(deviceInfo);
}

std::string Engine::manifestString() const
{
    // QUAN TRỌNG: Yêu cầu server giải mã sang YUV420
    return R"json({
        "capabilities": "needUncompressedVideoFrames_yuv420",
        "streamTypeFilter": "uncompressedVideo"
    })json";
}

} // namespace face_detection_plugin