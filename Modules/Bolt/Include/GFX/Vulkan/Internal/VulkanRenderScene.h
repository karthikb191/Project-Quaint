#ifndef _H_VULKAN_RENDER_SCENE
#define _H_VULKAN_RENDER_SCENE

#include <vulkan/vulkan.h>
#include <QMath.h>
#include <Types/QArray.h>
#include "VulkanGraphicsContext.h"
#include "VulkanFrameBuffer.h"
#include "Entities/VulkanTexture.h"
#include "../../../GFX/Data/RenderInfo.h"
#include "GFX/Helpers.h"

namespace Bolt { 
    class RenderScene;
    class RenderSceneImpl
    {
    public:

    protected:
    };
    
    namespace vulkan{

    class VulkanSwapchain;

    struct Attachment
    {
        Attachment(const Bolt::AttachmentDefinition& pInfo);
        template<typename _T>
        _T* As() { return static_cast<_T*>(this); }
        
        void construct();
        const AttachmentDefinition& getInfo() const { return info; }
        const VkAttachmentDescription& getAttachmentDescription() const { return attachmentDescription; }
        const VkAttachmentReference& getAttachmentReference() const { return attachmentReference; } 

    protected:
        virtual void buildAttachmentDescription()
        {
            assert(false && "This shouldn't hit");
        }
        virtual void buildAttachmentReference()
        {
            assert(false && "This shouldn't hit");
        }
        
        VkAttachmentDescription attachmentDescription = {};
        VkAttachmentReference attachmentReference  = {};
        AttachmentDefinition info;
    };
    struct ImageAttachment : public Attachment
    {
        ImageAttachment(const Bolt::AttachmentDefinition& info, VulkanTexture tex)
        : Attachment(info)
        , texture(tex)
        {
        }
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
        {
        }
        VulkanSwapchain* getSwapchain();
        virtual void buildAttachmentReference() override;
        virtual void buildAttachmentDescription() override;
    };

    struct SubpassDescription
    {
        Quaint::QArray<VkAttachmentReference> references;
        VkSubpassDescription description;
    };

    class VulkanRenderScene : public Bolt::RenderSceneImpl
    {
    public:
        typedef Quaint::QUniquePtr<FrameBuffer, Bolt::Deleter<FrameBuffer>> TFramebufferPtr;
        typedef Quaint::QUniquePtr<Attachment, Bolt::Deleter<Attachment>> TAttachmentPtr;
        typedef Quaint::QArray<TAttachmentPtr> TAttachmentArray ;

        struct SceneParams
        {
            VkSemaphore renderFinishedSemaphore;
            VkFence renderFence;
            VkCommandBuffer commandBuffer;
        };

        VulkanRenderScene(Quaint::IMemoryContext* context);
        virtual ~VulkanRenderScene() noexcept = default;
        virtual void construct(const Bolt::RenderScene* scene);
        virtual void destroy();
    
        virtual bool begin();
        virtual SceneParams end(VkQueue queue);
        virtual void submit(VkQueue queue);

        GraphicsContext*    getContext() { return &m_graphicsContext; }
        VkRenderPass getRenderpass() const { return m_renderpass; }
        FrameBuffer* getFrameBuffer() { return m_framebuffer.get(); }

        AttachmentInfo&     beginAttachmentSetup();
        Attachment* getAttachment(const Quaint::QName& name);
        const TAttachmentArray& getAttachments() const { return m_attachments; }
        
        const SceneParams& getSceneParams() const { return m_sceneParams; }
        const VkExtent2D& getRenderExtent() const { return m_renderExtent; }
        const VkOffset2D& getRenderOffset() const { return m_renderOffset; }
    protected:
        void constructAttachments(const Bolt::RenderInfo& info);
        VulkanTexture constructVulkanTexture(const Bolt::AttachmentDefinition def);

        void constructSubpasses(const Bolt::RenderScene* scene);
        void constructFrameBuffer();

        Quaint::IMemoryContext*                                     m_context = nullptr;
        GraphicsContext                                             m_graphicsContext;
        //VkCommandPool                                               m_commandPool;
        VkRenderPass                                                m_renderpass;
        TAttachmentArray                                            m_attachments;
        Quaint::QArray<SubpassDescription>                          m_subpassDesc;
        Quaint::QArray<VkSubpassDependency>                         m_subpassDependencies;
        TFramebufferPtr                                             m_framebuffer;
        SceneParams                                                 m_sceneParams = {};
        VkExtent2D                                                  m_renderExtent = {512, 512};
        VkOffset2D                                                  m_renderOffset = {0, 0};
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
}}

#endif //_H_VULKAN_RENDER_SCENE