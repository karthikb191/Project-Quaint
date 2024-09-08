#include <GFX/Vulkan/Internal/VulkanRenderObject.h>
#include <GFX/Entities/RenderObject.h>
#include <GFX/Vulkan/VulkanHelpers.h>

namespace Bolt { namespace vulkan {
    VulkanRenderObject::VulkanRenderObject(RenderObject* ro)
    : IRenderObjectImpl(ro)
    , m_shaderGroup(nullptr,  Deleter<VulkanShaderGroup>(ro->getMemoryContext()))
    {

    }

    void VulkanRenderObject::build(const ShaderInfo& shaderinfo)
    {
        //TODO: fetch shader group
        // Create shader group
        // Create attachment information
        // Create descriptor layout information
        // Create descriptors
        // Create pipeline

        //TODO: Hardcoding for now, but get this information from ShaderData. Doesn't make sense otherwise
        createShaderGroup(shaderinfo);
    }

    void VulkanRenderObject::destroy()
    {
        if(m_shaderGroup.get())
        {
            m_shaderGroup->destroy();
            m_shaderGroup.release();
        }
    }

    void VulkanRenderObject::createShaderGroup(const ShaderInfo& shaderinfo)
    {
        //TODO: Get this data from shader data
        m_shaderGroup.reset(QUAINT_NEW(m_renderObject->getMemoryContext(), VulkanShaderGroup
                            , shaderinfo.vertShaderPath.getBuffer()
                            , shaderinfo.fragShaderPath.getBuffer()));
    }
    void VulkanRenderObject::createVertexAttributeInformation(const ShaderInfo& shaderinfo)
    {
        //TODO: Get this data from shader data
        
    }
    
    void VulkanRenderObject::createDescriptorLayoutInformation(const ShaderInfo& shaderinfo)
    {
        //TODO: Get this data from shader data
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        VkDescriptorSetLayout layout{};
        

        VkPipelineLayoutCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //info.
    }

    void VulkanRenderObject::createDescriptors()
    {
        //TODO: Get this data from shader data

    }

    void VulkanRenderObject::createPipeline()
    {

    }

}}