#include <GFX/Vulkan/VulkanRenderer.h>
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
    , m_framebuffers(context, context)
    , m_frameInfo(context, framesInFlight)
    {}

    void RenderFrameScene::construct()
    {
        //TODO: Find a place to add command buffers
        buildGraphicsContext();
        m_renderPass.constructFromScene(this);
        buildFrameBuffer();
        // Build Graphics context
        // Build swapchain
    }

    void RenderFrameScene::destroy()
    {
        if(m_swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(VulkanRenderer::get()->getDevice(), m_swapchain, VulkanRenderer::get()->getAllocationCallbacks());
        }
        for(uint32_t i = 0; i < m_framebuffers.getSize(); ++i)
        {
            m_framebuffers[i].destroy();
        }
        m_graphicsContext.destroy();
    }

    void RenderFrameScene::setupSwapchain()
    {
        VulkanRenderer::SwapchainSupportInfo supportInfo = 
        querySwapchainSupport(m_context, VulkanRenderer::get()->getPhysicalDevice(), VulkanRenderer::get()->getSurface());

        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(supportInfo);
        VkPresentModeKHR presentMode = choosePresentationMode(m_context, supportInfo);
        VkExtent2D swapExtent = chooseSwapExtent(m_context, supportInfo);
        
        uint32_t imageCount = supportInfo.surfaceCapabilities.minImageCount + 1;
        if (supportInfo.surfaceCapabilities.maxImageCount > 0 && imageCount > supportInfo.surfaceCapabilities.maxImageCount) {
            imageCount = supportInfo.surfaceCapabilities.maxImageCount;
        }

        AttachmentInfo* swapchainInfo = nullptr;
        for(auto& attachment : m_attchmentInfos)
        {
            if(attachment.getIsSwapchainImage())
            {
                swapchainInfo = &attachment;
                break;
            }
        }
        assert(swapchainInfo != nullptr && "could not find swapchain in the attachments setup");

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = VulkanRenderer::get()->getSurface() ;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = swapchainInfo->getFormat();
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = swapExtent;
        createInfo.imageArrayLayers = 1; //This is always 1 unless you are developing stereoscopic 3D app
        //TODO: Change this later if we are using a memory command to transfer images to swapchain
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        // Only allowing exclusive sharing mode for now. 
        // TODO: Handle this once we support multiple queue operations
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        
        //Change this if you want the image to have a different transform 
        createInfo.preTransform = supportInfo.surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //This specifies if alpha channel should be used for blending with other windows
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_outOfDateSwapchain; //This will eventually have a null handle 
        createInfo.presentMode = presentMode;

        VkResult res = vkCreateSwapchainKHR(VulkanRenderer::get()->getDevice(), &createInfo, VulkanRenderer::get()->getAllocationCallbacks(), &m_swapchain);
        assert(res == VK_SUCCESS && "Swapchain creation failed! Application will terminate");
    }

    void RenderFrameScene::buildGraphicsContext()
    {
        m_graphicsContext.buildCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        , EQueueType::Graphics | EQueueType::Transfer);
        m_graphicsContext.construct();
        // How to build renderpass
    }

    void RenderFrameScene::buildFrameBuffer()
    {
        uint32_t swapchainImageCount = 0;
        vkGetSwapchainImagesKHR(VulkanRenderer::get()->getDevice(), m_swapchain, &swapchainImageCount, nullptr);
        assert(swapchainImageCount != 0 && "No images were retrieved from swapchain.");
        Quaint::QArray<VkImage> swapchainImages(m_context);
        swapchainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(VulkanRenderer::get()->getDevice(), m_swapchain, &swapchainImageCount, swapchainImages.getBuffer_NonConst());

        AttachmentInfo* swapchainInfo = nullptr;
        for(auto& attachment : m_attchmentInfos)
        {
            if(attachment.getIsSwapchainImage())
            {
                swapchainInfo = &attachment;
                break;
            }
        }
        assert(swapchainInfo != nullptr && "could not find swapchain in the attachments setup");

        uint32_t family = m_graphicsContext.getCommandPool().getQueueDefinition().getQueueFamily();
        Quaint::QArray<uint32_t> families(m_context);
        families.pushBack(family);

        m_framebuffers.resize(swapchainImageCount);
        for(uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            m_framebuffers[i]
            .addAttachment(swapchainImages[i], swapchainInfo, families)
            .setExtent(swapchainInfo->getExtent())
            .setRenderpass(m_renderPass.getRenderPass())
            .construct(this);
        }
    }

    void RenderFrameScene::begin()
    {

    }
    void RenderFrameScene::end()
    {

    }
}}