//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 07:47:40
//

#pragma once

#include <Core/Types.h>
#include <RHI/Device.h>

constexpr uint MAX_POINT_LIGHTS = 8192;

struct PointLight
{
    glm::vec3 Position;
    float Radius;

    glm::vec3 Color;
    uint ShadowMapSRV;
};

class LightList
{
public:
    LightList(IRHIDevice* device);
    ~LightList();

    void Update();

    Array<PointLight> PointLights;
private:
    IRHIBuffer* mPointLightBuffer;
    IRHIBufferView* mPointLightBufferView;
};
