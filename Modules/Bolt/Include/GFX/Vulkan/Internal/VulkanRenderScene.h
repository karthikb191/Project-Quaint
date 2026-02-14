#ifndef _H_VULKAN_RENDER_SCENE
#define _H_VULKAN_RENDER_SCENE

#include <vulkan/vulkan.h>
#include <QMath.h>
#include <Types/QArray.h>
#include <Types/QVector.h>
#include "VulkanGraphicsContext.h"
#include "VulkanFrameBuffer.h"
#include "Entities/VulkanTexture.h"
#include "../../../GFX/Data/RenderInfo.h"

namespace Bolt { 
    class RenderScene;
    class RenderSceneImpl
    {
    public:
        virtual ~RenderSceneImpl(){}

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
        DepthAttachment(const Bolt::AttachmentDefinition& info, VulkanTexture tex)
        : Attachment(info)
        , texture(tex)
        {}

        virtual void buildAttachmentReference() override;
        virtual void buildAttachmentDescription() override;

        VulkanTexture texture;
    };
    struct CubemapAttachment : public Attachment
    {
        CubemapAttachment(Quaint::IMemoryContext* context, const Bolt::AttachmentDefinition& info, VulkanTexture tex)
        : Attachment(info)
        , m_context(context)
        , texture(tex)
        , cubemapViews(context)
        {}
        
        virtual void buildAttachmentReference() override;
        virtual void buildAttachmentDescription() override;
        void addImageView(int layer, int count);

        VulkanTexture texture;
        Quaint::QArray<VkImageView> cubemapViews;
    private:
        Quaint::IMemoryContext* m_context;
    };
    //This holds a reference to the depth map. This is then converted to shader input format
    struct ShadowMapAttachment : public Attachment
    {
        ShadowMapAttachment(const Bolt::AttachmentDefinition& info, VulkanTexture tex)
        : Attachment(info)
        , texture(tex)
        {}

        virtual void buildAttachmentReference() override;
        virtual void buildAttachmentDescription() override;

        VulkanTexture texture;
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
        Quaint::QArray<VkAttachmentReference> colorAttachReferences;
        Quaint::QArray<VkAttachmentReference> inputAttachReferences;
        bool hasDepthAttachment = false;
        VkAttachmentReference depthAttachment;
    };

    class VulkanRenderScene : public Bolt::RenderSceneImpl
    {
    public:
        typedef Quaint::QUniquePtr<FrameBuffer, Quaint::Deleter<FrameBuffer>> TFramebufferPtr;
        typedef Quaint::QUniquePtr<Attachment, Quaint::Deleter<Attachment>> TAttachmentPtr;
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
    
        virtual bool start();
        virtual bool beginRenderPass(uint32_t framebufferIdx = 0);
        virtual void finishSubpass();
        virtual SceneParams end();
        virtual void submit(VkQueue queue);

        GraphicsContext*    getContext() { return &m_graphicsContext; }
        VkRenderPass getRenderpass() const { return m_renderpass; }
        FrameBuffer* getFrameBuffer() { return m_framebuffer.get(); }

        AttachmentInfo&     beginAttachmentSetup();
        Attachment* getAttachment(const Quaint::QName& name);
        const Attachment* const getAttachment(const Quaint::QName& name) const;
        const TAttachmentArray& getAttachments() const { return m_attachments; }
        
        const SceneParams& getSceneParams() const { return m_sceneParams; }
        const VkExtent2D& getRenderExtent() const { return m_renderExtent; }
        const VkOffset2D& getRenderOffset() const { return m_renderOffset; }
        
        bool skipSemaphores() { m_skipSemaphores = true; }

    protected:
        virtual void constructAttachments(const Bolt::RenderInfo& info);
        VulkanTexture constructVulkanTexture(const Bolt::AttachmentDefinition def);
        VulkanTexture constructDepthTexture(const Bolt::AttachmentDefinition def);

        virtual void constructSubpasses(const Bolt::RenderScene* scene);
        virtual void constructFrameBuffer();

        
        Attachment* getAttachment_internal(const Quaint::QName& name) const;

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
        bool                                                        m_hasDepth = false;
        uint32_t                                                    m_currentSubpass = 0;
        
        bool                                                        m_skipSemaphores = false;
    };

    //Has support to render to cubemap
    class VulkanCubeMapRenderScene : public VulkanRenderScene
    {
    public:
        VulkanCubeMapRenderScene(Quaint::IMemoryContext* context);
        void setCubemapFaceToRender(uint8_t face) { m_renderToFace = face; }

    private:
        virtual void constructAttachments(const Bolt::RenderInfo& info) override;
        virtual void constructFrameBuffer() override;
        
        VulkanTexture constructCubemapTexture(const Bolt::AttachmentDefinition def);

        uint8_t m_renderToFace = 0;

        Quaint::QVector<VkImageView> m_cubemapViews;
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