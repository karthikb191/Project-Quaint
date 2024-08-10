#ifndef _H_VULKAN_RENDER_PASS
#define _H_VULKAN_RENDER_PASS

#include <Vulkan/vulkan_core.h>
#include <Types/QArray.h>

namespace Bolt
{
    class VulkanRenderPass;

    class VulkanRenderPass
    {
    public:
        enum EAttachmentType
        {
            Input = 0,
            Color = 1,
            Depth = 2,
            Resolve = 3,
            Max = 4
        };

    private:
        struct AttachmentInfo
        {
            EAttachmentType             type = EAttachmentType::Max;
            VkAttachmentDescription     desc = { };

            uint32_t getIndex() { return index; }
        private:
            friend class VulkanRenderPass;
            uint32_t                    index = -1;
        };

        //TODO: Is this class really necessary?
        class Subpass
        {
        public:
            Subpass(Quaint::IMemoryContext* context);

            void construct();
            
            // Add attachments in sequence. Do NOT mix them up. 
            // TODO: Add checks validating this
            void addColorAttachment(const VkAttachmentReference& ref);

            //TODO: Add more attachment types

            const VkSubpassDescription& getDescription() const { return m_desc; }
            uint32_t getIndex() const { return m_index; }

        private:
            Subpass() {}

            friend class VulkanRenderPass;
            uint32_t                                m_index = (~0U);
            VkSubpassDescription                    m_desc;
            VkPipelineBindPoint                     m_pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            uint32_t                                m_numAttachments[EAttachmentType::Max];
            Quaint::QArray<VkAttachmentReference>   m_attachmentRefs;
        };

    public:

        struct AttachmentReferenceInfo
        {
            const AttachmentInfo&       attachmentInfo;
            uint32_t                    index; /* Index into the subpass */
            EAttachmentType             type;
            VkImageLayout               layout;
        };

        VulkanRenderPass(Quaint::IMemoryContext* context);
        virtual ~VulkanRenderPass();
        void destroy();
        //TODO: Add input and other output attachments

        /*Returns and empty AttachmentInfo that should be filled*/
        AttachmentInfo& addEmptyAttachment();
        /* Returns an empty SubPass that needs to be filled appropriately */
        Subpass& addEmptySubpass();

        void addSubpassDependency(const Subpass& src, const Subpass& dst
                                , VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask
                                , VkAccessFlags srcFlag, VkAccessFlags dstFlag
                                , VkDependencyFlags dependencyFlags);

        void construct();

        bool isValid() const { return m_renderPass != VK_NULL_HANDLE; }
        bool isBeingUsed () const { return m_beingUsed; }

    private:

        Quaint::IMemoryContext*                 m_context = nullptr;
        VkRenderPass                            m_renderPass = VK_NULL_HANDLE;
        bool                                    m_beingUsed = false;
        Quaint::QArray<AttachmentInfo>          m_attchmentInfos;
        Quaint::QArray<Subpass>                 m_subPasses;
        Quaint::QArray<VkSubpassDependency>     m_subPassDependencies;
        static const VulkanRenderPass::Subpass  SUBPASS_EXTERNAL;
    };


    //TODO: Should this be here?
    class VulkanFrameBuffer
    {

    };
}

#endif //_H_VULKAN_RENDER_PASS