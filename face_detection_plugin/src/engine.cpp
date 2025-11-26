#include "engine.h"
#include "device_agent.h"

namespace face_detection_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

// THÊM: Gọi constructor lớp cha. 
// enableOutput = false vì Engine này không trực tiếp đẩy metadata (DeviceAgent làm việc đó)
Engine::Engine(): nx::sdk::analytics::Engine(/*enableOutput*/ false)
{
}

std::string Engine::manifestString() const
{
    return "{}";
}

void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
{
    // Tạo mới DeviceAgent cho Camera được yêu cầu
    *outResult = new DeviceAgent(deviceInfo);
}

} // namespace face_detection_plugin