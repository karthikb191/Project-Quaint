#include <GFX/Vulkan/Internal/VulkanShaderGroup.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt
{
    VulkanShaderGroup::VulkanShaderGroup()
    {
        Quaint::IMemoryContext* context = VulkanRenderer::get()->getMemoryContext();
        m_VIBs = Quaint::QArray<VkVertexInputBindingDescription>(context);
        m_VIAs = Quaint::QArray<VkVertexInputAttributeDescription>(context);
    }

    VulkanShaderGroup::VulkanShaderGroup(const char* vertSprvPath, const char* fragSpirvPath)
    {
        m_vertShader.reset(QUAINT_NEW(Bolt::VulkanRenderer::get()->getMemoryContext(), Bolt::VulkanVertexShader, vertSprvPath));
        m_fragShader.reset(QUAINT_NEW(Bolt::VulkanRenderer::get()->getMemoryContext(), Bolt::VulkanFragmentShader, fragSpirvPath));
        
        Quaint::IMemoryContext* context = VulkanRenderer::get()->getMemoryContext();
        m_VIBs = Quaint::QArray<VkVertexInputBindingDescription>(context);
        m_VIAs = Quaint::QArray<VkVertexInputAttributeDescription>(context);
    }

    VulkanShaderGroup::~VulkanShaderGroup()
    {
        
    }

    bool VulkanShaderGroup::isValid() const
    {
        return m_vertShader.get() != nullptr && m_fragShader.get() != nullptr;
    }

    
    void VulkanShaderGroup::addVertexInputBindingDescription(const VkVertexInputBindingDescription& desc)
    {
        //TODO: Perform any validation checks here
        m_VIBs.pushBack(desc);
    }

    void VulkanShaderGroup::addAttributeInputAttributeDescription(const VkVertexInputAttributeDescription& desc)
    {
        m_VIAs.pushBack(desc);
    }
    
}