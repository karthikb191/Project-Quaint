#include <GFX/Vulkan/Internal/VulkanShaderGroup.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt
{
    VulkanShaderGroup::VulkanShaderGroup()
    {

    }

    VulkanShaderGroup::VulkanShaderGroup(const char* vertSprvPath, const char* fragSpirvPath)
    {
        m_vertShader.reset(QUAINT_NEW(Bolt::VulkanRenderer::get()->getMemoryContext(), Bolt::VulkanVertexShader, vertSprvPath));
        m_fragShader.reset(QUAINT_NEW(Bolt::VulkanRenderer::get()->getMemoryContext(), Bolt::VulkanFragmentShader, fragSpirvPath));
        //m_fragShader = std::make_unique(fragSpirvPath);
    }

    VulkanShaderGroup::~VulkanShaderGroup()
    {
        
    }

    bool VulkanShaderGroup::isValid() const
    {
        return m_vertShader.get() != nullptr && m_fragShader.get() != nullptr;
    }
}