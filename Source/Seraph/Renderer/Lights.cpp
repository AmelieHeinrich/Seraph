//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 07:59:36
//

#include "Lights.h"

LightList::LightList(IRHIDevice* device)
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mPointLightBuffer[i] = device->CreateBuffer(RHIBufferDesc(sizeof(PointLight) * MAX_POINT_LIGHTS, sizeof(PointLight), RHIBufferUsage::kStaging | RHIBufferUsage::kShaderRead));
        mPointLightBuffer[i]->SetName("Point Light Buffer");
        mPointLightBufferView[i] = device->CreateBufferView(RHIBufferViewDesc(mPointLightBuffer[i], RHIBufferViewType::kStructured));

        mSpotLightBuffer[i] = device->CreateBuffer(RHIBufferDesc(sizeof(PointLight) * MAX_POINT_LIGHTS, sizeof(PointLight), RHIBufferUsage::kStaging | RHIBufferUsage::kShaderRead));
        mSpotLightBuffer[i]->SetName("Spot Light Buffer");
        mSpotLightBufferView[i] = device->CreateBufferView(RHIBufferViewDesc(mSpotLightBuffer[i], RHIBufferViewType::kStructured));
    }
}

LightList::~LightList()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mSpotLightBuffer[i];
        delete mSpotLightBufferView[i];

        delete mPointLightBuffer[i];
        delete mPointLightBufferView[i];
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
}
