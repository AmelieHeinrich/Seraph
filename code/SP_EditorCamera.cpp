//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 18:37:38
//

#include "SP_EditorCamera.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <KernelInput/KI_InputSystem.h>

namespace SP
{
    void EditorCamera::Update(float dt, int width, int height)
    {
        if (KI::InputSystem::IsKeyDown(KI::Keycode::kZ)) {
            mPosition += mForward * dt * 3.0f;
        }
        if (KI::InputSystem::IsKeyDown(KI::Keycode::kS)) {
            mPosition -= mForward * dt * 3.0f;
        }
        if (KI::InputSystem::IsKeyDown(KI::Keycode::kQ)) {
            mPosition -= mRight * dt * 3.0f;
        }
        if (KI::InputSystem::IsKeyDown(KI::Keycode::kD)) {
            mPosition += mRight * dt * 3.0f;
        }

        if (KI::InputSystem::IsMouseDown(KI::MouseButton::kLeft)) {
            auto delta = KI::InputSystem::GetMouseDelta();

            mYaw += delta.x * 0.1f;
            mPitch -= delta.y * 0.1f;
        }

        mForward.x = glm::cos(glm::radians(mYaw)) * glm::cos(glm::radians(mPitch));
        mForward.y = glm::sin(glm::radians(mPitch));
        mForward.z = glm::sin(glm::radians(mYaw)) * glm::cos(glm::radians(mPitch));
        mForward = glm::normalize(mForward);

        mRight = glm::normalize(glm::cross(mForward, float3(0.0f, 1.0f, 0.0f)));
        mUp = glm::normalize(glm::cross(mRight, mForward));

        mView = glm::lookAt(mPosition, mPosition + mForward, float3(0.0f, 1.0f, 0.0f));
        mProjection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, CAMERA_NEAR, CAMERA_FAR);
    }
}
