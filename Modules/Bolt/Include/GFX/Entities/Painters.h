#ifndef _H_BOLT_PAINTERS
#define _H_BOLT_PAINTERS

#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>
#include <Types/QStaticString.h>

namespace Bolt
{
    class Pipeline;
    class Model;
    class RenderScene;
    class Painter 
    {
    public:
        Painter::Painter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline)
        : m_context(context)
        , m_pipeline(pipeline)
        {}

        virtual void preRender(RenderScene* scene, uint32_t stage) = 0;
        virtual void postRender() = 0;
        virtual void render(RenderScene* scene) = 0;

        const Quaint::QName& getPipelineName() const { return m_pipeline; }

    protected:
        Quaint::IMemoryContext* m_context = nullptr;
        Quaint::QName m_pipeline = "";
    };

    /* Geometry pipeline doesn't own models. Calling code should ensure the model is not destroyed as it's being used by the pipeline*/
    class GeometryPainter : public Painter
    {
    public:
        GeometryPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline);
        virtual void render(RenderScene* scene) override;
        virtual void preRender(RenderScene* scene, uint32_t stage) override;
        virtual void postRender() override;

        void AddModel(Model* model);

    private:
        Pipeline*   m_pipeline = nullptr;
        Quaint::QArray<Model*> m_models = nullptr;
    };
}

#endif //_H_BOLT_PAINTERS