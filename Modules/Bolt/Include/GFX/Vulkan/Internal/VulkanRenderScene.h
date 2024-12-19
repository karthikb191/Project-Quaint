#ifndef _H_VULKAN_RENDER_SCENE
#define _H_VULKAN_RENDER_SCENE

#include <vulkan/vulkan.h>
#include <QMath.h>
#include <Types/QArray.h>
#include "VulkanGraphicsContext.h"
#include "VulkanFrameBuffer.h"
#include "Entities/VulkanTexture.h"
#include "../../../GFX/Data/RenderInfo.h"

namespace Bolt { 
    class RenderScene;
    class RenderSceneImpl
    {
    public:

    protected:
    };
    
    namespace vulkan{
    
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
        inline AttachmentInfo& setType(const EAttachmentType pType) { type = pType; return *this; }
        inline AttachmentInfo& setDescription(const VkAttachmentDescription pDesc) { desc = pDesc; return *this; }
        inline AttachmentInfo& setFormat(const VkFormat pFormat) { desc.format = pFormat; return *this;}
        inline AttachmentInfo& setSamples(const VkSampleCountFlagBits pSamples) { desc.samples = pSamples; return *this;}

        //TODO: Maybe Ops and Layout info is not necessary here. This is render-pass specific logic. Could probably moved there
        inline AttachmentInfo& setOps(const VkAttachmentLoadOp pLoadOp, const VkAttachmentStoreOp pStoreOp) 
        {
            desc.loadOp = pLoadOp; desc.storeOp = pStoreOp; return *this;
        }
        inline AttachmentInfo& setStencilOps(const VkAttachmentLoadOp pLoadOp, const VkAttachmentStoreOp pStoreOp)
        {
            desc.stencilLoadOp = pLoadOp; desc.stencilStoreOp = pStoreOp; return *this;
        }
        inline AttachmentInfo& setLayout(const VkImageLayout pInitialLayout, const VkImageLayout pFinalLayout)
        {
            desc.initialLayout = pInitialLayout; desc.finalLayout = pFinalLayout; return *this;
        }
        inline AttachmentInfo& setExtent(const VkExtent3D& pExtent) { extent = pExtent; return *this; }
        inline AttachmentInfo& setMemoryPropertyFlags(const VkMemoryPropertyFlags pFlags) { memoryPropertyFlags = pFlags; return *this; }
        inline AttachmentInfo& setImageUsage(const VkImageUsageFlags pFlags) { imageUsage = pFlags; return *this; }

        /* Image view is constructed from a few settings above and the ones below*/
        inline AttachmentInfo& setImageViewType(const VkImageViewType pType) { viewType = pType; return*this; }
        inline AttachmentInfo& setImageViewFormat(const VkFormat pFormat) { viewFormat = pFormat; return*this; }
        inline AttachmentInfo& setImageViewComponentMapping(const VkComponentMapping& pMapping) { viewComponentMapping = pMapping; return*this; }
        inline AttachmentInfo& setImageViewSubresourceRange(const VkImageSubresourceRange& pRange) { subresourceRange = pRange; return*this; }
        inline AttachmentInfo& setIsSwapchainImage(bool pIsSwapChainImage) { isSwapchainImage = pIsSwapChainImage; return*this; }
 
        inline uint32_t getIndex() const { return index; }
        inline const RenderScene* getScene() const { return scene; }
        inline const EAttachmentType getType() const { return type; }
        inline const VkFormat getFormat() const { return desc.format; }
        inline const VkSampleCountFlagBits getSampleCount() const { return desc.samples; }
        inline const VkExtent3D& getExtent() const { return extent; }
        inline const VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
        inline const VkImageViewType getImageViewType() const { return viewType; }
        inline const VkFormat getImageViewFormat() const { return viewFormat; }
        inline const VkComponentMapping& getImageViewComponentMapping() const { return viewComponentMapping; }
        inline const VkImageSubresourceRange& getImageViewSubresourceRange() const { return subresourceRange; }
        inline bool getIsSwapchainImage() const { return isSwapchainImage; }
        inline const VkImageUsageFlags getImageUsage() const { return imageUsage; }

        inline const VkAttachmentDescription& getDescription() const { return desc; }
        inline RenderScene& finalizeAttachmentInfo() { return *scene; }

    private:
        friend class Quaint::QArray<AttachmentInfo>;
        friend class RenderScene;
        AttachmentInfo() = default;
        
        EAttachmentType             type = EAttachmentType::Max;
        VkAttachmentDescription     desc = { };
        uint32_t                    index = -1;
        RenderScene*                scene = nullptr;
        VkExtent3D                  extent = {512, 512, 1};
        bool                        isSwapchainImage = false;

