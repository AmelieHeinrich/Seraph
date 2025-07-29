//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 18:36:31
//

#pragma once

#include <KernelGPU/KGPU_ShaderCompat.h>

namespace SP
{
    constexpr float CAMERA_NEAR = 0.1f;
    constexpr float CAMERA_FAR = 150.0f;

    using namespace KGPU;

    class EditorCamera
    {
    public:
        EditorCamera() = default;
        ~EditorCamera() = default;

        void Update(float dt, int width, int height);

        float4x4 View() const { return mView; }
        float4x4 Projection() const { return mProjection; }
        float3 Position() const { return mPosition; }
    private:
        float4x4 mView = float4x4(1.0f);
        float4x4 mProjection = float4x4(1.0f);
        float3 mPosition = float3(0.0f, 0.0f, 1.0f);
        float3 mForward = float3(0.0f);
        float3 mUp = float3(0.0f);
        float3 mRight = float3(0.0f);

        // To calculate forward
        float mPitch = 0.0f;
        float mYaw = -90.0f;

        // Last mouse
        float mLastX = 0.0f;
        float mLastY = 0.0f;
    };
}
