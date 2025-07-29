//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-29 19:16:27
//

#include "KGPUT_Base.h"

#include <Graphics/Gfx_ShaderManager.h>

namespace KGPUT
{
    BaseTest::BaseTest()
    {
        Data = ITest::CreateData();
    }

    BaseTest::~BaseTest()
    {
        ITest::DeleteData(Data);
    }

    KC::Array<uint8> BaseTest::Run()
    {
        Begin();
        Execute();
        Flush();
        Readback();

        Data.CommandQueue->Wait();
        Cleanup();
        return std::move(Data.ScreenshotData);
    }

    void BaseTest::Begin()
    {
        mCommandList = Data.CommandQueue->CreateCommandList(true);
        mCommandList->Begin();
    }

    void BaseTest::Flush()
    {
        mCommandList->End();
        Data.CommandQueue->CommitCommandList(mCommandList);
        KC_DELETE(mCommandList);
    }

    void BaseTest::Readback()
    {
        Begin();

        KGPU::BufferBarrier beginBufferBarrier(Data.ScreenshotBuffer);
        beginBufferBarrier.SourceAccess = KGPU::ResourceAccess::kMemoryRead;
        beginBufferBarrier.DestAccess = KGPU::ResourceAccess::kMemoryWrite;
        beginBufferBarrier.SourceStage = KGPU::PipelineStage::kAllCommands;
        beginBufferBarrier.DestStage = KGPU::PipelineStage::kCopy;
        
        KGPU::BufferBarrier endBufferBarrier(Data.ScreenshotBuffer);
        endBufferBarrier.SourceAccess = KGPU::ResourceAccess::kMemoryWrite;
        endBufferBarrier.DestAccess = KGPU::ResourceAccess::kMemoryRead;
        endBufferBarrier.SourceStage = KGPU::PipelineStage::kCopy;
        endBufferBarrier.DestStage = KGPU::PipelineStage::kAllCommands;
        
        mCommandList->Barrier(beginBufferBarrier);
        mCommandList->CopyTextureToBuffer(Data.ScreenshotBuffer, Data.RenderTexture);
        mCommandList->Barrier(endBufferBarrier);

        Flush();

        Data.ScreenshotData.resize(Data.ScreenshotBuffer->GetDesc().Size);
        void* ptr = Data.ScreenshotBuffer->Map();
        memcpy(Data.ScreenshotData.data(), ptr, Data.ScreenshotBuffer->GetDesc().Size);
        Data.ScreenshotBuffer->Unmap();
    }
}
