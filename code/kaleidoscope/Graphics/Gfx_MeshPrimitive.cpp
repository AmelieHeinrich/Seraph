//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 12:25:50
//

#include "Gfx_MeshPrimitive.h"
#include "Gfx_Uploader.h"
#include "Gfx_Manager.h"

namespace Gfx
{
    void MeshPrimitive::FromPrimitive(const KDA::MeshPrimitive& primitive)
    {
        KGPU::IDevice* device = Manager::GetDevice();

        mVertexBuffer = device->CreateBuffer(KGPU::BufferDesc(primitive.Vertices.size() * sizeof(KDA::MeshVertex), sizeof(KDA::MeshVertex), KGPU::BufferUsage::kShaderRead));
        mIndexBuffer = device->CreateBuffer(KGPU::BufferDesc(primitive.Indices.size() * sizeof(uint), sizeof(uint), KGPU::BufferUsage::kIndex));
        mVertexBufferView = device->CreateBufferView(KGPU::BufferViewDesc(mVertexBuffer, KGPU::BufferViewType::kStructured));
        mIndexBufferView = device->CreateBufferView(KGPU::BufferViewDesc(mIndexBuffer, KGPU::BufferViewType::kStructured));
    
        Uploader::EnqueueBufferUpload(primitive.Vertices.data(), mVertexBuffer->GetDesc().Size, mVertexBuffer);
        Uploader::EnqueueBufferUpload(primitive.Indices.data(), mIndexBuffer->GetDesc().Size, mIndexBuffer);
        if (device->SupportsRaytracing()) {
            mBLAS = device->CreateBLAS(KGPU::BLASDesc(mVertexBuffer, mIndexBuffer));
            Uploader::EnqueueBLASBuild(mBLAS);
        }
    }

    MeshPrimitive::~MeshPrimitive()
    {
        if (mVertexBuffer) KC_DELETE(mVertexBuffer);
        if (mIndexBuffer) KC_DELETE(mIndexBuffer);
        if (mBLAS) KC_DELETE(mBLAS);
        if (mIndexBufferView) KC_DELETE(mIndexBufferView);
        if (mVertexBufferView) KC_DELETE(mVertexBufferView);        
    }
}
