#pragma once

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/string.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/ptr.h>
#include <memory>
#include <vector>
#include "FaceDetector.h"

namespace face_detection_plugin {

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent
{
public:
    DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo);
    virtual ~DeviceAgent() override;

protected:
    virtual std::string manifestString() const override;

    // SỬA: Dùng hàm này để nhận video đã giải mã (YUV)
    virtual bool pushUncompressedVideoFrame(const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame) override;

    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;

private:
    nx::sdk::Uuid trackIdByIndex(int index);
    
    std::unique_ptr<FaceDetector> m_faceDetector;
    std::string m_modelPath;
    std::vector<nx::sdk::Uuid> m_trackIds;
};

} // namespace face_detection_plugin