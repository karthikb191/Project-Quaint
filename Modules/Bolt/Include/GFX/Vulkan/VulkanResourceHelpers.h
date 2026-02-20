
#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>

namespace Bolt{ namespace vulkan{
class VulkanCombinedImageSamplerResource;
inline VkWriteDescriptorSet getCombinedImageSamplerDescriptorWrite(VkDescriptorSet set, VkDescriptorImageInfo* imgInfo, vulkan::VulkanCombinedImageSamplerResource* image, uint16_t descriptorCount, uint16_t bindingIdx)
{
    *imgInfo = {};
    imgInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgInfo->sampler = image->getSampler();
    imgInfo->imageView = image->getTexture()->getImageView();

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.descriptorCount = descriptorCount;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = imgInfo;
    write.dstBinding = bindingIdx;
    return write;
}

inline VkWriteDescriptorSet getUniformBufferDescriptorWrite(VkDescriptorSet set, VkDescriptorBufferInfo* bufferInfo, vulkan::VulkanBufferObjectResource* buffer, uint16_t descriptorCount, uint16_t bindingIdx)
{
    *bufferInfo = {};
    bufferInfo->offset = buffer->getBufferInfo().offset;
    bufferInfo->range = buffer->getBufferInfo().size;
    bufferInfo->buffer = buffer->getBufferhandle();

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.descriptorCount = descriptorCount;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = bufferInfo;
    write.dstBinding = bindingIdx;
    return write;
}

inline void createDefaultSampler(VkSampler* sampler)
{
    VkDevice device = VulkanRenderer::get()->getDevice();
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.minLod = -1000;
    info.maxLod = 1000;
    info.maxAnisotropy = 1.0f;
    VkResult res = vkCreateSampler(device, &info, VulkanRenderer::get()->getAllocationCallbacks(), sampler);
}
}}