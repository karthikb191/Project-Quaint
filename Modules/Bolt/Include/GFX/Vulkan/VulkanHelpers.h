#include <vulkan/vulkan.h>
#include <assert.h>
#include "../Data/ShaderInfo.h"
#include "../Data/RenderInfo.h"

namespace Bolt{ namespace vulkan{
    
    #define TRY_CONSUME_FLAG(res, ipFlags, IMAGE_FLAG, VULKAN_FLAG) \
        if(ipFlags & IMAGE_FLAG) { \
            ipFlags &= ~IMAGE_FLAG; \
            res |= VULKAN_FLAG; \
        }\
    
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
            case Bolt::EFormat::R32G32_SFLOAT:
                return VK_FORMAT_R32G32_SFLOAT; 
            case Bolt::EFormat::R32G32B32A32_SFLOAT:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case Bolt::EFormat::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case Bolt::EFormat::R8G8B8A8_UNORM:
                return VK_FORMAT_R8G8B8A8_UNORM;
                
            default:
            {
                assert(false && "Format conversion not available. invalid format passed");
                break;
            }
        }
        return VK_FORMAT_UNDEFINED;
    }

    inline VkShaderStageFlagBits toShaderStageFlags(const EShaderStage stage)
    {
        switch(stage)
        {
            case EShaderStage::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
            case EShaderStage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
            case EShaderStage::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
            case EShaderStage::COMPUTE: return VK_SHADER_STAGE_COMPUTE_BIT;
            default: return VK_SHADER_STAGE_ALL;
        }
    }

    inline VkImageUsageFlags toVulkanImageUsage(const EImageUsageFlags flags)
    {
        EImageUsageFlags remainingFlags = flags;
        VkImageUsageFlags res = 0;

        TRY_CONSUME_FLAG(res, remainingFlags, EImageUsage::COLOR_ATTACHMENT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        TRY_CONSUME_FLAG(res, remainingFlags, EImageUsage::DEPTH_ATTACHMENT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        TRY_CONSUME_FLAG(res, remainingFlags, EImageUsage::INPUT_ATTACHMENT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
        TRY_CONSUME_FLAG(res, remainingFlags, EImageUsage::SAMPLED, VK_IMAGE_USAGE_SAMPLED_BIT)
        TRY_CONSUME_FLAG(res, remainingFlags, EImageUsage::COMBINED_IMAGE_SAMPLER, VK_IMAGE_USAGE_SAMPLED_BIT)
        TRY_CONSUME_FLAG(res, remainingFlags, EImageUsage::COPY_DST, VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        TRY_CONSUME_FLAG(res, remainingFlags, EImageUsage::COPY_SRC, VK_IMAGE_USAGE_TRANSFER_SRC_BIT)

        assert(remainingFlags == 0 && "Not all flags have been consumed");
        return res;
    }

    inline uint8_t getTexelSizeFromFormat(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
            return 4;

        case VK_FORMAT_R32G32B32A32_SFLOAT: 
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_UINT:
            return 16;

        case VK_FORMAT_R32G32_SFLOAT:
            return 8;

        case VK_FORMAT_R32G32B32_SFLOAT:
            return 12;

        case VK_FORMAT_D32_SFLOAT:
            return 4;

        default:
            assert(false && "unsupported format");
            break;
        }
    }
}}