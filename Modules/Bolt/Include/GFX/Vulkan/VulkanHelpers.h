#include <vulkan/vulkan.h>
#include <assert.h>
#include "../Data/ShaderInfo.h"

namespace Bolt{ namespace vulkan{
    
    inline VkDescriptorType toVulkanDescriptorType(const EResourceType type)
    {
        switch(type)
        {
            case EResourceType::COMBINED_IMAGE_SAMPLER:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case EResourceType::UNIFORM_BUFFER:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case EResourceType::INVALID:
            default:
                assert(false && "Not handled yet!");
        }
    }

    inline VkShaderStageFlagBits toVulkanShaderStageFlagBits(const EShaderStage stage)
    {
        switch(stage)
        {
            case EShaderStage::VERTEX:
                return  VK_SHADER_STAGE_VERTEX_BIT;
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
    }

}}