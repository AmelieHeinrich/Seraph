//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:22:34
//

#pragma once

#include "Core/Window.h"

#include "RHI/Device.h"
#include "RHI/ShaderCompiler.h"
#include "RHI/Uploader.h"

struct ApplicationSpecs
{
    RHIBackend Backend;
    int WindowWidth;
    int WindowHeight;
};

class Application
{
public:
    Application(const ApplicationSpecs& specs);
    ~Application();

    void Run();
private:
    SharedPtr<Window> mWindow;

    IRHIDevice* mDevice;
    IRHICommandQueue* mGraphicsQueue;
    IRHISurface* mSurface;
    StaticArray<IRHICommandBuffer*, FRAMES_IN_FLIGHT> mCommandBuffers;
    IRHIF2FSync* mF2FSync;
    IRHIImGuiContext* mImGuiContext;
    
    IRHIGraphicsPipeline* mPipeline;
    IRHIBuffer* mVertexBuffer;
    IRHIBuffer* mIndexBuffer;
    IRHIBuffer* mTestCBV;
    IRHIBufferView* mCBV;
    IRHITexture* mTexture;
    IRHITextureView* mTextureSRV;
    IRHISampler* mSampler;

    Array<TLASInstance> mInstances;
    IRHIBLAS* mBLAS;
    IRHITLAS* mTLAS;
    IRHIBuffer* mInstanceBuffer;
};
