#include <GFX/Vulkan/Internal/RenderScene.h>

namespace Bolt { namespace vulkan{
    RenderScene::RenderScene(Quaint::IMemoryContext* context)
    : m_graphicsContext(context)
    , m_attchmentInfos(context)
    {}

    AttachmentInfo& RenderScene::beginAttachmentSetup()
    {
        AttachmentInfo info;
        info.scene = this;
        info.index = m_attchmentInfos.getSize();
        m_attchmentInfos.pushBack(info);
        return m_attchmentInfos[info.index];
    }

    RenderFrameScene::RenderFrameScene(Quaint::IMemoryContext* context, const uint32_t framesInFlight)
    : RenderScene(context)
    , m_framesInFlight(framesInFlight)
    , m_nextFrameIndex(0)
    {}

    void RenderFrameScene::construct()
    {
        // Build Commandpool
        // Build swapchain
        // Build Graphics context
    }

    void RenderFrameScene::destroy()
    {
        
    }

    RenderFrameScene& RenderFrameScene::buildFrameBuffer()
    {
        return *this;
    }
}}