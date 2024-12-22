#include <vulkan/vulkan.h>
#include <assert.h>
#include "../Data/ShaderInfo.h"
#include "../Data/RenderInfo.h"

namespace Bolt{ namespace vulkan{
    
    inline VkDescriptorType toVulkanDescriptorType(const EShaderResourceType type)
    {
        switch(type)
        {
            case EShaderResourceType::COMBINED_IMAGE_SAMPLER:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case EShaderResourceType::UNIFORM_BUFFER:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case EShaderResourceType::INVALID:
            default:
                assert(false && "Not handled yet!");
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
    
    //TODO: Extend this to support a combination of shader stages
    inline VkShaderStageFlagBits toVulkanShaderStageFlagBits(const EShaderStage stage)
    {
        switch(stage)
        {
            case EShaderStage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case EShaderStage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case EShaderStage::GEOMETRY:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            case EShaderStage::COMPUTE:
                return VK_SHADER_STAGE_COMPUTE_BIT;

            case EShaderStage::INVALID:
            default:
                assert(false && "Not handled yet!");
        }
        return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }

    inline VkFormat toVulkanVkFormat(const Bolt::EFormat format)
    {
        switch(format)
        {
            case Bolt::EFormat::R32G32B32A32_UINT:
                return VK_FORMAT_R32G32B32A32_UINT;
            case Bolt::EFormat::R32G32B32A32_SFLOAT:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case Bolt::EFormat::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            default:
            {
                assert(false && "Format conversion not available. invalid format passed");
                break;
            }
        }
        return VK_FORMAT_UNDEFINED;
    }
}}