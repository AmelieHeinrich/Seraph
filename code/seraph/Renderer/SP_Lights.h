//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 20:33:13
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelGPU/KGPU_Device.h>

using namespace KGPU;

namespace SP
{   
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

    struct DirectionalLight
    {
        float3 Direction;
        float Intensity;

        float3 Color;
        float Pad;
    };

    class LightList
    {
    public:
        LightList();
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

        KC::Array<PointLight> PointLights;
        KC::Array<SpotLight> SpotLights;
        DirectionalLight Sun;

        KGPU::IBuffer* GetPointLightBuffer(uint frameIndex) { return mPointLightBuffer[frameIndex]; }
        KGPU::IBufferView* GetPointLightBufferView(uint frameIndex) { return mPointLightBufferView[frameIndex]; }

        KGPU::IBuffer* GetSpotLightBuffer(uint frameIndex) { return mSpotLightBuffer[frameIndex]; }
        KGPU::IBufferView* GetSpotLightBufferView(uint frameIndex) { return mSpotLightBufferView[frameIndex]; }

        KGPU::IBuffer* GetSunBuffer(uint frameIndex) { return mSunBuffer[frameIndex]; }
        KGPU::IBufferView* GetSunBufferView(uint frameIndex) { return mSunBufferView[frameIndex]; }
    private:
        KGPU::IBuffer* mPointLightBuffer[FRAMES_IN_FLIGHT];
        KGPU::IBufferView* mPointLightBufferView[FRAMES_IN_FLIGHT];

        KGPU::IBuffer* mSpotLightBuffer[FRAMES_IN_FLIGHT];
        KGPU::IBufferView* mSpotLightBufferView[FRAMES_IN_FLIGHT];

        KGPU::IBuffer* mSunBuffer[FRAMES_IN_FLIGHT];
        KGPU::IBufferView* mSunBufferView[FRAMES_IN_FLIGHT];
    };
}
