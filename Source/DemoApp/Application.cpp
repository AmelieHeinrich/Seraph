//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

#include <imgui/imgui.h>
#include <chrono>

static const float VERTICES[] = {
     0.5f,  0.5f, 0.0f, 1.0f, -1.0f,
     0.5f, -0.5f, 0.0f, 1.0f,  0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f,  0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, -1.0f
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

    mTestCBV = mDevice->CreateBuffer(RHIBufferDesc(256, 0, RHIBufferUsage::kConstant));
    mCBV = mDevice->CreateBufferView(RHIBufferViewDesc(mTestCBV, RHIBufferViewType::kConstant));
    mSampler = mDevice->CreateSampler(RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kLinear, false));

    {
        RHITextureDesc desc = {};
        desc.Width = mSpecs.WindowWidth;
        desc.Height = mSpecs.WindowHeight;
        desc.Format = RHITextureFormat::kD32_FLOAT;
        desc.Usage = RHITextureUsage::kDepthTarget;

        mDepthBuffer = mDevice->CreateTexture(desc);
        mDepthView = mDevice->CreateTextureView(RHITextureViewDesc(mDepthBuffer, RHITextureViewType::kDepthTarget));
    }

    RHIGraphicsPipelineDesc desc = {};
    desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    desc.ReflectInputLayout = true;
    desc.PushConstantSize = sizeof(BindlessHandle) * 4 + sizeof(glm::mat4) * 2;
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

    mModel = new Model(mDevice, "Data/Models/Sponza/Sponza.gltf");
    Uploader::Flush();
}

Application::~Application()
{
    Uploader::Shutdown();

    delete mModel;

    delete mScreenshotBuffer;

    delete mTestCBV;
    delete mCBV;
    delete mSampler;
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
    auto lastFrame = std::chrono::high_resolution_clock::now();

    while (mWindow->IsOpen()) {
        auto time = std::chrono::high_resolution_clock::now();
        float delta = (std::chrono::duration<float>(lastFrame - time).count());
        lastFrame = time;

        mWindow->PollEvents();
        mCamera.Begin();

        uint frameIndex = mF2FSync->BeginSynchronize();
        IRHICommandBuffer* commandBuffer = mCommandBuffers[frameIndex];
        IRHITexture* swapchainTexture = mSurface->GetTexture(frameIndex);
        IRHITextureView* swapchainTextureView = mSurface->GetTextureView(frameIndex);

        commandBuffer->Reset();
        commandBuffer->Begin();
        
        RHITextureBarrier beginRenderBarrier(swapchainTexture, RHIResourceAccess::kNone, RHIResourceAccess::kColorAttachmentWrite, RHIPipelineStage::kBottomOfPipe, RHIPipelineStage::kColorAttachmentOutput, RHIResourceLayout::kColorAttachment);
        RHITextureBarrier beginDepthBarrier(mDepthBuffer, RHIResourceAccess::kNone, RHIResourceAccess::kDepthStencilWrite, RHIPipelineStage::kBottomOfPipe, RHIPipelineStage::kEarlyFragmentTests, RHIResourceLayout::kDepthStencilWrite);
        RHITextureBarrier endRenderBarrier(swapchainTexture, RHIResourceAccess::kColorAttachmentWrite, RHIResourceAccess::kNone, RHIPipelineStage::kColorAttachmentOutput, RHIPipelineStage::kBottomOfPipe, RHIResourceLayout::kPresent);
        RHITextureBarrier endDepthBarrier(mDepthBuffer, RHIResourceAccess::kDepthStencilWrite, RHIResourceAccess::kNone, RHIPipelineStage::kEarlyFragmentTests, RHIPipelineStage::kBottomOfPipe, RHIResourceLayout::kGeneral);
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

        commandBuffer->PushMarker("Meow");
        commandBuffer->BarrierGroup(beginGroup);
        
        commandBuffer->BeginRendering(renderBegin);

        commandBuffer->SetGraphicsPipeline(mPipeline);
        commandBuffer->SetViewport(renderBegin.Width, renderBegin.Height, 0, 0);
        for (auto& node : mModel->GetNodes()) {
            for (auto& primitive : node.Primitives) {
                ModelMaterial material = mModel->GetMaterials()[primitive.MaterialIndex];

                struct PushConstant {
                    BindlessHandle Texture;
                    BindlessHandle Sampler;
                    BindlessHandle CBV;
                    BindlessHandle Pad;

                    glm::mat4 View;
                    glm::mat4 Projection;
                } constant = {
                    material.TextureRead->GetBindlessHandle(),
                    mSampler->GetBindlessHandle(),
                    mCBV->GetBindlessHandle(),
                    {},
                
                    mCamera.View(),
                    mCamera.Projection()
                };

                commandBuffer->SetVertexBuffer(primitive.VertexBuffer);
                commandBuffer->SetIndexBuffer(primitive.IndexBuffer);
                commandBuffer->SetGraphicsConstants(mPipeline, &constant, sizeof(constant));
                commandBuffer->DrawIndexed(primitive.IndexCount, 1, 0, 0, 0);
            }
        }

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

            RHITextureBarrier beginTextureBarrier(swapchainTexture, RHIResourceAccess::kNone, RHIResourceAccess::kMemoryRead, RHIPipelineStage::kBottomOfPipe, RHIPipelineStage::kCopy, RHIResourceLayout::kTransferSrc);
            RHIBufferBarrier beginBufferBarrier(mScreenshotBuffer, RHIResourceAccess::kMemoryRead, RHIResourceAccess::kMemoryWrite, RHIPipelineStage::kAllCommands, RHIPipelineStage::kCopy);
            RHIBarrierGroup beginGroup = {};
            beginGroup.BufferBarriers = { beginBufferBarrier };
            beginGroup.TextureBarriers = { beginTextureBarrier };

            RHITextureBarrier endTextureBarrier(swapchainTexture, RHIResourceAccess::kMemoryRead, RHIResourceAccess::kNone, RHIPipelineStage::kCopy, RHIPipelineStage::kBottomOfPipe, RHIResourceLayout::kPresent);
            RHIBufferBarrier endBufferBarrier(mScreenshotBuffer, RHIResourceAccess::kMemoryWrite, RHIResourceAccess::kMemoryRead, RHIPipelineStage::kCopy, RHIPipelineStage::kAllCommands);
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

        mCamera.Update(delta, 16, 9);
    }
}
