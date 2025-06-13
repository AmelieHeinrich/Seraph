//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 07:59:36
//

#include "Lights.h"

LightList::LightList(IRHIDevice* device)
{
    mPointLightBuffer = device->CreateBuffer(RHIBufferDesc(sizeof(PointLight) * MAX_POINT_LIGHTS, sizeof(PointLight), RHIBufferUsage::kStaging | RHIBufferUsage::kShaderRead));
    mPointLightBufferView = device->CreateBufferView(RHIBufferViewDesc(mPointLightBuffer, RHIBufferViewType::kStructured));
}

LightList::~LightList()
{
    delete mPointLightBuffer;
    delete mPointLightBufferView;
}

void LightList::Update()
{
    void* mem = mPointLightBuffer->Map();
    memcpy(mem, PointLights.data(), PointLights.size() * sizeof(PointLight));
    mPointLightBuffer->Unmap();
}
