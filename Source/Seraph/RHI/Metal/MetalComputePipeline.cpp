//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:04:55
//

#include "MetalComputePipeline.h"
#include "MetalDevice.h"

#include <metal_irconverter/metal_irconverter.h>

MetalComputePipeline::MetalComputePipeline(MetalDevice* device, RHIComputePipelineDesc desc)
{
    // Convert to MBC
    IRCompiler* compiler = IRCompilerCreate();
    IRObject* pDXIL = IRObjectCreateFromDXIL(desc.ComputeBytecode.Bytecode.data(), desc.ComputeBytecode.Bytecode.size(), IRBytecodeOwnershipNone);

    IRError* err = nullptr;
    IRObject* pOutIR = IRCompilerAllocCompileAndLink(compiler, desc.ComputeBytecode.Entry.c_str(), pDXIL, &err);
    if (!pOutIR) {
        SERAPH_ERROR("%s", IRErrorGetPayload(err));
        IRErrorDestroy(err);

    }

    IRMetalLibBinary* pMetalLib = IRMetalLibBinaryCreate();
    IRObjectGetMetalLibBinary(pOutIR, IRShaderStageCompute, pMetalLib);
    uint64 libSize = IRMetalLibGetBytecodeSize(pMetalLib);
    dispatch_data_t data = IRMetalLibGetBytecodeData(pMetalLib);

    NS::Error* nserr = nullptr;
    MTL::Library* library = device->GetDevice()->newLibrary(data, &nserr);
    if (nserr) {
        SERAPH_ERROR("NS Error! %s", nserr->localizedDescription()->utf8String());
    }

    // Create state
    NS::String* functionName = NS::String::alloc()->init(desc.ComputeBytecode.Entry.c_str(), NS::ASCIIStringEncoding);
    mPipelineState = device->GetDevice()->newComputePipelineState(library->newFunction(functionName), &nserr);
    if (nserr) {
        SERAPH_ERROR("NS Error! %s", nserr->localizedDescription()->utf8String());
    }

    IRMetalLibBinaryDestroy(pMetalLib);
    IRObjectDestroy(pDXIL);
    IRObjectDestroy(pOutIR);
    IRCompilerDestroy(compiler);

    SERAPH_WHATEVER("Created Metal compute pipeline state!");
}

MetalComputePipeline::~MetalComputePipeline()
{
   
}
