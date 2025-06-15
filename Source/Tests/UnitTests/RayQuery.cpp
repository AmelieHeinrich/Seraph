//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 12:00:05
//

#include "Test.h"
#include "Base.h"

#include <glm/gtc/matrix_transform.hpp>

class RayQueryTest : public RHIBaseTest
{
public:
    RayQueryTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        uint indices[] = {
            0, 1, 2
        };

        float3 vertices[] = {
            float3{  0.0f, -0.5f, 1.0f },
            float3{ -0.5f,  0.5f, 1.0f },
            float3{  0.5f,  0.5f, 1.0f }
        };

        mVertexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(vertices), sizeof(float3), RHIBufferUsage::kVertex));
        mIndexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(indices), sizeof(uint), RHIBufferUsage::kIndex));
        mBLAS = mStarters.Device->CreateBLAS(RHIBLASDesc(mVertexBuffer, mIndexBuffer));
        mTLAS = mStarters.Device->CreateTLAS();
        mInstanceBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(TLASInstance), sizeof(TLASInstance), RHIBufferUsage::kConstant));
        mView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mStarters.RenderTexture, RHITextureViewType::kShaderWrite));

        TLASInstance instance = {};
        instance.Transform = glm::identity<glm::mat3x4>();
        instance.AccelerationStructureReference = mBLAS->GetAddress();
        instance.Mask = 1;
        instance.InstanceCustomIndex = 0;
        instance.Flags = TLAS_INSTANCE_OPAQUE;

        void* ptr = mInstanceBuffer->Map();
        memcpy(ptr, &instance, sizeof(instance));
        mInstanceBuffer->Unmap();

        CompiledShader shader = ShaderCompiler::Compile("Tests/RayQuery.slang", { "CSMain" });

        RHIComputePipelineDesc desc = {};
        desc.ComputeBytecode = shader.Entries["CSMain"];
        desc.PushConstantSize = sizeof(uint) * 4;
        mComputePipeline = mStarters.Device->CreateComputePipeline(desc);

        Uploader::EnqueueBufferUpload(vertices, sizeof(vertices), mVertexBuffer);
        Uploader::EnqueueBufferUpload(indices, sizeof(indices), mIndexBuffer);
        Uploader::EnqueueBLASBuild(mBLAS);
        Uploader::EnqueueTLASBuild(mTLAS, mInstanceBuffer, 1);
        Uploader::Flush();
    }

    ~RayQueryTest()
    {
        delete mTLAS;
        delete mBLAS;
        delete mIndexBuffer;
        delete mVertexBuffer;
        delete mComputePipeline;
        delete mView;
    }

    void Execute() override
    {
        RHITextureBarrier beginRenderBarrier(mStarters.RenderTexture);
        beginRenderBarrier.SourceStage = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage = RHIPipelineStage::kComputeShader;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess = RHIResourceAccess::kShaderWrite;
        beginRenderBarrier.NewLayout = RHIResourceLayout::kGeneral;

        RHITextureBarrier endRenderBarrier(mStarters.RenderTexture);
        endRenderBarrier.SourceStage = RHIPipelineStage::kComputeShader;
        endRenderBarrier.DestStage = RHIPipelineStage::kCopy;
        endRenderBarrier.SourceAccess = RHIResourceAccess::kShaderWrite;
        endRenderBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
        endRenderBarrier.NewLayout = RHIResourceLayout::kTransferSrc;

        struct PushConstants {
            BindlessHandle handle;
            BindlessHandle tlas;
            uint width;
            uint height;
        } handle = {
            mView->GetBindlessHandle(),
            mTLAS->GetBindlessHandle(),
            TEST_WIDTH,
            TEST_HEIGHT
        };

        mCommandList->Barrier(beginRenderBarrier);
        mCommandList->SetComputePipeline(mComputePipeline);
        mCommandList->SetComputeConstants(mComputePipeline, &handle, sizeof(handle));
        mCommandList->Dispatch((TEST_WIDTH + 7) / 8, (TEST_HEIGHT + 7) / 8, 1);
        mCommandList->Barrier(endRenderBarrier);
    }
private:
    IRHITextureView* mView;
    
    IRHIBuffer* mVertexBuffer;
    IRHIBuffer* mIndexBuffer;
    IRHIBLAS* mBLAS;
    
    IRHITLAS* mTLAS;
    IRHIBuffer* mInstanceBuffer;
    
    IRHIComputePipeline* mComputePipeline;
};

DEFINE_RHI_TEST(RayQuery) {
    RayQueryTest test(backend);
    return test.Run();
}
