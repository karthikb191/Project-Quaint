#include <GFX/Vulkan/Internal/RenderScene.h>

namespace Bolt { namespace vulkan{
    RenderScene::RenderScene(Quaint::IMemoryContext* context)
    : m_graphicsContext(context)
    {}

    RenderFrameScene::RenderFrameScene(Quaint::IMemoryContext* context, const uint32_t framesInFlight)
    : RenderScene(context)
    , m_framesInFlight(framesInFlight)
    , m_nextFrameIndex(0)
    
    {}

}}