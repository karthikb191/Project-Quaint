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
    , m_renderPass(context)
    {}

    void RenderFrameScene::construct()
    {
        m_renderPass.construct();
        buildGraphicsContext();
        // Build Graphics context
        // Build swapchain
    }

    void RenderFrameScene::destroy()
    {

    }

    void RenderFrameScene::buildGraphicsContext()
    {
        m_graphicsContext.buildCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        , EQueueType::Graphics | EQueueType::Transfer);

        // How to build renderpass
    }

    void RenderFrameScene::buildFrameBuffer()
    {
    }
}}