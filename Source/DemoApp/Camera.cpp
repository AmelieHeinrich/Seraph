//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-05 22:30:28
//

#include "Camera.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>

void Camera::Begin()
{
    ImVec2 pos = ImGui::GetMousePos();
    mLastX = pos.x;
    mLastY = pos.y;
}

void Camera::Update(float dt, int width, int height)
{
    if (ImGui::IsKeyDown(ImGuiKey_Z)) {
        mPosition -= mForward * dt * 3.0f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        mPosition += mForward * dt * 3.0f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        mPosition += mRight * dt * 3.0f;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        mPosition -= mRight * dt * 3.0f;
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 pos = ImGui::GetMousePos();
        float dx = (pos.x - mLastX) * 0.1f;
        float dy = (pos.y - mLastY) * 0.1f;

        mYaw += dx;
        mPitch -= dy;
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