#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/RenderScene.h>

namespace Bolt { namespace vulkan{
    RenderScene::RenderScene(Quaint::IMemoryContext* context)
    : m_context(context)
    , m_graphicsContext(context)
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
    , m_frameInfo(context)
    {}

    void RenderFrameScene::construct()
    {
        //TODO: Find a place to add command buffers
        buildGraphicsContext();
        m_renderPass.constructFromScene(this);
        setupSwapchain();
        buildFrameBuffer();
        setupFrameInfo();
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
        createInfo.imageExtent = {swapchainInfo->getExtent().width, swapchainInfo->getExtent().height}; //Should match with the one setup in attachment
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

        //Registering swapchain extent for easier access
        m_swapchainExtent = createInfo.imageExtent;
        VkResult res = vkCreateSwapchainKHR(VulkanRenderer::get()->getDevice(), &createInfo, VulkanRenderer::get()->getAllocationCallbacks(), &m_swapchain);
        assert(res == VK_SUCCESS && "Swapchain creation failed! Application will terminate");
    }

    void RenderFrameScene::buildGraphicsContext()
    {
        m_graphicsContext.buildCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        , EQueueType::Graphics | EQueueType::Transfer, true);
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

        m_framebuffers.resizeWithArgs(swapchainImageCount, m_context);
        for(uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            FrameBuffer buffer(m_context);
            m_framebuffers[i]
            .addAttachment(swapchainImages[i], swapchainInfo, families)
            .setExtent(swapchainInfo->getExtent())
            .setRenderpass(m_renderPass.getRenderPass())
            .construct(this);
        }
    }

    void RenderFrameScene::setupFrameInfo()
    {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semInfo.flags = 0;

        for(uint32_t i = 0; i < m_framesInFlight; ++i)
        {
            FrameInfo info{};
            auto buffers = m_graphicsContext.addCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &info.commandBuffer);
            info.commandBuffer = buffers[0];

            ASSERT_SUCCESS(vkCreateFence(VulkanRenderer::get()->getDevice(), &fenceInfo, VulkanRenderer::get()->getAllocationCallbacks(), &info.renderFence)
                , "Coule not create fence");

            ASSERT_SUCCESS(vkCreateSemaphore(VulkanRenderer::get()->getDevice(), &semInfo, VulkanRenderer::get()->getAllocationCallbacks(), &info.scImageAvailableSemaphore)
                , "Could not create semaphore");

            ASSERT_SUCCESS(vkCreateSemaphore(VulkanRenderer::get()->getDevice(), &semInfo, VulkanRenderer::get()->getAllocationCallbacks(), &info.renderFinishedSemaphore)
            , "Could not create Render finished semaphore");
            m_frameInfo.pushBack(info);
        }
    }

    void RenderFrameScene::begin()
    {
        m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;

        FrameInfo& frameInfo = m_frameInfo[m_currentFrame];
        VkDevice device = VulkanRenderer::get()->getDevice();
        vkWaitForFences(device, 1, &frameInfo.renderFence, VK_TRUE, UINT64_MAX);
        
        //Semaphore is signalled when presentation engine finishes using the image
        VkResult res = vkAcquireNextImageKHR(device, m_swapchain, UINT64_MAX, frameInfo.scImageAvailableSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);

        //if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        //{
        //    recreateSwapchain();
        //    return;
        //}
        assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR && "Failed to acquire swapchain image");
        vkResetFences(device, 1, &frameInfo.renderFence);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional\\

        vkResetCommandBuffer(frameInfo.commandBuffer, 0);

        res = vkBeginCommandBuffer(frameInfo.commandBuffer, &beginInfo);
        assert(res==VK_SUCCESS && "Could not being command buffer");

        //Now we begin a renderpass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass.getRenderPass();
        renderPassInfo.framebuffer = m_framebuffers[m_currentImageIndex].getHandle();

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchainExtent;

        VkClearValue clearColor = {0.5f, 0.1f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderFrameScene::end()
    {
        FrameInfo& frameInfo = m_frameInfo[m_currentFrame];
        vkCmdEndRenderPass(frameInfo.commandBuffer);
        vkEndCommandBuffer(frameInfo.commandBuffer);
    }

    void RenderFrameScene::submit()
    {
        FrameInfo& frameInfo = m_frameInfo[m_currentFrame];
        VkQueue handle = m_graphicsContext.getCommandPool().getQueueDefinition().getVulkanQueueHandle();

        //Submit to graphics queue.
        // Wait for image to be available at COLOR_ATTACHMENT pipeline stage and signal render finished semaphore
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &frameInfo.commandBuffer;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &frameInfo.scImageAvailableSemaphore;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &frameInfo.renderFinishedSemaphore;
        info.pWaitDstStageMask = waitStages;
        ASSERT_SUCCESS(vkQueueSubmit(handle, 1, &info, frameInfo.renderFence), "Submit to graphics queue failed");

        //Submit to presentation queue
        //Wait for render to complete
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &frameInfo.renderFinishedSemaphore;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &m_currentImageIndex;

       ASSERT_SUCCESS(vkQueuePresentKHR(handle, &presentInfo), "Failed to submit to present queue");
    }

}}