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
    class RenderStage;
    class Pipeline;
    class Painter;

    class IRenderer
    {
    public:
        virtual void init() = 0;
        virtual void shutdown() = 0;
        virtual void render() = 0;
        virtual ~IRenderer(){}

        //virtual IShaderGroupConstructor* getShaderGroupConstructor() = 0;

        //TODO: Implement these
        /* Adds and builds the render scenes */
        virtual void addRenderScene(Quaint::QName name, const RenderInfo& renderInfo, const uint32_t numStages, const RenderStage* pStages) = 0;
        virtual void addImmediateRenderScene(Quaint::QName name, const RenderInfo& renderInfo, const uint32_t numStages, const RenderStage* pStages) = 0;
        virtual void renderSceneImmediate(const Quaint::QName& name, Bolt::Painter* painter, uint32_t framebufferIdx = 0) = 0;
        virtual RenderScene* getRenderScene(Quaint::QName name) = 0;

        virtual void addPipeline(Bolt::Pipeline* pipeline) = 0;
        virtual Bolt::Pipeline* getPipeline(const Quaint::QName& name) = 0;
        //TODO:
        //virtual void mapBuffer() = 0;
        //virtual void unmapBuffer() = 0;
    };

    class ResourceGPUProxy
    {
    public:
        ResourceGPUProxy(Quaint::IMemoryContext* context)
        : m_context(context)
        {}
        
        virtual void destroy() = 0;
        Quaint::IMemoryContext* getMemoryContext() { return m_context; }
    
    protected:
        Quaint::IMemoryContext* m_context = nullptr;
    };

    class IRenderObjectImpl : public ResourceGPUProxy
    {
    public:
        IRenderObjectImpl(Quaint::IMemoryContext* context)
        : ResourceGPUProxy(context)
        {}

        template<typename T>
        void pushUniform(const Quaint::QName& name, T* data);
        void pushUniformBuffer(const Quaint::QName& name, void* data, uint32_t size);

        virtual void build(const GeometryRenderInfo& shaderinfo) = 0;
        virtual void draw(RenderScene* scene) = 0;
    };
}

#endif //_H_I_RENDERER