//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-04-07 00:31:49
//

#pragma once

#include <Core/Types.h>

constexpr float CAMERA_NEAR = 0.1f;
constexpr float CAMERA_FAR = 150.0f;

class Camera
{
public:
    Camera() = default;
    ~Camera() = default;

    void Begin();
    void Update(float dt, int width, int height);

    glm::mat4 View() const { return mView; }
    glm::mat4 Projection() const { return mProjection; }
    float3 Position() const { return mPosition; }
private:
    glm::mat4 mView = glm::mat4(1.0f);
    glm::mat4 mProjection = glm::mat4(1.0f);
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