        //View specific information. Framebuffer is setup based on this
        VkMemoryPropertyFlags       memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkImageViewType             viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkFormat                    viewFormat = VK_FORMAT_R8G8B8A8_SRGB;
        VkComponentMapping          viewComponentMapping = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };
        VkImageSubresourceRange     subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        VkImageUsageFlags           imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    };

    struct Attachment
    {
        Attachment(const Bolt::AttachmentDefinition& info)
        {
            buildAttachmentDescription();
        };
        template<typename _T>
        _T* As() { return static_cast<_T*>(this); }
        
        const AttachmentDefinition& getInfo() const { return info; }
        const VkAttachmentDescription& getAttachmentDescription() const { return attachmentDescription; }
        const VkAttachmentReference& getAttachmentReference() const { return attachmentReference; } 

    protected:
        virtual void buildAttachmentDescription() = 0;
        virtual void buildAttachmentReference() = 0;
        
        VkAttachmentDescription attachmentDescription = {};
        VkAttachmentReference attachmentReference  = {};
        AttachmentDefinition info;
    };
    struct ImageAttachment : public Attachment
    {
        ImageAttachment(const Bolt::AttachmentDefinition& info, VulkanTexture tex)
        : Attachment(info)
        , texture(tex)
        {}
        virtual void buildAttachmentReference() override;
        virtual void buildAttachmentDescription() override;

        VulkanTexture texture;
    };
    struct DepthAttachment : public Attachment
    {
        DepthAttachment(const Bolt::AttachmentDefinition& info)
        : Attachment(info)
        {}
        virtual void buildAttachmentReference() override {}
        virtual void buildAttachmentDescription() override {}
        //TODO:
    };
    struct SwapchainAttachment : public Attachment
    {
        SwapchainAttachment(const Bolt::AttachmentDefinition& info)
        : Attachment(info)
        {}
        VulkanSwapchain* getSwapchain() { return VulkanRenderer::get()->getSwapchain(); }
        virtual void buildAttachmentReference() override;
        virtual void buildAttachmentDescription() override;
    };

    class VulkanRenderScene : public Bolt::RenderSceneImpl
    {
    public:
        VulkanRenderScene(Quaint::IMemoryContext* context);
        virtual ~VulkanRenderScene() noexcept = default;
        virtual void construct(const Bolt::RenderScene* scene);
        virtual void destroy();
    
        virtual bool begin() = 0;
        virtual void end() = 0;
        virtual void submit() = 0;

        GraphicsContext*    getContext() { return &m_graphicsContext; }
        
        AttachmentInfo&     beginAttachmentSetup();

        //const Quaint::QArray<AttachmentInfo>& getAttachmentInfos() const { return m_attchmentInfos; }
        const Quaint::QArray<Quaint::QUniquePtr<Attachment>>& getAttachments() const { return m_attachments; }

    protected:
        void constructAttachments(const Bolt::RenderInfo& info);
        VulkanTexture constructVulkanTexture(const Bolt::AttachmentDefinition def);

        void constructSubpasses(const Bolt::RenderScene* scene);
        virtual void buildFrameBuffer() = 0;

        Quaint::IMemoryContext*                                     m_context = nullptr;
        GraphicsContext                                             m_graphicsContext;
        VkCommandPool                                               m_commandPool;
        Quaint::QArray<Quaint::QUniquePtr<Attachment>>              m_attachments;
        Quaint::QArray<VkSubpassDescription>                        m_subpassDesc;
        Quaint::QArray<VkSubpassDependency>                         m_subpassDependencies;                     
    };

    struct MVP
    {
        Quaint::QMat4x4     model;
        Quaint::QMat4x4     view;
        Quaint::QMat4x4     proj;
    };

    struct FrameInfo
    {
        MVP                         mvpUBO;
        VkCommandBuffer             commandBuffer = VK_NULL_HANDLE;
        VkFence                     renderFence = VK_NULL_HANDLE;
        VkSemaphore                 scImageAvailableSemaphore = VK_NULL_HANDLE;
        VkSemaphore                 renderFinishedSemaphore = VK_NULL_HANDLE;
    };

    /* Renders to a swapchain */
    class RenderFrameScene : public VulkanRenderScene
    {
    public:
        RenderFrameScene(Quaint::IMemoryContext* context, const uint32_t framesInFlight = 1);
        virtual ~RenderFrameScene() noexcept {}
        
        /* Should be called once all the required information has been set*/
        virtual void construct() override;

        virtual bool begin() override;
        virtual void end() override;
        virtual void submit() override;

        VulkanRenderPass& editRenderPassSetup() { return m_renderPass; }
        void setupSwapchain();
        VulkanRenderPass* getRenderPass() { return &m_renderPass; }
        const VkExtent2D& getSwapchainExtent() const { return m_swapchainExtent; }
        FrameInfo& getCurrentFrameInfo() { return m_frameInfo[m_currentFrame]; }
    private:

        virtual void destroy() override;

        //RenderFrameScene& buildCommandPool();
        // Let Graphics context build command pool. If we need a new command buffer, we can get the pool from context
        void buildGraphicsContext(); 
        virtual void buildFrameBuffer() override;
        void setupFrameInfo();

        uint32_t getNextFrame();
        
        Quaint::QArray<FrameInfo>                   m_frameInfo;
        Quaint::QArray<FrameBuffer>                 m_framebuffers;
        uint8_t                                     m_nextFrameIndex = 0;
        uint32_t                                    m_currentImageIndex = 0;
        const uint8_t                               m_framesInFlight = 1;
        VulkanRenderPass                            m_renderPass;
        VkSwapchainKHR                              m_swapchain;
        VkSwapchainKHR                              m_outOfDateSwapchain;
        uint32_t                                    m_currentFrame = -1;
        VkExtent2D                                  m_swapchainExtent = {};
    };
}}

#endif //_H_VULKAN_RENDER_SCENE