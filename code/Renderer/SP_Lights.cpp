//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 20:36:22
//

#include "SP_Lights.h"

#include <Graphics/Gfx_Manager.h>

namespace SP
{
    LightList::LightList()
    {
        KGPU::IDevice* device = Gfx::Manager::GetDevice();

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            mPointLightBuffer[i] = device->CreateBuffer(KGPU::BufferDesc(sizeof(PointLight) * MAX_POINT_LIGHTS, sizeof(PointLight), KGPU::BufferUsage::kStaging | KGPU::BufferUsage::kShaderRead));
            mPointLightBuffer[i]->SetName("Point Light Buffer");
            mPointLightBufferView[i] = device->CreateBufferView(KGPU::BufferViewDesc(mPointLightBuffer[i], KGPU::BufferViewType::kStructured));

            mSpotLightBuffer[i] = device->CreateBuffer(KGPU::BufferDesc(sizeof(PointLight) * MAX_POINT_LIGHTS, sizeof(PointLight), KGPU::BufferUsage::kStaging | KGPU::BufferUsage::kShaderRead));
            mSpotLightBuffer[i]->SetName("Spot Light Buffer");
            mSpotLightBufferView[i] = device->CreateBufferView(KGPU::BufferViewDesc(mSpotLightBuffer[i], KGPU::BufferViewType::kStructured));

            mSunBuffer[i] = device->CreateBuffer(KGPU::BufferDesc(sizeof(DirectionalLight), sizeof(DirectionalLight), KGPU::BufferUsage::kStaging | KGPU::BufferUsage::kShaderRead));
            mSunBuffer[i]->SetName("Sun Buffer");
            mSunBufferView[i] = device->CreateBufferView(KGPU::BufferViewDesc(mSunBuffer[i], KGPU::BufferViewType::kStructured));
        }
    }

    LightList::~LightList()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            KC_DELETE(mSunBuffer[i]);
            KC_DELETE(mSunBufferView[i]);

            KC_DELETE(mSpotLightBuffer[i]);
            KC_DELETE(mSpotLightBufferView[i]);

            KC_DELETE(mPointLightBuffer[i]);
            KC_DELETE(mPointLightBufferView[i]);
        }
    }

    void LightList::Update(uint frameIndex)
    {
        void* mem = mPointLightBuffer[frameIndex]->Map();
        memcpy(mem, PointLights.data(), PointLights.size() * sizeof(PointLight));
        mPointLightBuffer[frameIndex]->Unmap();

        mem = mSpotLightBuffer[frameIndex]->Map();
        memcpy(mem, SpotLights.data(), SpotLights.size() * sizeof(SpotLight));
        mSpotLightBuffer[frameIndex]->Unmap();

        mem = mSunBuffer[frameIndex]->Map();
        memcpy(mem, &Sun, sizeof(Sun));
        mSunBuffer[frameIndex]->Unmap();
    }
}
