//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:22:34
//

#pragma once

#include "Core/Window.h"

#include "RHI/Device.h"
#include "RHI/ShaderCompiler.h"
#include "RHI/Uploader.h"

class Application
{
public:
    Application();
    ~Application();

    void Run();
private:
    RHIBackend mBackend;

    SharedPtr<Window> mWindow;

    IRHIDevice* mDevice;
    IRHISurface* mSurface;
    IRHICommandQueue* mGraphicsQueue;
    StaticArray<IRHICommandBuffer*, FRAMES_IN_FLIGHT> mCommandBuffers;
    IRHIF2FSync* mF2FSync;
    
    IRHIGraphicsPipeline* mPipeline;
    IRHIBuffer* mVertexBuffer;
    IRHIBuffer* mIndexBuffer;
    IRHIBuffer* mTestCBV;
    IRHIBufferView* mCBV;
    IRHITexture* mTexture;
    IRHITextureView* mTextureSRV;
    IRHISampler* mSampler;
};
