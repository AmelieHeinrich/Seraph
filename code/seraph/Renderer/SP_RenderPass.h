//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 19:42:38
//

#pragma once

#include <KernelGPU/KGPU_Device.h>
#include <Graphics/Gfx_ResourceManager.h>
#include <Graphics/Gfx_Skybox.h>

#include "SP_RenderWorld.h"

namespace SP
{
    struct CameraData
    {
        KGPU::float4x4 View;
        KGPU::float4x4 PrevView;
        KGPU::float4x4 Proj;
        KGPU::float4x4 PrevProj;
        KGPU::float4x4 ViewProj;
        KGPU::float4x4 PrevViewProj;

        KGPU::float4x4 InvView;
        KGPU::float4x4 PrevInvView;
        KGPU::float4x4 InvProj;
        KGPU::float4x4 PrevInvProj;
        KGPU::float4x4 InvViewProj;
        KGPU::float4x4 PrevInvViewProj;

        KGPU::float4 Position;
    };

    struct RenderPassBegin
    {
        int Width;
        int Height;
        uint FrameIndex;
        uint FrameCount;
        KGPU::ITexture* SwapTexture;
        KGPU::ITextureView* SwapView;
        KGPU::ICommandList* CmdList;
        
        RenderWorld* World;
        Gfx::Skybox* Sky;
        CameraData CamData;
    };

    class RenderPass
    {
    public:
        RenderPass() = default;
        virtual ~RenderPass() = default;

        virtual void Render(RenderPassBegin& begin) = 0;
        virtual void UI(RenderPassBegin& begin) {}
    };
}


