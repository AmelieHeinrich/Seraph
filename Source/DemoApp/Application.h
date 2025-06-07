//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:22:34
//

#pragma once

#include <Seraph/Seraph.h>

#include "Camera.h"

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
    ApplicationSpecs mSpecs;
    Camera mCamera;

    SharedPtr<Window> mWindow;
    Model* mModel;

    IRHIDevice* mDevice;
    IRHICommandQueue* mGraphicsQueue;
    IRHISurface* mSurface;
    StaticArray<IRHICommandList*, FRAMES_IN_FLIGHT> mCommandBuffers;
    IRHIF2FSync* mF2FSync;
    IRHIImGuiContext* mImGuiContext;
    
    IRHIGraphicsPipeline* mPipeline;
    IRHIBuffer* mTestCBV;
    IRHIBufferView* mCBV;
    IRHISampler* mSampler;

    IRHITexture* mDepthBuffer;
    IRHITextureView* mDepthView;

    IRHIBuffer* mScreenshotBuffer;
    ImageData mScreenshotData;
    bool mScreenshotted = false;
};
