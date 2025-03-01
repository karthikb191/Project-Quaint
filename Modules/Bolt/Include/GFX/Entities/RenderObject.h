#ifndef _H_BOLT_RENDER_OBJECT
#define _H_BOLT_RENDER_OBJECT

#include <stdint.h>
#include <cstddef>
#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>
#include <QMath.h>
#include "../Interface/IRenderer.h"
#include "../Interface/IShaderGroup.h"
#include "../Data/ShaderInfo.h"

namespace Bolt
{
    // class RenderObject
    // {
    // public:
    //     RenderObject(Quaint::IMemoryContext* context)
    //     : m_context(context)
    //     {}
    //     virtual void load() = 0;
    //     virtual void draw() = 0;
    //     virtual void destroy() = 0;
    //     virtual void setShaderGroup(IShaderGroup* shaderGroup) { m_shaderGroup = shaderGroup; }
    //     Quaint::IMemoryContext* getMemoryContext() { return m_context; }
    //     const IShaderGroup* getShaderGroup() { return m_shaderGroup; }

    //     //TODO: Remove all of these once testing is done


    // protected:
    //     Quaint::IMemoryContext* m_context = nullptr;
    //     IShaderGroup* m_shaderGroup = nullptr;
    //     IRenderObjectImpl* m_impl = nullptr;
    //     //TODO: Add a GPU-backed object to give renderer necessary information
    // };

    // class Geometry : public RenderObject
    // {
    
    // };

    // class RenderQuad : public Geometry
    // {
    // public:
    //     RenderQuad(Quaint::IMemoryContext* context);
    //     virtual void load();
    //     virtual void draw() override;
    //     virtual void destroy() override;

    //     //TODO: Remove this
    //     //void drawTemp(vulkan::RenderFrameScene* context);
    // };

    /* This should be client facing */
    /* Model is basically a resource */
    
}

#endif //_H_BOLT_RENDER_OBJECT