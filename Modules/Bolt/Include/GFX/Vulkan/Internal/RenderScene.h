#ifndef _H_VULKAN_RENDER_SCENE
#define _H_VULKAN_RENDER_SCENE

#include <vulkan/vulkan.h>
#include <QMath.h>
#include <Types/QArray.h>
#include "VulkanGraphicsContext.h"
#include "VulkanFrameBuffer.h"
#include "Texture/VulkanTexture.h"

namespace Bolt { namespace vulkan{
    
    class RenderScene;

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
 
        inline uint32_t getIndex() const { return index; }
        inline const RenderScene* getScene() const { return scene; }
        inline const EAttachmentType getType() const { return type; }
        inline const VkFormat getFormat() const { return desc.format; }
        inline const VkSampleCountFlagBits getSampleCount() { return desc.samples; }

        inline VkAttachmentDescription& getDescription() { return desc; }
        inline RenderScene& finalizeAttachmentInfo() { return *scene; }

    private:
        friend class Quaint::QArray<AttachmentInfo>;
        friend class RenderScene;
        AttachmentInfo() = default;
        
        EAttachmentType             type = EAttachmentType::Max;
        VkAttachmentDescription     desc = { };
        uint32_t                    index = -1;
        RenderScene*                scene = nullptr;
    };

    class RenderScene
    {
    protected:
        RenderScene(Quaint::IMemoryContext* context);
        virtual ~RenderScene() noexcept = default;
        virtual void destroy() = 0;

    public:
    
        void begin();
        void end();

        GraphicsContext*    getContext() { return &m_graphicsContext; }
        
        AttachmentInfo&     beginAttachmentSetup();


    protected:
        virtual RenderScene& buildFrameBuffer() = 0;

        Quaint::IMemoryContext*                 m_context = nullptr;
        GraphicsContext                         m_graphicsContext;
        VkCommandPool                           m_commandPool;
        Quaint::QArray<AttachmentInfo>          m_attchmentInfos;
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
        FrameBuffer                 frameBuffer;
        VkCommandBuffer             commandBuffer;
        Bolt::VulkanTexture         texture;
    };

    /* Renders to a swapchain */
    class RenderFrameScene : public RenderScene
    {
    public:
        RenderFrameScene(Quaint::IMemoryContext* context, const uint32_t framesInFlight = 1);
        virtual ~RenderFrameScene() noexcept {}
        
        /* Should be called once all the required information has been set*/
        void construct();

    private:

        virtual void destroy() override;

        //RenderFrameScene& buildCommandPool();
        // Let Graphics context build command pool. If we need a new command buffer, we can get the pool from context
        RenderFrameScene& buildGraphicsContext(); 
        virtual RenderFrameScene& buildFrameBuffer() override;

        uint32_t getNextFrame();
        
        Quaint::QArray<FrameInfo>                   m_frameInfo;
        uint8_t                                     m_nextFrameIndex = 0;
        const uint8_t                               m_framesInFlight = 1;
        VkSwapchainKHR                              m_swapchain;
    };
}}

#endif //_H_VULKAN_RENDER_SCENE