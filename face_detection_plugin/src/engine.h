#pragma once

#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/helpers/plugin.h>

namespace face_detection_plugin {

class Engine: public nx::sdk::analytics::Engine
{
public:
    // SỬA: Constructor nhận con trỏ Plugin
    explicit Engine(nx::sdk::analytics::Plugin* plugin);
    virtual ~Engine() override;

protected:
    virtual std::string manifestString() const override;
    virtual void doObtainDeviceAgent(nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult, const nx::sdk::IDeviceInfo* deviceInfo) override;

private:
    nx::sdk::analytics::Plugin* m_plugin;
};

} // namespace face_detection_plugin