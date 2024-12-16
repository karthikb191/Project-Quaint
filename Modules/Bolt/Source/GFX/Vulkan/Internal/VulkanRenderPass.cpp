#include <LoggerModule.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/VulkanRenderPass.h>
#include <GFX/Vulkan/Internal/RenderScene.h>

namespace Bolt { namespace vulkan
{
    DECLARE_LOG_CATEGORY(VulkanRenderPass);
    DEFINE_LOG_CATEGORY(VulkanRenderPass);

    const VulkanRenderPass::Subpass  VulkanRenderPass::SUBPASS_EXTERNAL;

//=============================================================================================

    VulkanRenderPass::Subpass::Subpass(Quaint::IMemoryContext* context)
    {
        m_attachmentRefs = Quaint::QArray<VkAttachmentReference>(context);
    }

    void VulkanRenderPass::Subpass::construct()
    {
        const VkAttachmentReference* attachmentRefBuffer = m_attachmentRefs.getBuffer();

        // Attchment refs are stored in order Input - Color - Depth - Resolve
        m_desc.inputAttachmentCount = m_numAttachments[EAttachmentType::Input];
        m_desc.colorAttachmentCount = m_numAttachments[EAttachmentType::Color];
        
        assert(m_pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && "Only supporting graphics bind point for now");
        assert(m_numAttachments[EAttachmentType::Input] == 0 &&
                m_numAttachments[EAttachmentType::Depth] == 0 && 
                m_numAttachments[EAttachmentType::Resolve] == 0 && "Not yet supported");

        m_desc.pipelineBindPoint = m_pipelineBindPoint;

        m_desc.pInputAttachments = nullptr;
        m_desc.pColorAttachments = attachmentRefBuffer;
        m_desc.pDepthStencilAttachment = nullptr;
        m_desc.pPreserveAttachments = nullptr;
        m_desc.pResolveAttachments = nullptr;
        m_desc.flags = 0;

        //TODO: If everything works, uncomment the code below and check that nothing breaks
        //m_desc.pInputAttachments = attachmentRefBuffer;
        //m_desc.pColorAttachments = attachmentRefBuffer + m_numAttachments[EAttachmentType::Input];
        //m_desc.pDepthStencilAttachment = attachmentRefBuffer 
        //                                + m_numAttachments[EAttachmentType::Input]
        //                                + m_numAttachments[EAttachmentType::Color];
        //m_desc.pResolveAttachments = attachmentRefBuffer 
        //                                + m_numAttachments[EAttachmentType::Input]
        //                                + m_numAttachments[EAttachmentType::Color]
        //                                + m_numAttachments[EAttachmentType::Depth];
        //
        ////TODO: Implement attachment preserve across the subpass
        //m_desc.preserveAttachmentCount = 0;
        //m_desc.pPreserveAttachments = nullptr;


        //TODO: Populate other types of attachments
    }
    void VulkanRenderPass::Subpass::destroy()
    {

    }

    VulkanRenderPass::Subpass& VulkanRenderPass::Subpass::addColorAttachment(const VkAttachmentReference& ref)
    {
        assert(m_numAttachments[EAttachmentType::Depth] == 0 && m_numAttachments[EAttachmentType::Resolve] == 0
            && "Add attachments in order input - color - depth - resolve");

        ++m_numAttachments[EAttachmentType::Color];
        m_attachmentRefs.pushBack(ref);
        return *this;
    }

    VulkanRenderPass::Subpass& VulkanRenderPass::Subpass::addColorAttachment(const uint32_t attachIndex, const VkImageLayout layout)
    {
        assert(m_numAttachments[EAttachmentType::Depth] == 0 && m_numAttachments[EAttachmentType::Resolve] == 0
            && "Add attachments in order input - color - depth - resolve");

        VkAttachmentReference ref {};
        ref.attachment = attachIndex;
        ref.layout = layout;

        ++m_numAttachments[EAttachmentType::Color];
        m_attachmentRefs.pushBack(ref);
        return *this;
    }

//=============================================================================================

    VulkanRenderPass::VulkanRenderPass(Quaint::IMemoryContext* context, RenderScene* scene)
    : m_context(context)
    //, m_scene(scene)
    {
        m_attchmentInfos = Quaint::QArray<VulkanRenderPass::AttachmentInfo>(m_context);
        m_subPasses = Quaint::QArray<VulkanRenderPass::Subpass>(m_context);
        m_subPassDependencies = Quaint::QArray<VkSubpassDependency>(m_context);
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        destroy();
    }

    void VulkanRenderPass::destroy()
    {
        if(isValid())
        {
            VkDevice device = VulkanRenderer::get()->getDeviceManager()->getDeviceDefinition().getDevice();
            VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

            for(auto& subpass : m_subPasses)
            {
                subpass.destroy();
            }

            if(m_renderPass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(device, m_renderPass, callbacks);
            }
            m_renderPass = VK_NULL_HANDLE;
            QLOG_I(VulkanRenderPass, "Destroyed Vulkan Renderpass");
        }
    }

    VulkanRenderPass::AttachmentInfo& VulkanRenderPass::addEmptyAttachment()
    {
        AttachmentInfo info;
        info.index = (uint32_t)m_attchmentInfos.getSize();
        m_attchmentInfos.pushBack(info);
        return m_attchmentInfos[info.index];
    }

