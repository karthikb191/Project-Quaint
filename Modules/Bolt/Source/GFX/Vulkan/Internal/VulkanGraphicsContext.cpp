#include <GFX/Vulkan/Internal/VulkanGraphicsContext.h>


namespace Bolt { namespace vulkan
{

    GraphicsContext::GraphicsContext(Quaint::IMemoryContext* context)
    : m_context(context)
    {}

}}