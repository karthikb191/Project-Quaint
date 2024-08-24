#ifndef _H_VULKAN_RENDER_PASS
#define _H_VULKAN_RENDER_PASS

#include <Vulkan/vulkan_core.h>
#include <Types/QArray.h>

namespace Bolt { namespace vulkan 
{
    class VulkanRenderPass;
    class RenderScene;

    /* TODO: Should take a reference to FrameBuffer to create attachments*/
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
        struct AttachmentInfo
        {
            void setType(const EAttachmentType pType) { type = pType; }
            void setDescription(const VkAttachmentDescription pDesc) { desc = pDesc; }
            uint32_t getIndex() { return index; }
        private:
            friend class Quaint::QArray<AttachmentInfo>;
            friend class VulkanRenderPass;
            AttachmentInfo() = default;
            
            EAttachmentType             type = EAttachmentType::Max;
            VkAttachmentDescription     desc = { };
            uint32_t                    index = -1;
        };

        //TODO: Is this class really necessary?
        class Subpass
        {
        public:
            // Add attachments in sequence. Do NOT mix them up. 
            // TODO: Add checks validating this
            Subpass& addColorAttachment(const VkAttachmentReference& ref);
            Subpass& addColorAttachment(const uint32_t attachIndex, const VkImageLayout layout);

            //TODO: Add more attachment types

            const VkSubpassDescription& getDescription() const { return m_desc; }
            uint32_t getIndex() const { return m_index; }

        private:
            Subpass(Quaint::IMemoryContext* context);
            Subpass() {}

            void construct();
            void destroy();

            friend class Quaint::QArray<Subpass>;
            friend class VulkanRenderPass;
            uint32_t                                m_index = (~0U);
            VkSubpassDescription                    m_desc = { };
            VkPipelineBindPoint                     m_pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            uint32_t                                m_numAttachments[EAttachmentType::Max] = {0};
            Quaint::QArray<VkAttachmentReference>   m_attachmentRefs;
        };

        struct AttachmentReferenceInfo
        {
            const AttachmentInfo&       attachmentInfo;
            uint32_t                    index; /* Index into the subpass */
            EAttachmentType             type;
            VkImageLayout               layout;
        };

    public:
        VulkanRenderPass(Quaint::IMemoryContext* context, RenderScene* scene = nullptr);
        virtual ~VulkanRenderPass();
        void destroy();
        //TODO: Add input and other output attachments

        /*Returns and empty AttachmentInfo that should be filled*/
        AttachmentInfo& addEmptyAttachment();
        /* Returns an empty SubPass that needs to be filled appropriately */
        Subpass& addEmptySubpass();
        Subpass& beginSubpassSetup();

        VulkanRenderPass& addSubpassDependency(const Subpass& src, const Subpass& dst
                                , VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask
                                , VkAccessFlags srcFlag, VkAccessFlags dstFlag
                                , VkDependencyFlags dependencyFlags);

        void construct();
        void constructFromScene(const RenderScene* scene);

        const VkRenderPass getRenderPass() { return m_renderPass; }
        bool isValid() const { return m_renderPass != VK_NULL_HANDLE; }
        bool isBeingUsed () const { return m_beingUsed; }

        
        static const VulkanRenderPass::Subpass  SUBPASS_EXTERNAL;
    
    private:
        Quaint::IMemoryContext*                 m_context = nullptr;
        RenderScene*                            m_scene = nullptr;
        VkRenderPass                            m_renderPass = VK_NULL_HANDLE;
        bool                                    m_beingUsed = false;
        Quaint::QArray<AttachmentInfo>          m_attchmentInfos;
        Quaint::QArray<Subpass>                 m_subPasses;
        Quaint::QArray<VkSubpassDependency>     m_subPassDependencies;
    };

    using SAttachmentInfo = VulkanRenderPass::AttachmentInfo;
    using CSubpass = VulkanRenderPass::Subpass;
    #define SUBPASS_EXTERNAL VulkanRenderPass::SUBPASS_EXTERNAL
}}

#endif //_H_VULKAN_RENDER_PASS