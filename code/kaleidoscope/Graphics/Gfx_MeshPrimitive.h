//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 12:22:54
//

#pragma once

#include <KDAsset/KDA_MeshLoader.h>
#include <KernelGPU/KGPU_Device.h>

namespace Gfx
{
    class MeshPrimitive
    {
    public:
        MeshPrimitive() = default;
        MeshPrimitive(const KDA::MeshPrimitive& primitive) { FromPrimitive(primitive); }
        ~MeshPrimitive();

        void FromPrimitive(const KDA::MeshPrimitive& primitive);

        KGPU::IBuffer* GetVertexBuffer() { return mVertexBuffer; }
        KGPU::IBuffer* GetIndexBuffer() { return mIndexBuffer; }
        KGPU::IBLAS* GetBLAS() { return mBLAS; }
        KGPU::IBufferView* GetVertexBufferView() { return mVertexBufferView; }
        KGPU::IBufferView* GetIndexBufferView() { return mIndexBufferView; }

        uint GetIndexCount() const { return mIndexBuffer->GetDesc().Size / sizeof(uint); }
    private:
        KGPU::IBuffer* mVertexBuffer = nullptr;
        KGPU::IBufferView* mVertexBufferView = nullptr;
        KGPU::IBuffer* mIndexBuffer = nullptr;
        KGPU::IBufferView* mIndexBufferView = nullptr;
        KGPU::IBLAS* mBLAS = nullptr;
    };
}
