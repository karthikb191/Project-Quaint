#define NOMINMAX

#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/VulkanRenderScene.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <GFX/Entities/RenderScene.h>
#include <GFX/Vulkan/VulkanHelpers.h>
#include <GFX/Vulkan/Internal/Entities/VulkanSwapchain.h>
#include <algorithm>


namespace Bolt { 
    RenderScene::RenderScene(Quaint::IMemoryContext* context, Quaint::QName name, const RenderInfo& renderInfo)
    : m_stages(context)
    , m_impl(nullptr, context)
    , m_context(context)
    , m_renderInfo(renderInfo)
    {
        m_impl.reset(QUAINT_NEW(context, vulkan::VulkanRenderScene, context));
        assert(m_impl != nullptr && "Vulkan scene not constructed");
    }
    RenderScene::~RenderScene()
    {
        if(m_isValid)
        {
            destroy();
        }
        m_isValid = false;
    }

    void RenderScene::destroy()
    {

    }

    bool RenderScene::construct()
    {
        VulkanRenderScene* scene = getRenderSceneImplAs<VulkanRenderScene>();
        scene->construct(this);
        m_isValid = true;
        return true;
    }

    bool RenderScene::begin()
    {
        VulkanRenderScene* scene = getRenderSceneImplAs<VulkanRenderScene>();
        return scene->begin();
    }
    bool RenderScene::render()
    {
        return true;
    }
    bool RenderScene::end()
    {
        VulkanRenderScene* scene = getRenderSceneImplAs<VulkanRenderScene>();
        //scene->end();
        return true;
    }

    void RenderScene::addRenderStage(const RenderStage& pStage)
    {
        for(auto& stage : m_stages)
        {
            if(stage.index == pStage.index)
            {
                assert(false && "stage with specified index already added. Skipping");
                return;
            }
        }
        m_stages.pushBack(pStage);
    }
    
    namespace vulkan{


    Attachment::Attachment(const Bolt::AttachmentDefinition& pInfo)
    : info(pInfo)
    {}
    void Attachment::construct()
    {
        this->buildAttachmentDescription();
        this->buildAttachmentReference();
    }
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
    VulkanSwapchain* SwapchainAttachment::getSwapchain()
    {
         return VulkanRenderer::get()->getSwapchain();
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
    , m_framebuffer(nullptr, context)
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
                    assert(false && "unsupported attachment");
                    break;
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

        Quaint::QArray<VkSubpassDescription> subpassDescs(m_context);
        for(auto& desc : m_subpassDesc)
        {
            subpassDescs.pushBack(desc.description);
        }

        Quaint::QArray<VkAttachmentDescription> attachDescs(m_context);
        for(auto& attachment : m_attachments)
        {
            attachDescs.pushBack(attachment->getAttachmentDescription());
        }

        VkRenderPassCreateInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        rpInfo.attachmentCount = attachDescs.getSize();
        rpInfo.pAttachments = attachDescs.getBuffer();
        rpInfo.subpassCount = subpassDescs.getSize();
        rpInfo.pSubpasses = subpassDescs.getBuffer();
        rpInfo.dependencyCount = m_subpassDependencies.getSize();
        rpInfo.pDependencies = m_subpassDependencies.getBuffer();
        
        VkResult res = vkCreateRenderPass(device, &rpInfo, callbacks, &m_renderpass);
        ASSERT_SUCCESS(res, "could not create renderpass");

        constructFrameBuffer();

        //Constructing scene params
        VulkanSwapchain* swapchain = VulkanRenderer::get()->getSwapchain();
        uint32_t swapchainImages = swapchain->getNumSwapchainImages();
        //TODO: One for each swapchain image later
        m_sceneParams.commandBuffer = VulkanRenderer::get()->getGraphicsCommandBuffers(1)[0];
        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semInfo.flags = 0;
        semInfo.pNext = nullptr;
        ASSERT_SUCCESS(vkCreateSemaphore(device, &semInfo, callbacks, &m_sceneParams.renderFinishedSemaphore), "failed to create semaphore");

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;
        fenceInfo.pNext = nullptr;
        ASSERT_SUCCESS(vkCreateFence(device, &fenceInfo, callbacks, &m_sceneParams.renderFence), "failed to create fence");

        //TODO: Make internal constructions bools and return failure if construction fails
        VkExtent2D& swapchainExtent = swapchain->getSwapchainExtent();
        if(info.extents.x == -1 || info.extents.y == -1)
        {
            m_renderExtent = swapchainExtent;
            m_renderOffset = {0, 0};
        }
        else
        {
            assert((info.extents.x + info.offset.x) <= swapchainExtent.width && "Invalid extent and offset combo");
            assert((info.extents.y + info.offset.y) <= swapchainExtent.height && "Invalid extent and offset combo");

            if(info.extents.x + info.offset.x > swapchainExtent.width
            || info.extents.y + info.offset.y > swapchainExtent.height)
            {
                m_renderExtent = swapchainExtent;
                m_renderOffset = {0, 0};
            }
            else
            {
                m_renderExtent = {(uint32_t)info.extents.x, (uint32_t)info.extents.y};
                m_renderOffset = {(int32_t)info.offset.x, (int32_t)info.offset.y};
            }
        }
    }

