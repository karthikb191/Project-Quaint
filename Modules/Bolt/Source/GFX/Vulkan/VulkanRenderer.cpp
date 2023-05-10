#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt
{
    void VulkanRenderer::init(Quaint::IMemoryContext* context)
    {
        m_context = context;
    }
}