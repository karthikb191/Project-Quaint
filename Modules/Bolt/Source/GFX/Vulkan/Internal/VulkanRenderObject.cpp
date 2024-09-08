#include <GFX/Vulkan/Internal/VulkanRenderObject.h>
#include <GFX/Entities/RenderObject.h>
#include <GFX/Vulkan/VulkanHelpers.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QMap.h>

namespace Bolt { namespace vulkan {
    VulkanRenderObject::VulkanRenderObject(RenderObject* ro)
    : IRenderObjectImpl(ro)
    , m_shaderGroup(nullptr,  Deleter<VulkanShaderGroup>(ro->getMemoryContext()))
    , m_setLayouts(ro->getMemoryContext())
    {

    }

    void VulkanRenderObject::build(const ShaderInfo& shaderinfo)
    {
        //TODO: fetch shader group
        // Create shader group
        // Create descriptor layout information
        // Create vertex attribute
        // Create descriptors
        // Create pipeline

        //TODO: Hardcoding for now, but get this information from ShaderData. Doesn't make sense otherwise
        createShaderGroup(shaderinfo);
        createDescriptorLayoutInformation(shaderinfo);
    }

    void VulkanRenderObject::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        if(m_shaderGroup.get())
        {
            m_shaderGroup->destroy();
            m_shaderGroup.release();
        }

        //Destroying pipeline layouts
        for(auto& layout : m_setLayouts)
        {
            vkDestroyDescriptorSetLayout(device, layout, callbacks);
        }
        m_setLayouts.clear();

        if(m_pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, m_pipelineLayout, callbacks);
            m_pipelineLayout = VK_NULL_HANDLE;
        }

        if(m_descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device, m_descriptorPool, callbacks);
            m_descriptorPool = VK_NULL_HANDLE;
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
    
    //TODO: Handle layout for multiple frames-in-flight
    void VulkanRenderObject::createDescriptorLayoutInformation(const ShaderInfo& shaderinfo)
    {
        Quaint::IMemoryContext* memContext = m_renderObject->getMemoryContext();
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        Quaint::QArray<VkDescriptorSetLayoutCreateInfo> layoutInfos(memContext);
        Quaint::QArray<VkDescriptorSetLayoutBinding> bindingInfos(memContext);

        uint32_t currentSet = -1;
        for(auto& resource : shaderinfo.resources)
        {
            // TODO: Handle this better
            assert(resource.set == currentSet || resource.set == currentSet + 1 && "Information not in order.");
            if(resource.set == currentSet + 1)
            {
                VkDescriptorSetLayoutCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfos.pushBack(info);

                if(bindingInfos.getSize() > 0)
                {
                    layoutInfos[currentSet].bindingCount = bindingInfos.getSize();
                    layoutInfos[currentSet].pBindings = bindingInfos.getBuffer();
                    layoutInfos[currentSet].flags = 0;
                    layoutInfos[currentSet].pNext = nullptr;

                    VkDescriptorSetLayout layout;
                    ASSERT_SUCCESS(vkCreateDescriptorSetLayout(device, &info, callbacks, &layout), "Failed to create descriptor set layout");
                    m_setLayouts.pushBack(layout);
                    bindingInfos.clear();
                }
                ++currentSet;
            }

            VkDescriptorSetLayoutBinding binding{};
            binding.binding = resource.binding;
            binding.descriptorCount = 1; //TODO: Only supporting one for now
            binding.descriptorType = toVulkanDescriptorType(resource.type);
            binding.stageFlags = toVulkanShaderStageFlagBits(resource.stage);
        }

        if(bindingInfos.getSize() > 0)
        {
            VkDescriptorSetLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfos.pushBack(info);

            layoutInfos[currentSet].bindingCount = bindingInfos.getSize();
            layoutInfos[currentSet].pBindings = bindingInfos.getBuffer();
            layoutInfos[currentSet].flags = 0;
            layoutInfos[currentSet].pNext = nullptr;

            VkDescriptorSetLayout layout;
            ASSERT_SUCCESS(vkCreateDescriptorSetLayout(device, &info, callbacks, &layout), "Failed to create descriptor set layout");
            m_setLayouts.pushBack(layout);
            bindingInfos.clear();
        }

        VkPipelineLayoutCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = m_setLayouts.getSize();
        info.pSetLayouts = m_setLayouts.getBuffer();

        info.pushConstantRangeCount = 0; //TODO: Not handling it for now 
        info.pPushConstantRanges = nullptr; //TODO: Not handling it for now
        info.flags = 0;
        info.pNext = nullptr;
        ASSERT_SUCCESS(vkCreatePipelineLayout(device, &info, callbacks, &m_pipelineLayout), "Failed to create pipeline layout");
    }

    void VulkanRenderObject::createDescriptors(const ShaderInfo& shaderInfo)
    {
        Quaint::IMemoryContext* memContext = m_renderObject->getMemoryContext();
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        
        Quaint::QMap<VkDescriptorType, uint32_t> numTypes(m_renderObject->getMemoryContext());
        for(auto& resource : shaderInfo.resources)
        {
            VkDescriptorType type = toVulkanDescriptorType(resource.type);
            if(!numTypes.contains(type))
            {
                numTypes.insert({type, 1});
            }
            else
            {
                ++numTypes[type];
            }
        }

        Quaint::QArray<VkDescriptorPoolSize> poolSizes(m_renderObject->getMemoryContext());
        for(auto& entry : numTypes)
        {
            VkDescriptorPoolSize size{};
            size.type = entry.first;
            size.descriptorCount = entry.second;
            poolSizes.pushBack(size);
        }

        //TODO: Get this data from shader data
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = m_setLayouts.getSize();
        poolInfo.poolSizeCount = poolSizes.getSize();
        poolInfo.pPoolSizes = poolSizes.getBuffer();
        poolInfo.flags = 0;
        poolInfo.pNext = nullptr;

        ASSERT_SUCCESS( vkCreateDescriptorPool(device, &poolInfo, callbacks, &m_descriptorPool), "Failed to create descriptor pool");

    }

    void VulkanRenderObject::createPipeline()
    {
        
    }

}}