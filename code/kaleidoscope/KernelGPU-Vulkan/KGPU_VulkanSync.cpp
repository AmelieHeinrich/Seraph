//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:27:36
//

#include "KGPU_VulkanSync.h"
#include "KGPU_VulkanDevice.h"
#include "KGPU_VulkanCommandQueue.h"
#include "KGPU_VulkanCommandList.h"

namespace KGPU
{
    VulkanSync::VulkanSync(VulkanDevice* device, VulkanSurface* surface, VulkanCommandQueue* queue)
        : mParentDevice(device), mSurface(surface), mCommandQueue(queue)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            VkResult result = vkCreateSemaphore(device->Device(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore[i]);
            KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create image available semaphore!");

            result = vkCreateSemaphore(device->Device(), &semaphoreInfo, nullptr, &mImageRenderedSemaphore[i]);
            KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create image rendered semaphore!");

            result = vkCreateFence(device->Device(), &fenceInfo, nullptr, &mInFlightFences[i]);
            KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create in flight fence!");
        }

        KD_WHATEVER("Created Vulkan F2F sync");
    }

    VulkanSync::~VulkanSync()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(mParentDevice->Device(), mImageRenderedSemaphore[i], nullptr);
            vkDestroySemaphore(mParentDevice->Device(), mImageAvailableSemaphore[i], nullptr);
            vkDestroyFence(mParentDevice->Device(), mInFlightFences[i], nullptr);
        }
    }

    uint VulkanSync::BeginSynchronize()
    {
        vkWaitForFences(mParentDevice->Device(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(mParentDevice->Device(), mSurface->GetSwapchain(), UINT64_MAX, mImageAvailableSemaphore[mCurrentFrame], VK_NULL_HANDLE, &mImageIndex);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to get next swapchain image!");

        vkResetFences(mParentDevice->Device(), 1, &mInFlightFences[mCurrentFrame]);

        return mCurrentFrame;
    }

    void VulkanSync::EndSynchronize(ICommandList* submitBuffer)
    {
        VulkanCommandList* vkBuffer = static_cast<VulkanCommandList*>(submitBuffer);
        VkCommandBuffer cmdBuffer = vkBuffer->GetCommandBuffer();

        VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore[mCurrentFrame] };
        VkSemaphore signalSemaphores[] = { mImageRenderedSemaphore[mCurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.pSignalSemaphores = signalSemaphores;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.waitSemaphoreCount = 1;

        VkResult result = vkQueueSubmit(mCommandQueue->GetQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to end F2F synchronization!");
    }

    void VulkanSync::PresentSurface()
    {
        VkSemaphore waitSemaphores[] = { mImageRenderedSemaphore[mCurrentFrame] };
        VkSwapchainKHR swapChains[] = { mSurface->GetSwapchain() };

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &mImageIndex;

        VkResult result = vkQueuePresentKHR(mCommandQueue->GetQueue(), &presentInfo);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to present F2F surface!");

        mCurrentFrame = (mCurrentFrame + 1) % FRAMES_IN_FLIGHT;
    }
}
