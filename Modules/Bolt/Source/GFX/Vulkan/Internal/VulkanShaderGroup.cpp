#include <GFX/Vulkan/Internal/VulkanShaderGroup.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt { namespace vulkan
{
    VulkanShaderGroup::VulkanShaderGroup()
    : m_vertShader(nullptr, Deleter<VulkanVertexShader>(VulkanRenderer::get()->getMemoryContext()))
    , m_fragShader(nullptr, Deleter<VulkanFragmentShader>(VulkanRenderer::get()->getMemoryContext()))
    {
        Quaint::IMemoryContext* context = VulkanRenderer::get()->getMemoryContext();
        m_VIBs = Quaint::QArray<VkVertexInputBindingDescription>(context);
        m_VIAs = Quaint::QArray<VkVertexInputAttributeDescription>(context);
    }

    VulkanShaderGroup::VulkanShaderGroup(const char* vertSprvPath, const char* fragSpirvPath)
    : m_vertShader(nullptr, Deleter<VulkanVertexShader>(VulkanRenderer::get()->getMemoryContext()))
    , m_fragShader(nullptr, Deleter<VulkanFragmentShader>(VulkanRenderer::get()->getMemoryContext()))
    {
        m_vertShader.reset(QUAINT_NEW(Bolt::VulkanRenderer::get()->getMemoryContext(), Bolt::VulkanVertexShader, vertSprvPath));
        m_fragShader.reset(QUAINT_NEW(Bolt::VulkanRenderer::get()->getMemoryContext(), Bolt::VulkanFragmentShader, fragSpirvPath));
        
        Quaint::IMemoryContext* context = VulkanRenderer::get()->getMemoryContext();
        m_VIBs = Quaint::QArray<VkVertexInputBindingDescription>(context);
        m_VIAs = Quaint::QArray<VkVertexInputAttributeDescription>(context);
    }

    VulkanShaderGroup::~VulkanShaderGroup()
    {
        //TODO: Assert destroyed. It'll be an invalid state otherwise
    }

    void VulkanShaderGroup::destroy()
    {
        if(m_vertShader.get())
        {
            m_vertShader->destroy();
            m_vertShader.release();
        }
        if(m_fragShader.get())
        {
            m_fragShader->destroy();
            m_fragShader.release();
        }
    }

    bool VulkanShaderGroup::isValid() const
    {
        return m_vertShader.get() != nullptr && m_fragShader.get() != nullptr;
    }
    
    //TODO: Not sure there are necessary here. Remove if they aren't
    void VulkanShaderGroup::addVertexInputBindingDescription(const VkVertexInputBindingDescription& desc)
    {
        //TODO: Perform any validation checks here
        m_VIBs.pushBack(desc);
    }

    void VulkanShaderGroup::addAttributeInputAttributeDescription(const VkVertexInputAttributeDescription& desc)
    {
        m_VIAs.pushBack(desc);
    }
    
}}