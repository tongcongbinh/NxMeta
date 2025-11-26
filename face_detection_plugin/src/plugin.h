#pragma once

// SỬA DÒNG NÀY: Dùng helpers/plugin.h thay vì i_plugin.h
#include <nx/sdk/analytics/helpers/plugin.h> 

namespace face_detection_plugin {

// Class Plugin kế thừa từ nx::sdk::analytics::Plugin (Helper class)
class Plugin: public nx::sdk::analytics::Plugin
{
public:
    virtual ~Plugin() override = default;

protected:
    virtual nx::sdk::Result<nx::sdk::analytics::IEngine*> doObtainEngine() override;
    virtual std::string manifestString() const override;
};

} // namespace face_detection_plugin

// Hàm này cần NX_PLUGIN_API (đã được fix trong CMake)
extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin();