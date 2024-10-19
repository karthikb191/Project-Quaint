#include <GFX/ResourceBuilder.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>

//Vulkan Specific implementation of resource builder
//TODO: Surround with VULKAN_API macro
namespace Bolt {
    using namespace vulkan;

    //Image Resource builder
    GraphicsResource* CombinedImageSamplerTextureBuilder::buildFromPath(const char* path)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        VulkanCombinedImageSamplerResource* proxy = QUAINT_NEW(m_context, VulkanCombinedImageSamplerResource);
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.unnormalizedCoordinates = VK_FALSE; // We are using normalized coordinates
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.magFilter = VK_FILTER_LINEAR;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        //TODO: Not entirely sure of the way this functions. Need to read more on this
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = 0;

        VkSampler sampler;
        VkResult res = vkCreateSampler(device, &samplerInfo, callbacks, &sampler);

        VulkanTexture texture;
        texture.defaultInit();
        //TODO: Hardcoding flag for now. Create a comming enum for that later
        VulkanRenderer::get()->createTextureFromFile(path, texture, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        proxy->wrap(sampler, texture);

        GraphicsResource* resource = GraphicsResource::create(m_context, EResourceType::COMBINED_IMAGE_SAMPLER, proxy);
        return resource;
    }

    //UB resource builder
    GraphicsResource* UniformBufferResourceBuilder::buildFromData(void* data, uint32_t size)
    {
        return nullptr;
    }
}