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
    , m_impl(nullptr, context)
    , m_context(context)
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
    }

    void RenderScene::destroy()
    {

    }

    bool RenderScene::construct()
    {
        VulkanRenderScene* scene = getRenderSceneImplAs<VulkanRenderScene>();
        scene->construct(this);
        return true;
    }

    bool RenderScene::begin()
    {
        return true;
    }
    bool RenderScene::render()
    {
        return true;
    }
    bool RenderScene::end()
    {
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


    Attachment::Attachment(const Bolt::AttachmentDefinition& info)
    {
        this->buildAttachmentDescription();
        this->buildAttachmentReference();
    };
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
        
        //TODO: Make internal constructions bools and return failure if construction fails
        m_valid = true;
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
            const Bolt::RenderStage& stage = stages[i];
            VkSubpassDescription desc{};
            desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            Quaint::QArray<VkAttachmentReference> colorAttachrefs(m_context);
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
        if(m_framebuffer.get() != nullptr)
        {
            m_framebuffer->destroy();
        }
        m_framebuffer.reset(QUAINT_NEW( m_context, FrameBuffer, m_context));
        m_framebuffer->construct(this);
    }

    bool VulkanRenderScene::begin()
    {
        return true;
    }
    void VulkanRenderScene::end()
    {

    }
    void VulkanRenderScene::submit()
    {

    }
}}