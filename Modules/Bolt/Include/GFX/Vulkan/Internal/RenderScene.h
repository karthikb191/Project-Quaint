#ifndef _H_VULKAN_RENDER_SCENE
#define _H_VULKAN_RENDER_SCENE

#include <vulkan/vulkan.h>
#include <QMath.h>
#include <Types/QArray.h>
#include "VulkanGraphicsContext.h"
#include "VulkanFrameBuffer.h"

namespace Bolt { namespace vulkan{
    

    class RenderScene
    {
    protected:
        RenderScene(Quaint::IMemoryContext* context);
    
    public:
        void begin();
        void end();

        GraphicsContext*    getContext() { return &m_graphicsContext; }

    protected:
        Quaint::IMemoryContext*         m_context = nullptr;
        GraphicsContext                 m_graphicsContext;
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
        VkSwapchainKHR              swapchain;
    };

    /* Renders to a swapchain */
    class RenderFrameScene : public RenderScene
    {
        RenderFrameScene(Quaint::IMemoryContext* context, const uint32_t framesInFlight = 1);



    private:
        uint32_t getNextFrame();
        
        Quaint::QArray<FrameInfo>                   m_frameInfo;
        uint8_t                                     m_nextFrameIndex = 0;
        const uint8_t                               m_framesInFlight = 1;
    };
}}

#endif //_H_VULKAN_RENDER_SCENE