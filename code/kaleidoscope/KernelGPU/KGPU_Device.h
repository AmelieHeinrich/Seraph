//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 18:58:58
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelOS/KOS_Window.h>

#include "KGPU_Backend.h"
#include "KGPU_Surface.h"
#include "KGPU_Texture.h"
#include "KGPU_TextureView.h"
#include "KGPU_CommandQueue.h"
#include "KGPU_Sync.h"
#include "KGPU_GraphicsPipeline.h"
#include "KGPU_Buffer.h"
#include "KGPU_Sampler.h"
#include "KGPU_ComputePipeline.h"
#include "KGPU_BLAS.h"
#include "KGPU_TLAS.h"
#include "KGPU_BufferView.h"

namespace KGPU
{
    class IDevice
    {
    public:
        static IDevice* Create(bool debug);
        virtual ~IDevice() = default;

        virtual ISurface* CreateSurface(KOS::IWindow* window, ICommandQueue* graphicsQueue) = 0;
        virtual ITexture* CreateTexture(TextureDesc desc) = 0;
        virtual ITextureView* CreateTextureView(TextureViewDesc desc) = 0;
        virtual ICommandQueue* CreateCommandQueue(CommandQueueType type) = 0;
        virtual ISync* CreateSync(ISurface* surface, ICommandQueue* queue) = 0;
        virtual IGraphicsPipeline* CreateGraphicsPipeline(GraphicsPipelineDesc desc) = 0;
        virtual IBuffer* CreateBuffer(BufferDesc desc) = 0;
        virtual ISampler* CreateSampler(SamplerDesc desc) = 0;
        virtual IComputePipeline* CreateComputePipeline(ComputePipelineDesc desc) = 0;
        virtual IBLAS* CreateBLAS(BLASDesc desc) = 0;
        virtual ITLAS* CreateTLAS() = 0;
        virtual IBufferView* CreateBufferView(BufferViewDesc desc) = 0;
        
        virtual Backend GetBackend() = 0;
        virtual bool SupportsRaytracing() { return false; };
        virtual bool SupportsMeshShaders() { return false; };

        virtual TextureFormat GetSurfaceFormat() = 0;
        virtual uint64 GetOptimalRowPitchAlignment() = 0;
        virtual uint64 GetBufferImageGranularity() = 0;
        virtual KGPU::ShaderBytecodeType GetTargetBytecode() = 0;
    };
}
