//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:13:55
//

#pragma once

#include "KGPU_ShaderModule.h"
#include "KGPU_Texture.h"

#include <KernelCore/KC_HashMap.h>

namespace KGPU
{
    enum class FillMode
    {
        kSolid,
        kWireframe
    };
    
    enum class CullMode
    {
        kBack,
        kFront,
        kNone
    };
    
    enum class DepthOperation
    {
        kGreater,
        kLess,
        kEqual,
        kLessEqual,
        kNone
    };
    
    struct GraphicsPipelineDesc
    {
        FillMode FillMode = FillMode::kSolid;
        CullMode CullMode = CullMode::kNone;
        bool CounterClockwise = true;
        bool LineTopology = false;
    
        uint PushConstantSize = 0;
        
        KC::Array<TextureFormat> RenderTargetFormats;
        DepthOperation DepthOperation = DepthOperation::kLess;
        TextureFormat DepthFormat = TextureFormat::kD32_FLOAT;
        bool DepthEnabled = false;
        bool DepthClampEnabled = false;
        bool DepthWrite = true;
        bool ReflectInputLayout = true;
        bool EnableBlend = false;
    
        KC::HashMap<ShaderStage, ShaderModule> Modules;
    };
    
    class IGraphicsPipeline
    {
    public:
        virtual ~IGraphicsPipeline() = default;
    
        const GraphicsPipelineDesc& GetDesc() const { return mDesc; }
    protected:
        GraphicsPipelineDesc mDesc;
    };
}
