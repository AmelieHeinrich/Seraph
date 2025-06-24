//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-24 08:15:07
//

#include "Test.h"
#include "Base.h"

#include <glm/gtc/matrix_transform.hpp>

class TLASNonOpaqueInstanceTest : public RHIBaseTest
{
public:
    TLASNonOpaqueInstanceTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        uint indices[] = {
            0, 1, 3,   // First triangle
            2, 3, 1    // Second triangle to complete the quad
        };
        
        float4 vertices[] = {
            float4{ -0.5f, -0.5f, 1.0f, 1.0f }, float4{ 0.0f, 0.0f, 0.0f, 0.0f }, // Bottom-left
            float4{ -0.5f,  0.5f, 1.0f, 1.0f }, float4{ 0.0f, 1.0f, 0.0f, 0.0f }, // Top-left
            float4{  0.5f,  0.5f, 1.0f, 1.0f }, float4{ 1.0f, 1.0f, 0.0f, 0.0f }, // Top-right
            float4{  0.5f, -0.5f, 1.0f, 1.0f }, float4{ 1.0f, 0.0f, 0.0f, 0.0f }, // Bottom-right
        };


        mVertexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(vertices), sizeof(float4) * 2, RHIBufferUsage::kVertex | RHIBufferUsage::kShaderRead));
        mVBV = mStarters.Device->CreateBufferView(RHIBufferViewDesc(mVertexBuffer, RHIBufferViewType::kStructured));

        mIndexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(indices), sizeof(uint), RHIBufferUsage::kIndex | RHIBufferUsage::kShaderRead));
        mIBV = mStarters.Device->CreateBufferView(RHIBufferViewDesc(mIndexBuffer, RHIBufferViewType::kStructured));
        
        mBLAS = mStarters.Device->CreateBLAS(RHIBLASDesc(mVertexBuffer, mIndexBuffer));
        mTLAS = mStarters.Device->CreateTLAS();
        mInstanceBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(TLASInstance), sizeof(TLASInstance), RHIBufferUsage::kConstant));
        
        mView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mStarters.RenderTexture, RHITextureViewType::kShaderWrite));

        ImageData data = Image::LoadImageData("Data/Textures/AlphaCutoutCheck.png");

        RHITextureDesc textureDesc;
        textureDesc.Width = data.Width;
        textureDesc.Height = data.Height;
        textureDesc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
        textureDesc.Usage = RHITextureUsage::kShaderResource;
        
        mTexture = mStarters.Device->CreateTexture(textureDesc);
        mTextureSRV = mStarters.Device->CreateTextureView(RHITextureViewDesc(mTexture, RHITextureViewType::kShaderRead));
        mSampler = mStarters.Device->CreateSampler(RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kNearest, false));

        TLASInstance instance = {};
        instance.Transform = glm::identity<glm::mat3x4>();
        instance.AccelerationStructureReference = mBLAS->GetAddress();
        instance.Mask = 1;
        instance.InstanceCustomIndex = 0;
        instance.Flags = TLAS_INSTANCE_NON_OPAQUE;

        void* ptr = mInstanceBuffer->Map();
        memcpy(ptr, &instance, sizeof(instance));
        mInstanceBuffer->Unmap();

        CompiledShader shader = ShaderCompiler::Compile("Tests/TLASNonOpaqueInstance.slang", { "CSMain" });

        RHIComputePipelineDesc desc = {};
        desc.ComputeBytecode = shader.Entries["CSMain"];
        desc.PushConstantSize = sizeof(uint) * 8;
        mComputePipeline = mStarters.Device->CreateComputePipeline(desc);

        Uploader::EnqueueTextureUploadRaw(data.Pixels.data(), data.Pixels.size(), mTexture);
        Uploader::EnqueueBufferUpload(vertices, sizeof(vertices), mVertexBuffer);
        Uploader::EnqueueBufferUpload(indices, sizeof(indices), mIndexBuffer);
        Uploader::EnqueueBLASBuild(mBLAS);
        Uploader::EnqueueTLASBuild(mTLAS, mInstanceBuffer, 1);
        Uploader::Flush();
    }

    ~TLASNonOpaqueInstanceTest()
    {
        delete mSampler;
        delete mTextureSRV;
        delete mTexture;
        delete mTLAS;
        delete mBLAS;
        delete mIBV;
        delete mIndexBuffer;
        delete mVBV;
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

            BindlessHandle VBV;
            BindlessHandle IBV;
            BindlessHandle TextureSRV;
            BindlessHandle Sampler;
        } handle = {
            mView->GetBindlessHandle(),
            mTLAS->GetBindlessHandle(),
            TEST_WIDTH,
            TEST_HEIGHT,

            mVBV->GetBindlessHandle(),
            mIBV->GetBindlessHandle(),
            mTextureSRV->GetBindlessHandle(),
            mSampler->GetBindlessHandle()
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
    IRHIBufferView* mVBV;

    IRHIBuffer* mIndexBuffer;
    IRHIBufferView* mIBV;

    IRHITexture* mTexture;
    IRHITextureView* mTextureSRV;
    
    IRHISampler* mSampler;

    IRHIBLAS* mBLAS;
    IRHITLAS* mTLAS;
    IRHIBuffer* mInstanceBuffer;
    
    IRHIComputePipeline* mComputePipeline;
};

DEFINE_RHI_TEST(TLASNonOpaqueInstance) {
    TLASNonOpaqueInstanceTest test(backend);
    return test.Run();
}
