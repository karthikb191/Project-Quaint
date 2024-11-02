#ifndef _H_BOLT
#define _H_BOLT
#include <memory>
#include <iostream>
#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <Math/QMat.h>
#include "Core/Camera.h"

//TODO: This should be moved to Application Module
#include <GFX/Window.h>

//TODO: Move this to some place better
namespace Quaint
{
    struct UniformBufferObject
    {
        QMat4x4     model;
        QMat4x4     view;
        QMat4x4     proj;
    };
}

namespace Bolt
{
    class RenderModule;

    class BoltRenderer
    {
        friend class RenderModule;
        
        template<typename T, typename ...ARGS>
        friend T* ::allocFromContext(Quaint::IMemoryContext* context, ARGS...);
        template<typename T>
        friend void ::deleteFromContext(Quaint::IMemoryContext* context, T* mem);

    public:

        void startEngine(Quaint::IMemoryContext* context);
        void initCamera();
        void update();
        void shutdown();

        const Window& getWindow() { return m_window; }
        IRenderer* GetRenderer() { return m_renderer_impl; }
        IRenderObjectBuilder* getRenderObjectBuilder() { return m_renderObjectBuilder; }
        Camera& getCamera() { return m_camera; }
        const Quaint::UniformBufferObject& getMVPMatrix() { return m_ubo; }

        void mapBufferResource(GraphicsResource* resource, void* out);
    private:
        BoltRenderer();
        ~BoltRenderer();

        void updateUniformBufferProxy();

        IRenderer*                          m_renderer_impl         = nullptr;
        IRenderObjectBuilder*               m_renderObjectBuilder   = nullptr;
        Quaint::IMemoryContext*             m_context               = nullptr;

        Window                              m_window;
        Camera                              m_camera;
        Quaint::UniformBufferObject         m_ubo;

        bool m_engineRunning;
    };
}
#endif //_H_BOLT