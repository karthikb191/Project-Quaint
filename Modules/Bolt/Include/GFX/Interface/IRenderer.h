#ifndef _H_I_RENDERER
#define _H_I_RENDERER

#include <Interface/IMemoryContext.h>
#include "../Data/ShaderInfo.h"
#include "IShaderGroup.h"

namespace Bolt
{
    class IRenderer
    {
    public:
        virtual void init() = 0;
        virtual void shutdown() = 0;
        virtual void render() = 0;
        virtual ~IRenderer(){}

        virtual IShaderGroupConstructor* getShaderGroupConstructor() = 0;
        
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

        virtual void build(const ShaderInfo& shaderinfo) = 0;
        virtual void draw() = 0;

        RenderObject* getRenderObject() { return m_renderObject; }
    protected:
        RenderObject*       m_renderObject = nullptr;
    };

    class IRenderObjectBuilder
    {
    public:
        virtual IRenderObjectImpl*  buildRenderObjectImplFor(RenderObject* obj) = 0;
    };
}

#endif //_H_I_RENDERER