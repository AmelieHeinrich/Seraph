//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 16:29:33
//

#include "VulkanBindlessManager.h"
#include "VulkanDevice.h"

VulkanBindlessManager::VulkanBindlessManager(VulkanDevice* device)
    : mParentDevice(device)
{
    // Layout
    VkDescriptorType cbvSrvUavTypes[] = {
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    };

    VkMutableDescriptorTypeListVALVE cbvSrvUavTypeList = {};
    cbvSrvUavTypeList.descriptorTypeCount = sizeof(cbvSrvUavTypes)/sizeof(VkDescriptorType);
    cbvSrvUavTypeList.pDescriptorTypes = cbvSrvUavTypes;

    VkMutableDescriptorTypeCreateInfoEXT mutableTypeInfo = {};
    mutableTypeInfo.sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT;
    mutableTypeInfo.pNext = nullptr;
    mutableTypeInfo.mutableDescriptorTypeListCount = 1;
    mutableTypeInfo.pMutableDescriptorTypeLists = &cbvSrvUavTypeList;

    VkDescriptorSetLayoutBinding cbvSrvUavBinding = {};
    cbvSrvUavBinding.binding = 0;
    cbvSrvUavBinding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    cbvSrvUavBinding.descriptorCount = MAX_BINDLESS_RESOURCES;
    cbvSrvUavBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    cbvSrvUavBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.binding = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerBinding.descriptorCount = MAX_BINDLESS_SAMPLERS;
    samplerBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    samplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding asBinding = {};
    asBinding.binding = 2;
    asBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    asBinding.descriptorCount = MAX_BINDLESS_AS;
    asBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    asBinding.pImmutableSamplers = nullptr;

    Array<VkDescriptorSetLayoutBinding> bindings = { cbvSrvUavBinding, samplerBinding, asBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = &mutableTypeInfo;
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    VkResult result = vkCreateDescriptorSetLayout(mParentDevice->Device(), &layoutInfo, nullptr, &mLayout);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create bindless set layout!");

    // Pool
    Array<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_MUTABLE_EXT, MAX_BINDLESS_RESOURCES },
        { VK_DESCRIPTOR_TYPE_SAMPLER, MAX_BINDLESS_SAMPLERS },
        { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_BINDLESS_AS }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.pNext = &mutableTypeInfo;

    result = vkCreateDescriptorPool(mParentDevice->Device(), &poolInfo, nullptr, &mPool);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan descriptor pool!");

    // Set
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &mLayout;

    result = vkAllocateDescriptorSets(mParentDevice->Device(), &allocInfo, &mSet);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan descriptor set!");

    SERAPH_INFO("Initialized Vulkan Bindless Manager");
}

VulkanBindlessManager::~VulkanBindlessManager()
{
    if (mSet) vkFreeDescriptorSets(mParentDevice->Device(), mPool, 1, &mSet);
    if (mLayout) vkDestroyDescriptorSetLayout(mParentDevice->Device(), mLayout, nullptr);
    if (mPool) vkDestroyDescriptorPool(mParentDevice->Device(), mPool, nullptr);
}