    void VulkanRenderScene::constructAttachments(const Bolt::RenderInfo& info)
    {
        for(size_t i = 0; i < info.attachments.getSize(); ++i)
        {
            const Bolt::AttachmentDefinition& def = info.attachments[i];
            switch (def.type)
            {
            case AttachmentDefinition::Type::Image:
            {
                //TODO: handle the scenario where we need multiple textures for each swapchain image
                VulkanTexture texture = constructVulkanTexture(def);
                m_attachments.emplace(QUAINT_NEW(m_context, ImageAttachment, def, texture), Bolt::Deleter<Attachment>(m_context));
            }
            break;
            case AttachmentDefinition::Type::Swapchain:
            {
                m_attachments.emplace(QUAINT_NEW(m_context, SwapchainAttachment, def), Bolt::Deleter<Attachment>(m_context));
            }
            break;
            case AttachmentDefinition::Type::Depth:
            default:
                assert(false && "Not yet supported");
            break;
            }
        }

        //Construct all attachments
        for(auto& attachment : m_attachments)
        {
            attachment->construct();
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
        return tex;
    }

    void VulkanRenderScene::constructSubpasses(const Bolt::RenderScene* scene)
    {
        const Quaint::QArray<Bolt::RenderStage>& stages = scene->getRenderStages();
        if(stages.getSize() == 0)
        {
            assert(false && "No render stages supplied");
            return;
        }

        m_subpassDesc.clear();
        m_subpassDependencies.clear();
        uint32_t idx = 0;
        for(size_t i = 0; i < stages.getSize(); ++i)
        {
            SubpassDescription desc{};
            desc.references = Quaint::QArray<VkAttachmentReference>(m_context);
            desc.description = {};
            const Bolt::RenderStage& stage = stages[i];
            desc.description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            for(size_t i = 0; i < stage.attachmentRefs.getSize(); ++i)
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
                desc.references.pushBack(ref);
            }
            desc.description.colorAttachmentCount = desc.references.getSize();
            desc.description.pColorAttachments = desc.references.getBuffer();

            //TODO: No other attachment types are supported for now
            desc.description.inputAttachmentCount = 0;
            desc.description.pInputAttachments = nullptr;
            desc.description.pDepthStencilAttachment = nullptr;
            desc.description.preserveAttachmentCount = 0;
            desc.description.pPreserveAttachments = nullptr;
            desc.description.pResolveAttachments = nullptr;
            m_subpassDesc.pushBack(std::move(desc));
            
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
        if(m_framebuffer.get() != nullptr)
        {
            m_framebuffer->destroy();
        }
        m_framebuffer.reset(QUAINT_NEW( m_context, FrameBuffer, m_context));
        m_framebuffer->construct(this);
    }

    bool VulkanRenderScene::begin()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VulkanSwapchain* swapchain = VulkanRenderer::get()->getSwapchain();
        vkResetCommandBuffer(m_sceneParams.commandBuffer, 0);
        vkResetFences(device, 1, &m_sceneParams.renderFence);

        VkCommandBufferBeginInfo cbinfo{};
        cbinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbinfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cbinfo.pInheritanceInfo = nullptr;
        cbinfo.pNext = nullptr;

        vkBeginCommandBuffer(m_sceneParams.commandBuffer, &cbinfo);

        VkRenderPassBeginInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.framebuffer = m_framebuffer->getHandle(swapchain->getCurrentSwapchainImageIndex());
        rpInfo.renderPass = m_renderpass;
        rpInfo.renderArea.extent = m_renderExtent;
        rpInfo.renderArea.offset = m_renderOffset;
        
        Quaint::QArray<VkClearValue> clearValues(m_context);
        for(auto& attachment : m_attachments)
        {
            if(attachment->getInfo().clearImage)
            {
                if(attachment->getInfo().type == AttachmentDefinition::Type::Swapchain || attachment->getInfo().type == AttachmentDefinition::Type::Image)
                {
                    VkClearValue val{};
                    val.color = {{attachment->getInfo().clearColor.x, 
                                attachment->getInfo().clearColor.y, 
                                attachment->getInfo().clearColor.z,
                                attachment->getInfo().clearColor.w}};
                    clearValues.pushBack(val);
                }
                else
                {
                    clearValues.pushBack({});
                }
            }
            else
            {
                clearValues.pushBack({});
            }
        }

        rpInfo.clearValueCount = clearValues.getSize();
        rpInfo.pClearValues = clearValues.getBuffer();
        rpInfo.pNext = nullptr;

        vkCmdBeginRenderPass(m_sceneParams.commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        return true;
    }
    VulkanRenderScene::SceneParams VulkanRenderScene::end(VkQueue queue)
    {
        vkCmdEndRenderPass(m_sceneParams.commandBuffer);
        vkEndCommandBuffer(m_sceneParams.commandBuffer);
        submit(queue);
        return m_sceneParams;
    }
    void VulkanRenderScene::submit(VkQueue queue)
    {
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //TODO: Add this when ready
        //VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        info.waitSemaphoreCount = 0;
        info.pWaitSemaphores = nullptr;
        info.pWaitDstStageMask = nullptr;

        info.commandBufferCount = 1;
        info.pCommandBuffers = &m_sceneParams.commandBuffer;

        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &m_sceneParams.renderFinishedSemaphore;

        info.pNext = nullptr;

        VkResult res = vkQueueSubmit(queue, 1, &info, m_sceneParams.renderFence);
        ASSERT_SUCCESS(res, "failed to submit to queue");
    }
}}