#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/VulkanFrameBuffer.h>

namespace Bolt{ namespace vulkan{

    FrameBuffer::FrameBuffer(Quaint::IMemoryContext* context)
    : m_context(context)
    {}

}}