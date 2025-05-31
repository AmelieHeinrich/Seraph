//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-30 20:51:41
//

#pragma once

#include "ShaderCompiler.h"
#include "Texture.h"

enum class RHIFillMode
{
    kSolid,
    kWireframe
};

enum class RHICullMode
{
    kBack,
    kFront,
    kNone
};

enum class RHIDepthOperation
{
    kGreater,
    kLess,
    kEqual,
    kLessEqual,
    kNone
};

struct RHIGraphicsPipelineDesc
{
    RHIFillMode FillMode = RHIFillMode::kSolid;
    RHICullMode CullMode = RHICullMode::kNone;
    bool CounterClockwise = true;
    bool LineTopology = false;

    uint PushConstantSize = 0;
    
    Array<RHITextureFormat> RenderTargetFormats;
    RHIDepthOperation DepthOperation = RHIDepthOperation::kLess;
    RHITextureFormat DepthFormat = RHITextureFormat::kD32_FLOAT;
    bool DepthEnabled = false;
    bool DepthClampEnabled = false;
    bool DepthWrite = true;
    bool ReflectInputLayout = true;

    UnorderedMap<ShaderStage, ShaderModule> Bytecode;
};

class IRHIGraphicsPipeline
{
public:
    ~IRHIGraphicsPipeline() = default;

    RHIGraphicsPipelineDesc GetDesc() const { return mDesc; }
protected:
    RHIGraphicsPipelineDesc mDesc;
};
