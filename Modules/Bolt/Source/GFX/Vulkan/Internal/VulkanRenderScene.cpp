#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/VulkanRenderScene.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <GFX/Entities/RenderScene.h>
#include <GFX/Vulkan/VulkanHelpers.h>
#include <GFX/Vulkan/Internal/Entities/VulkanSwapchain.h>

namespace Bolt { 
    RenderScene::RenderScene(Quaint::IMemoryContext* context, Quaint::QName name, const RenderInfo& renderInfo)
    : m_stages(context)
    {
        m_impl = Quaint::QUniquePtr<RenderSceneImpl>(QUAINT_NEW(context, vulkan::VulkanRenderScene, context));
        assert(m_impl != nullptr, "Vulkan scene not constructed");
    }
    RenderScene::~RenderScene()
    {
        if(m_isValid)
        {
            destroy();
        }
    }

    bool RenderScene::construct()
    {
        VulkanRenderScene* scene = getRenderSceneImplAs<VulkanRenderScene>();
        scene->construct(this);
    }

    bool RenderScene::begin()
    {

    }
    bool RenderScene::render()
    {

    }
    bool RenderScene::end()
    {

    }
    
    namespace vulkan{

    void ImageAttachment::buildAttachmentDescription()
    {
        attachmentDescription.initialLayout = texture.getCreateInfo().imageInfo.initialLayout;
        attachmentDescription.format = texture.getCreateInfo().imageInfo.format;
        attachmentDescription.samples = texture.getCreateInfo().imageInfo.samples;
        if(info.clearImage)
        {
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        }
        else
        {
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        }
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    void ImageAttachment::buildAttachmentReference()
    {
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    void SwapchainAttachment::buildAttachmentDescription()
    {
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //We don't care
        attachmentDescription.format = getSwapchain()->getFormat();
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        if(info.clearImage)
        {
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        }
        else
        {
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        }
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    void SwapchainAttachment::buildAttachmentReference()
    {
        attachmentReference.attachment = info.binding;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VulkanRenderScene::VulkanRenderScene(Quaint::IMemoryContext* context)
    : m_context(context)
    , m_graphicsContext(context)
    , m_attachments(context)
    , m_subpassDesc(context)
    , m_subpassDependencies(context)
    {}

    void VulkanRenderScene::destroy()
    {
        //Free all attachments
        for(auto& attachment : m_attachments)
        {
            switch(attachment->getInfo().type)
            {
                case AttachmentDefinition::Type::Image:
                {
                    ImageAttachment* atch = attachment->As<ImageAttachment>();
                    atch->texture.destroy();
                    attachment.release();
                }
                break;
                case AttachmentDefinition::Type::Swapchain:
                {
                    attachment.release();
                }
                break;
                case AttachmentDefinition::Type::Depth:
                default:
                    //Do nothing for now
            }
        }
        m_attachments.clear();
    }

    void VulkanRenderScene::construct(const Bolt::RenderScene* scene)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        const Bolt::RenderInfo& info = scene->getRenderInfo();
        constructAttachments(info);
        constructSubpasses(scene);

        Quaint::QArray<VkAttachmentDescription> attachDescs(m_context);
        for(auto& attachment : m_attachments)
        {
            attachDescs.pushBack(attachment->getAttachmentDescription());
        }

        VkRenderPassCreateInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        rpInfo.attachmentCount = attachDescs.getSize();
        rpInfo.pAttachments = attachDescs.getBuffer();
        rpInfo.subpassCount = m_subpassDesc.getSize();
        rpInfo.pSubpasses = m_subpassDesc.getBuffer();
        rpInfo.dependencyCount = m_subpassDependencies.getSize();
        rpInfo.pDependencies = m_subpassDependencies.getBuffer();
        
        VkResult res = vkCreateRenderPass(device, &rpInfo, callbacks, &m_renderpass);
        ASSERT_SUCCESS(res, "could not create renderpass");

        constructFrameBuffer();
    }

    void VulkanRenderScene::constructAttachments(const Bolt::RenderInfo& info)
    {
        for(int i = 0; i < info.attachments.getSize(); ++i)
        {
            const Bolt::AttachmentDefinition& def = info.attachments[i];
            switch (def.type)
            {
            case AttachmentDefinition::Type::Image:
            {
                //TODO: handle the scenario where we need multiple textures for each swapchain image
                VulkanTexture texture = constructVulkanTexture(def);
                m_attachments.emplace(Quaint::QUniquePtr<Attachment>(QUAINT_NEW(m_context, ImageAttachment, info, texture)));
            }
            break;
            case AttachmentDefinition::Type::Swapchain:
            {
                m_attachments.emplace(Quaint::QUniquePtr<Attachment>(QUAINT_NEW(m_context, SwapchainAttachment, info)));
            }
            break;
            case AttachmentDefinition::Type::Depth:
            default:
                assert(false, "Not yet supported");
            break;
            }
        }
    }

    VulkanTexture VulkanRenderScene::constructVulkanTexture(const Bolt::AttachmentDefinition def)
    {
        //TODO: Move this to builder
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        //Creates, sets backing memory and creates image view
        VulkanTextureBuilder builder;
        VulkanTexture tex = 
        builder.setFormat(toVulkanVkFormat(def.format))
        .setWidth((uint32_t)def.extents.x)
        .setHeight((uint32_t)def.extents.y)
        .setTiling(VK_IMAGE_TILING_LINEAR)
        .setInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        .setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .setImageViewInfo(viewInfo)
        .setBuildImage()
        .setBackingMemory()
        .setBuildImageView()
        .build();
    }

    void VulkanRenderScene::constructSubpasses(const Bolt::RenderScene* scene)
    {
        const Quaint::QArray<Bolt::RenderScene::RenderStage>& stages = scene->getRenderStages();
        if(stages.getSize() == 0)
        {
            assert(false, "No render stages supplied");
            return;
        }

        m_subpassDesc.clear();
        m_subpassDependencies.clear();
        uint32_t idx = 0;
        for(size_t i = 0; i < stages.getSize(); ++i)
        {
            const Bolt::RenderScene::RenderStage& stage = stages[i];
            VkSubpassDescription desc{};
            desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            Quaint::QArray<VkAttachmentReference> colorAttachrefs(m_context);
            for(int i = 0; i < stage.attachmentRefs.getSize(); ++i)
            {
                Attachment* targetAttachment = nullptr;
                const auto& stageAttach = stage.attachmentRefs[i];
                Attachment* sceneAttachRef = getAttachment(stageAttach.attachmentName);
                if (sceneAttachRef == nullptr)
                {
                    continue;
                }
                VkAttachmentReference ref{};
                ref.attachment = sceneAttachRef->getInfo().binding;
                if(sceneAttachRef->getInfo().type == AttachmentDefinition::Type::Depth)
                {
                    ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                }
                else
                {
                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
                colorAttachrefs.pushBack(ref);
            }
            desc.colorAttachmentCount = colorAttachrefs.getSize();
            desc.pColorAttachments = colorAttachrefs.getBuffer();

            //TODO: No other attachment types are supported for now
            desc.inputAttachmentCount = 0;
            desc.pInputAttachments = nullptr;
            desc.pDepthStencilAttachment = nullptr;
            desc.preserveAttachmentCount = 0;
            desc.pPreserveAttachments = nullptr;
            desc.pResolveAttachments = nullptr;
            m_subpassDesc.pushBack(desc);

            
            VkSubpassDependency dep{};

            //This is the only stage we care about for now
            dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            // Result of previous stage is visible to the next stage
            
            if(stage.index == stages.getSize() - 1)
            {
                //Final subpass
                // Allow read/write access for any blending operations. After this, we only have read access
                dep.srcSubpass = stage.index;
                dep.dstSubpass = VK_SUBPASS_EXTERNAL;
                dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dep.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                dep.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            }
            else 
            {
                dep.srcSubpass = stage.index;
                dep.dstSubpass = stage.index + 1; // Dependency with the next subpass
                //Dst will have shader read access
            }
            m_subpassDependencies.pushBack(dep);
        }
    }
    Attachment* VulkanRenderScene::getAttachment(const Quaint::QName& name)
    {
        for(auto& attachment : m_attachments)
        {
            if(attachment->getInfo().name == name)
            {
                return attachment.get();
            }
        }
        return nullptr;
    }

    void VulkanRenderScene::constructFrameBuffer()
    {

    }

    AttachmentInfo& RenderScene::beginAttachmentSetup()
    {
        AttachmentInfo info;
        info.scene = this;
        info.index = (uint32_t)m_attchmentInfos.getSize();
        m_attchmentInfos.pushBack(info);
        return m_attchmentInfos[info.index];
    }

    RenderFrameScene::RenderFrameScene(Quaint::IMemoryContext* context, const uint32_t framesInFlight)
    : RenderScene(context)
    , m_framesInFlight(framesInFlight)
    , m_nextFrameIndex(0)
    , m_renderPass(context)
    , m_framebuffers(context)
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
        m_swapchainExtent = swapExtent;
        
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

        const Bolt::Window& window = RenderModule::get().getBoltRenderer()->getWindow();
        
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = VulkanRenderer::get()->getSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = swapchainInfo->getFormat();
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = {m_swapchainExtent.width, m_swapchainExtent.height}; //Should match with the one setup in attachment
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
        , EQueueType::Graphics | EQueueType::Transfer, true);
        m_graphicsContext.construct();
        // How to build renderpass
    }

    void RenderFrameScene::buildFrameBuffer()
    {
        VulkanRenderer::SwapchainSupportInfo supportInfo = 
        querySwapchainSupport(m_context, VulkanRenderer::get()->getPhysicalDevice(), VulkanRenderer::get()->getSurface());

        VkExtent2D swapExtent = chooseSwapExtent(m_context, supportInfo);

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
            .setExtent({m_swapchainExtent.width, m_swapchainExtent.height, 1})
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

    bool RenderFrameScene::begin()
    {
        m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;

        FrameInfo& frameInfo = m_frameInfo[m_currentFrame];
        VkDevice device = VulkanRenderer::get()->getDevice();
        vkWaitForFences(device, 1, &frameInfo.renderFence, VK_TRUE, UINT64_MAX);
        
        //Semaphore is signalled when presentation engine finishes using the image
        VkResult res = vkAcquireNextImageKHR(device, m_swapchain, UINT64_MAX, frameInfo.scImageAvailableSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);

        //if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        if(res != VK_SUCCESS)
        {
            //recreateSwapchain();
            int i = 100;
            i -= 100;
            return false;
        }
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

        VkClearValue clearColor = {0.0f, 0.0f, 0.5f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return true;
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

        VkResult res = vkQueuePresentKHR(handle, &presentInfo);
        if(res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            vkDeviceWaitIdle(VulkanRenderer::get()->getDevice());
            m_outOfDateSwapchain = m_swapchain;
            setupSwapchain();

            buildFrameBuffer();
            return;
        }
        ASSERT_SUCCESS(res, "Failed to submit to present queue");
    }

}}