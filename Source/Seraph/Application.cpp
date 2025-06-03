//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

#include <imgui/imgui.h>

static const float VERTICES[] = {
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
};

static const uint INDICES[] = {
    0, 1, 3,
    1, 2, 3
};

Application::Application(const ApplicationSpecs& specs)
    : mSpecs(specs)
{
    ShaderCompiler::Initialize(specs.Backend);
    CompiledShader shader = ShaderCompiler::Compile("Textured", { "VSMain", "FSMain" });

    mWindow = SharedPtr<Window>(new Window(specs.WindowWidth, specs.WindowHeight, "Seraph"));
    mDevice = IRHIDevice::CreateDevice(specs.Backend, true);
    mGraphicsQueue = mDevice->CreateCommandQueue(RHICommandQueueType::kGraphics);
    mSurface = mDevice->CreateSurface(mWindow.get(), mGraphicsQueue);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mCommandBuffers[i] = mGraphicsQueue->CreateCommandBuffer(false);
    }
    mF2FSync = mDevice->CreateF2FSync(mSurface, mGraphicsQueue);
    Uploader::Initialize(mDevice, mGraphicsQueue);
    mImGuiContext = mDevice->CreateImGuiContext(mGraphicsQueue, mWindow.get());

    mVertexBuffer = mDevice->CreateBuffer(RHIBufferDesc(sizeof(VERTICES), sizeof(float) * 5, RHIBufferUsage::kVertex));
    mIndexBuffer = mDevice->CreateBuffer(RHIBufferDesc(sizeof(INDICES), sizeof(uint), RHIBufferUsage::kIndex));

    mTestCBV = mDevice->CreateBuffer(RHIBufferDesc(256, 0, RHIBufferUsage::kConstant));
    mCBV = mDevice->CreateBufferView(RHIBufferViewDesc(mTestCBV, RHIBufferViewType::kConstant));
    mSampler = mDevice->CreateSampler(RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kLinear, false));
    
    mBLAS = mDevice->CreateBLAS(RHIBLASDesc(mVertexBuffer, mIndexBuffer));
    mTLAS = mDevice->CreateTLAS();
    mInstanceBuffer = mDevice->CreateBuffer(RHIBufferDesc(sizeof(TLASInstance) * MAX_TLAS_INSTANCES, 0, RHIBufferUsage::kConstant));
    
    TLASInstance triangleInstance = {};
    triangleInstance.Transform[0][0] = 1;
    triangleInstance.Transform[1][2] = 1;
    triangleInstance.Transform[2][2] = 1;
    triangleInstance.AccelerationStructureReference = mBLAS->GetAddress();
    triangleInstance.Flags = TLAS_INSTANCE_OPAQUE;
    mInstances.push_back(triangleInstance);

    void* test = mInstanceBuffer->Map();
    memcpy(test, mInstances.data(), mInstances.size() * sizeof(TLASInstance));
    mInstanceBuffer->Unmap();

    {
        RHITextureDesc desc = {};
        desc.Width = mSpecs.WindowWidth;
        desc.Height = mSpecs.WindowHeight;
        desc.Format = RHITextureFormat::kD32_FLOAT;
        desc.Usage = RHITextureUsage::kDepthTarget;

        mDepthBuffer = mDevice->CreateTexture(desc);
        mDepthView = mDevice->CreateTextureView(RHITextureViewDesc(mDepthBuffer, RHITextureViewType::kDepthTarget));
    }
    
    {
        ImageData data = Image::LoadImageData("Data/Textures/Sweet.jpg");

        RHITextureDesc desc = {};
        desc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
        desc.Width = data.Width;
        desc.Height = data.Height;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.Usage = RHITextureUsage::kShaderResource;
        mTexture = mDevice->CreateTexture(desc);
        mTextureSRV = mDevice->CreateTextureView(RHITextureViewDesc(mTexture, RHITextureViewType::kShaderRead));

        Uploader::EnqueueTextureUploadRaw(data.Pixels.data(), data.Pixels.size(), mTexture);
    }

    Uploader::EnqueueBufferUpload(VERTICES, sizeof(VERTICES), mVertexBuffer);
    Uploader::EnqueueBufferUpload(INDICES, sizeof(INDICES), mIndexBuffer);
    Uploader::EnqueueBLASBuild(mBLAS);
    Uploader::EnqueueTLASBuild(mTLAS, mInstanceBuffer, mInstances.size());
    Uploader::Flush();

    RHIGraphicsPipelineDesc desc = {};
    desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    desc.ReflectInputLayout = true;
    desc.PushConstantSize = sizeof(BindlessHandle) * 3;
    desc.RenderTargetFormats.push_back(mSurface->GetTexture(0)->GetDesc().Format);
    desc.DepthEnabled = true;
    desc.DepthWrite = true;
    desc.DepthFormat = RHITextureFormat::kD32_FLOAT;
    desc.DepthOperation = RHIDepthOperation::kLess;

    mPipeline = mDevice->CreateGraphicsPipeline(desc);

    mScreenshotBuffer = mDevice->CreateBuffer(RHIBufferDesc(specs.WindowWidth * specs.WindowHeight * 4, 0, RHIBufferUsage::kReadback));
    mScreenshotData.Width = mSpecs.WindowWidth;
    mScreenshotData.Height = mSpecs.WindowHeight;
    mScreenshotData.Pixels.resize(mScreenshotData.Width * mScreenshotData.Height * 4);
}

