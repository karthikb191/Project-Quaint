
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
}}