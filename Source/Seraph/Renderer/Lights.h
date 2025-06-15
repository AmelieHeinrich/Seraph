//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 07:47:40
//

#pragma once

#include <Core/Types.h>
#include <RHI/Device.h>

constexpr uint MAX_POINT_LIGHTS = 16384;
constexpr uint MAX_SPOT_LIGHTS = 16384;

struct PointLight
{
    float3 Position;
    float Radius;

    float3 Color;
    float Intensity;
};

struct SpotLight
{
    float3 Position;
    float Size;

    float3 Forward;
    float Angle;

    float3 Color;
    float Intensity;
};

class LightList
{
public:
    LightList(IRHIDevice* device);
    ~LightList();

    void Update(uint frameIndex);
    
    void AddPointLight(float3 pos = float3(0.0f), float radius = 1.0f, float3 color = float3(1.0f), float intensity = 1.0f)
    {
        PointLight light;
        light.Position = pos;
        light.Radius = radius;
        light.Color = color;
        light.Intensity = intensity;
        PointLights.push_back(light);
    }

    void AddSpotLight(float3 pos = float3(0.0f), float size = 3.0f, float3 forward = float3(1.0f, 0.0f, 0.0f), float angle = 45.0f, float3 color = float3(1.0f), float intensity = 1.0f)
    {
        SpotLight light;
        light.Position = pos;
        light.Forward = forward;
        light.Angle = angle;
        light.Size = size;
        light.Color = color;
        light.Intensity = intensity;
        SpotLights.push_back(light);
    }

    Array<PointLight> PointLights;
    Array<SpotLight> SpotLights;

    IRHIBuffer* GetPointLightBuffer(uint frameIndex) { return mPointLightBuffer[frameIndex]; }
    IRHIBufferView* GetPointLightBufferView(uint frameIndex) { return mPointLightBufferView[frameIndex]; }

    IRHIBuffer* GetSpotLightBuffer(uint frameIndex) { return mSpotLightBuffer[frameIndex]; }
    IRHIBufferView* GetSpotLightBufferView(uint frameIndex) { return mSpotLightBufferView[frameIndex]; }
private:
    IRHIBuffer* mPointLightBuffer[FRAMES_IN_FLIGHT];
    IRHIBufferView* mPointLightBufferView[FRAMES_IN_FLIGHT];

    IRHIBuffer* mSpotLightBuffer[FRAMES_IN_FLIGHT];
    IRHIBufferView* mSpotLightBufferView[FRAMES_IN_FLIGHT];
};
