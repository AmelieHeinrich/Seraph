//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 07:47:40
//

#pragma once

#include <Core/Types.h>
#include <RHI/Device.h>

constexpr uint MAX_POINT_LIGHTS = 16384;

struct PointLight
{
    glm::vec3 Position;
    float Radius;

    glm::vec3 Color;
    float Intensity;
};

class LightList
{
public:
    LightList(IRHIDevice* device);
    ~LightList();

    void Update(uint frameIndex);
    void AddPointLight(glm::vec3 pos = glm::vec3(0.0f), float radius = 1.0f, glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f)
    {
        PointLight light;
        light.Position = pos;
        light.Radius = radius;
        light.Color = color;
        light.Intensity = intensity;
        PointLights.push_back(light);
    }

    Array<PointLight> PointLights;

    IRHIBuffer* GetPointLightBuffer(uint frameIndex) { return mPointLightBuffer[frameIndex]; }
    IRHIBufferView* GetPointLightBufferView(uint frameIndex) { return mPointLightBufferView[frameIndex]; }
private:
    IRHIBuffer* mPointLightBuffer[FRAMES_IN_FLIGHT];
    IRHIBufferView* mPointLightBufferView[FRAMES_IN_FLIGHT];
};
