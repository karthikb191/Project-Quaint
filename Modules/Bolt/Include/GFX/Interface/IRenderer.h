#ifndef _H_I_RENDERER
#define _H_I_RENDERER

#include <Interface/IMemoryContext.h>
#include "../Data/ShaderInfo.h"
#include "../Data/RenderInfo.h"
#include "IShaderGroup.h"

namespace Bolt
{
    class GraphicsResource;
    class RenderScene;

    class IRenderer
    {
    public:
        virtual void init() = 0;
        virtual void shutdown() = 0;
        virtual void render() = 0;
        virtual ~IRenderer(){}

        virtual IShaderGroupConstructor* getShaderGroupConstructor() = 0;

        //TODO: Implement these
        /* Adds and builds the render scenes */
        virtual void addRenderScene(Quaint::QName name, const RenderInfo& renderInfo) = 0;
        //TODO:
        //virtual void mapBuffer() = 0;
        //virtual void unmapBuffer() = 0;
    };

    class RenderObject;
    class IRenderObjectImpl
    {
    public:
        IRenderObjectImpl(RenderObject* ro)
        : m_renderObject(ro)
        {}

        virtual void build(const GeometryRenderInfo& shaderinfo) = 0;
        virtual void draw(const GeometryRenderInfo& info) = 0;
        virtual void destroy() = 0;

        RenderObject* getRenderObject() { return m_renderObject; }
    protected:
        RenderObject*       m_renderObject = nullptr;
    };

    class IRenderObjectBuilder
    {
    public:
        virtual IRenderObjectImpl*  buildRenderObjectImplFor(RenderObject* obj) = 0;
    };

    class ResourceGPUProxy
    {
    public:
        ResourceGPUProxy(Quaint::IMemoryContext* context)
        : m_context(context)
        {}
        
        virtual void destroy() = 0;
        Quaint::IMemoryContext* getMemoryContext() { return m_context; }
    private:
        Quaint::IMemoryContext* m_context = nullptr;
    };
}

#endif //_H_I_RENDERER