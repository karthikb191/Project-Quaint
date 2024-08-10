#include <GFX/Vulkan/Internal/VulkanRenderPass.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <LoggerModule.h>

namespace Bolt
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
        
        assert( m_pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && "Only supporting graphics bind point for now");
        assert(m_numAttachments[EAttachmentType::Input] == 0 &&
                m_numAttachments[EAttachmentType::Depth] == 0 && 
                m_numAttachments[EAttachmentType::Resolve] == 0 && "Not yet supported");

        m_desc.pipelineBindPoint = m_pipelineBindPoint;

        m_desc.pInputAttachments = nullptr;
        m_desc.pColorAttachments = attachmentRefBuffer;
        m_desc.pDepthStencilAttachment = nullptr;
        m_desc.pPreserveAttachments = nullptr;
        m_desc.pResolveAttachments = nullptr;

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

    void VulkanRenderPass::Subpass::addColorAttachment(const VkAttachmentReference& ref)
    {
        assert(m_numAttachments[EAttachmentType::Depth] == 0 && m_numAttachments[EAttachmentType::Resolve] == 0
            && "Add attachments in order input - color - depth - resolve");

        ++m_numAttachments[EAttachmentType::Color];
        m_attachmentRefs.pushBack(ref);
    }

//=============================================================================================

    VulkanRenderPass::VulkanRenderPass(Quaint::IMemoryContext* context)
    : m_context(context)
    {
        m_attchmentInfos = Quaint::QArray<VulkanRenderPass::AttachmentInfo>(m_context);
        m_subPasses = Quaint::QArray<VulkanRenderPass::Subpass>(m_context, m_context);
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

            if(m_renderPass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(device, m_renderPass, callbacks);
            }

            QLOG_I(VulkanRenderPass, "Destroyed Vulkan Renderpass");
        }
    }

    VulkanRenderPass::AttachmentInfo& VulkanRenderPass::addEmptyAttachment()
    {
        AttachmentInfo info;
        info.index = m_attchmentInfos.getSize();
        m_attchmentInfos.pushBack(info);
        return m_attchmentInfos[info.index];
    }

    //Create subpass from this info
    VulkanRenderPass::Subpass& VulkanRenderPass::addEmptySubpass()
    {
        Subpass subpass(m_context);
        subpass.m_index = m_subPasses.getSize();
        m_subPasses.pushBack(subpass);
        return m_subPasses[subpass.m_index];
    }

    void VulkanRenderPass::addSubpassDependency(const Subpass& src, const Subpass& dst
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
    }

    void VulkanRenderPass::construct()
    {
        //TODO: Construct subpasses
        for(Subpass& subpass : m_subPasses)
        {
            subpass.construct();
        }

        VkRenderPassCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        info.flags = 0; //Not used for now. TODO: Check back later
        
        // populating Attachments information
        uint32_t numAttachments = m_attchmentInfos.getSize();
        Quaint::QArray<VkAttachmentDescription> attachDescs(m_context, numAttachments);
        if(numAttachments > 0)
        {
            for(uint32_t i = 0; i < numAttachments; ++i)
            {
                attachDescs[i] = m_attchmentInfos[i].desc;
            }
            info.attachmentCount = m_attchmentInfos.getSize();
            info.pAttachments = attachDescs.getBuffer();
        }

        //Populating subpass information
        uint32_t numSubpasses = m_subPasses.getSize();
        Quaint::QArray<VkSubpassDescription> subpassDescs(m_context, numSubpasses);
        assert(numSubpasses > 0 && "RenderPass needs atleast a single subpass!!");
        for(uint32_t i = 0; i < numSubpasses; ++i)
        {
            subpassDescs[i] = m_subPasses[i].getDescription();
        }
        info.subpassCount = numSubpasses;
        info.pSubpasses = subpassDescs.getBuffer();

        //Populating subpass dependency information
        uint32_t numDependencies = m_subPassDependencies.getSize();
        if(numDependencies > 0)
        {
            info.dependencyCount = m_subPassDependencies.getSize();
            info.pDependencies = m_subPassDependencies.getBuffer();
        }
    }
}