Application::~Application()
{
    Uploader::Shutdown();

    delete mScreenshotBuffer;

    delete mTestCBV;
    delete mCBV;
    
    delete mTextureSRV;
    delete mTexture;
    delete mSampler;
    
    delete mIndexBuffer;
    delete mVertexBuffer;
    delete mPipeline;

    delete mDepthView;
    delete mDepthBuffer;

    delete mImGuiContext;
    delete mF2FSync;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mCommandBuffers[i];
    }
    delete mGraphicsQueue;
    delete mSurface;
    delete mDevice;

    ShaderCompiler::Shutdown();
}

void Application::Run()
{
    int firstFrame = 0;

    while (mWindow->IsOpen()) {
        mWindow->PollEvents();

        uint frameIndex = mF2FSync->BeginSynchronize();
        IRHICommandBuffer* commandBuffer = mCommandBuffers[frameIndex];
        IRHITexture* swapchainTexture = mSurface->GetTexture(frameIndex);
        IRHITextureView* swapchainTextureView = mSurface->GetTextureView(frameIndex);

        commandBuffer->Reset();
        commandBuffer->Begin();
        
        RHITextureBarrier beginRenderBarrier(swapchainTexture);
        beginRenderBarrier.SourceStage  = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage    = RHIPipelineStage::kColorAttachmentOutput;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess   = RHIResourceAccess::kColorAttachmentWrite;
        beginRenderBarrier.NewLayout    = RHIResourceLayout::kColorAttachment;

        RHITextureBarrier endRenderBarrier(swapchainTexture);
        endRenderBarrier.SourceStage   = RHIPipelineStage::kColorAttachmentOutput;
        endRenderBarrier.DestStage     = RHIPipelineStage::kBottomOfPipe;
        endRenderBarrier.SourceAccess  = RHIResourceAccess::kColorAttachmentWrite;
        endRenderBarrier.DestAccess    = RHIResourceAccess::kNone;
        endRenderBarrier.NewLayout     = RHIResourceLayout::kPresent;

        RHITextureBarrier beginDepthBarrier(mDepthBuffer);
        beginDepthBarrier.SourceStage  = RHIPipelineStage::kBottomOfPipe;
        beginDepthBarrier.DestStage    = RHIPipelineStage::kEarlyFragmentTests;
        beginDepthBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginDepthBarrier.DestAccess   = RHIResourceAccess::kDepthStencilWrite;
        beginDepthBarrier.NewLayout    = RHIResourceLayout::kDepthStencilWrite;

        RHITextureBarrier endDepthBarrier(mDepthBuffer);
        endDepthBarrier.SourceStage  = RHIPipelineStage::kEarlyFragmentTests;
        endDepthBarrier.DestStage    = RHIPipelineStage::kBottomOfPipe;
        endDepthBarrier.SourceAccess = RHIResourceAccess::kDepthStencilWrite;
        endDepthBarrier.DestAccess   = RHIResourceAccess::kNone;
        endDepthBarrier.NewLayout    = RHIResourceLayout::kGeneral;

        RHIBarrierGroup beginGroup = {};
        beginGroup.TextureBarriers = { beginRenderBarrier, beginDepthBarrier };

        RHIBarrierGroup endGroup = {};
        endGroup.TextureBarriers = { endRenderBarrier, endDepthBarrier };

        RHIRenderAttachment attachment(swapchainTextureView);
        RHIRenderBegin renderBegin(mSpecs.WindowWidth, mSpecs.WindowHeight, { RHIRenderAttachment(swapchainTextureView) }, RHIRenderAttachment(mDepthView, true));

        float testColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        void* test = mTestCBV->Map();
        SafeMemcpy(test, testColor, sizeof(testColor));
        mTestCBV->Unmap();

        struct PushConstant {
            BindlessHandle Texture;
            BindlessHandle Sampler;
            BindlessHandle CBV;
        } constant = {
            mTextureSRV->GetBindlessHandle(),
            mSampler->GetBindlessHandle(),
            mCBV->GetBindlessHandle()
        };

        commandBuffer->PushMarker("Meow");
        commandBuffer->BarrierGroup(beginGroup);
        
        commandBuffer->BeginRendering(renderBegin);

        commandBuffer->SetGraphicsPipeline(mPipeline);
        commandBuffer->SetViewport(renderBegin.Width, renderBegin.Height, 0, 0);
        commandBuffer->SetVertexBuffer(mVertexBuffer);
        commandBuffer->SetIndexBuffer(mIndexBuffer);
        commandBuffer->SetGraphicsConstants(mPipeline, &constant, sizeof(constant));
        commandBuffer->DrawIndexed(6, 1, 0, 0, 0);
        commandBuffer->BeginImGui();
        ImGui::ShowDemoWindow(nullptr);
        commandBuffer->EndImGui();

        commandBuffer->EndRendering();

        commandBuffer->BarrierGroup(endGroup);
        commandBuffer->PopMarker();
        
        commandBuffer->End();
        mF2FSync->EndSynchronize(mCommandBuffers[frameIndex]);
        mF2FSync->PresentSurface();

        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            auto tempCmd = mGraphicsQueue->CreateCommandBuffer(true);
            tempCmd->Begin();

            RHITextureBarrier beginTextureBarrier(swapchainTexture);
            beginTextureBarrier.SourceStage  = RHIPipelineStage::kBottomOfPipe;
            beginTextureBarrier.DestStage    = RHIPipelineStage::kCopy;
            beginTextureBarrier.SourceAccess = RHIResourceAccess::kNone;
            beginTextureBarrier.DestAccess   = RHIResourceAccess::kMemoryRead;
            beginTextureBarrier.NewLayout    = RHIResourceLayout::kTransferSrc;

            RHIBufferBarrier beginBufferBarrier(mScreenshotBuffer);
            beginBufferBarrier.SourceAccess = RHIResourceAccess::kMemoryRead;
            beginBufferBarrier.DestAccess = RHIResourceAccess::kMemoryWrite;
            beginBufferBarrier.SourceStage = RHIPipelineStage::kAllCommands;
            beginBufferBarrier.DestStage = RHIPipelineStage::kCopy;

            RHIBarrierGroup beginGroup = {};
            beginGroup.BufferBarriers = { beginBufferBarrier };
            beginGroup.TextureBarriers = { beginTextureBarrier };

            RHITextureBarrier endTextureBarrier(swapchainTexture);
            endTextureBarrier.SourceStage   = RHIPipelineStage::kCopy;
            endTextureBarrier.DestStage     = RHIPipelineStage::kBottomOfPipe;
            endTextureBarrier.SourceAccess  = RHIResourceAccess::kMemoryRead;
            endTextureBarrier.DestAccess    = RHIResourceAccess::kNone;
            endTextureBarrier.NewLayout     = RHIResourceLayout::kPresent;

            RHIBufferBarrier endBufferBarrier(mScreenshotBuffer);
            endBufferBarrier.SourceAccess = RHIResourceAccess::kMemoryWrite;
            endBufferBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
            endBufferBarrier.SourceStage = RHIPipelineStage::kCopy;
            endBufferBarrier.DestStage = RHIPipelineStage::kAllCommands;

            RHIBarrierGroup endGroup = {};
            endGroup.BufferBarriers = { endBufferBarrier };
            endGroup.TextureBarriers = { endTextureBarrier };
            
            tempCmd->BarrierGroup(beginGroup);
            tempCmd->CopyTextureToBuffer(mScreenshotBuffer, swapchainTexture);
            tempCmd->BarrierGroup(endGroup);
            tempCmd->End();
            mGraphicsQueue->SubmitAndFlushCommandBuffer(tempCmd);

            uint8* pixels = (uint8*)mScreenshotBuffer->Map();
            if (mSpecs.Backend == RHIBackend::kVulkan) {
                size_t pixelCount = mSpecs.WindowWidth * mSpecs.WindowHeight;
                for (size_t i = 0; i < pixelCount; ++i) {
                    size_t offset = i * 4;
                    std::swap(pixels[offset + 0], pixels[offset + 2]); // B <-> R
                }
            }

            memcpy(mScreenshotData.Pixels.data(), pixels, mSpecs.WindowWidth * mSpecs.WindowHeight * 4);
            mScreenshotBuffer->Unmap();
            Image::WriteImageData(mScreenshotData, "Data/Screenshot.png");

            delete tempCmd;
        }
    }
}