    //Create subpass from this info
    VulkanRenderPass::Subpass& VulkanRenderPass::addEmptySubpass()
    {
        Subpass subpass(m_context);
        subpass.m_index = (uint32_t)m_subPasses.getSize();
        m_subPasses.pushBack(subpass);
        return m_subPasses[subpass.m_index];
    }

    VulkanRenderPass::Subpass& VulkanRenderPass::beginSubpassSetup()
    {
        Subpass subpass(m_context);
        subpass.m_index = (uint32_t)m_subPasses.getSize();
        m_subPasses.pushBack(subpass);
        return m_subPasses[subpass.m_index];
    }

    VulkanRenderPass& VulkanRenderPass::addSubpassDependency(const Subpass& src, const Subpass& dst
                                , VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask
                                , VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask
                                , VkDependencyFlags dependencyFlags)
    {
        //TODO: Surround with a debug macro
        if(src.m_index == VK_SUBPASS_EXTERNAL || dst.m_index == VK_SUBPASS_EXTERNAL)
        {
            assert(src.m_index != dst.m_index && "src and dst subpasses should not both be equal to VK_SUBPASS_EXTERNAL");
        }
        else
        {
            assert(dst.m_index >= src.m_index && "dst subpass should be larger than src to avoid cyclic dependences");
        }

        VkSubpassDependency dependency {};
        dependency.srcSubpass = src.m_index;
        dependency.dstSubpass = dst.m_index;
        dependency.srcStageMask = srcStageMask;
        dependency.dstStageMask = dstStageMask;
        dependency.srcAccessMask = srcAccessMask;
        dependency.dstAccessMask = dstAccessMask;
        dependency.dependencyFlags = dependencyFlags;
        m_subPassDependencies.pushBack(dependency);

        return *this;
    }

    void VulkanRenderPass::construct()
    {
        for(Subpass& subpass : m_subPasses)
        {
            subpass.construct();
        }

        VkRenderPassCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        info.flags = 0; //Not used for now. TODO: Check back later
        
        // populating Attachments information
        uint32_t numAttachments = (uint32_t)m_attchmentInfos.getSize();
        Quaint::QArray<VkAttachmentDescription> attachDescs(m_context, numAttachments);
        if(numAttachments > 0)
        {
            for(uint32_t i = 0; i < numAttachments; ++i)
            {
                attachDescs[i] = m_attchmentInfos[i].desc;
            }
            info.attachmentCount = (uint32_t)m_attchmentInfos.getSize();
            info.pAttachments = attachDescs.getBuffer();
        }

        //Populating subpass information
        uint32_t numSubpasses = (uint32_t)m_subPasses.getSize();
        Quaint::QArray<VkSubpassDescription> subpassDescs(m_context, numSubpasses);
        assert(numSubpasses > 0 && "RenderPass needs atleast a single subpass!!");
        for(uint32_t i = 0; i < numSubpasses; ++i)
        {
            subpassDescs[i] = m_subPasses[i].getDescription();
        }
        info.subpassCount = numSubpasses;
        info.pSubpasses = subpassDescs.getBuffer();

        //Populating subpass dependency information
        uint32_t numDependencies = (uint32_t)m_subPassDependencies.getSize();
        if(numDependencies > 0)
        {
            info.dependencyCount = (uint32_t)m_subPassDependencies.getSize();
            info.pDependencies = m_subPassDependencies.getBuffer();
        }
        
        VkDevice device = VulkanRenderer::get()->getDeviceManager()->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        VkResult res = vkCreateRenderPass(device, &info, callbacks, &m_renderPass);
        ASSERT_SUCCESS(res, "Could not create render pass!!");
    }

    void VulkanRenderPass::constructFromScene(const RenderScene* scene)
    {
        for(Subpass& subpass : m_subPasses)
        {
            subpass.construct();
        }

        VkRenderPassCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        info.flags = 0; //Not used for now. TODO: Check back later
        
        // populating Attachments information
        const auto& attachmentInfos = scene->getAttachmentInfos();

        uint32_t numAttachments = (uint32_t)attachmentInfos.getSize();
        Quaint::QArray<VkAttachmentDescription> attachDescs(m_context, numAttachments);
        if(numAttachments > 0)
        {
            for(uint32_t i = 0; i < numAttachments; ++i)
            {
                attachDescs[i] = attachmentInfos[i].getDescription();
            }
            info.attachmentCount = (uint32_t)attachmentInfos.getSize();
            info.pAttachments = attachDescs.getBuffer();
        }

        //Populating subpass information
        uint32_t numSubpasses = (uint32_t)m_subPasses.getSize();
        Quaint::QArray<VkSubpassDescription> subpassDescs(m_context, numSubpasses);
        assert(numSubpasses > 0 && "RenderPass needs atleast a single subpass!!");
        for(uint32_t i = 0; i < numSubpasses; ++i)
        {
            subpassDescs[i] = m_subPasses[i].getDescription();
        }
        info.subpassCount = numSubpasses;
        info.pSubpasses = subpassDescs.getBuffer();

        //Populating subpass dependency information
        uint32_t numDependencies = (uint32_t)m_subPassDependencies.getSize();
        if(numDependencies > 0)
        {
            info.dependencyCount = (uint32_t)m_subPassDependencies.getSize();
            info.pDependencies = m_subPassDependencies.getBuffer();
        }
        
        VkDevice device = VulkanRenderer::get()->getDeviceManager()->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        VkResult res = vkCreateRenderPass(device, &info, callbacks, &m_renderPass);
        ASSERT_SUCCESS(res, "Could not create render pass!!");
    }
}}