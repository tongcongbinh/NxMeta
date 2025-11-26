#pragma once

#include <nx/sdk/analytics/helpers/engine.h>
// ĐÃ XÓA: #include <nx/sdk/analytics/helpers/plugin_manifest.h> (Gây lỗi)

namespace face_detection_plugin {

class Engine: public nx::sdk::analytics::Engine
{
public:
    Engine();
    virtual ~Engine() override = default;

protected:
    virtual std::string manifestString() const override;
    virtual void doObtainDeviceAgent(
        nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult,
        const nx::sdk::IDeviceInfo* deviceInfo) override;
};

} // namespace face_detection_plugin