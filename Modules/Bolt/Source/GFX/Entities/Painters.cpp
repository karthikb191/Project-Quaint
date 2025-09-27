#include <GFX/Entities/Painters.h>
#include <GFX/Entities/Model.h>

namespace Bolt
{
    GeometryPainter::GeometryPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline)
    : Painter(context, pipeline)
    , m_models(context)
    {
    }

    void GeometryPainter::preRender(RenderScene* scene, uint32_t stage)
    {

        //TODO:
    }
    void GeometryPainter::render(RenderScene* scene)
    {
        for(auto& model : m_models)
        {
            model->draw(scene);
            //TODO:
        }
    }
    void GeometryPainter::postRender()
    {
        //TODO:
    }

    void GeometryPainter::AddModel(Model* model)
    {
        m_models.pushBack(model);
    }


    //IMGGUI Painter
    ImguiPainter::ImguiPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline)
    : Painter(context, pipeline)
    {
        
    }
    void ImguiPainter::render(RenderScene* scene)
    {

    }
    void ImguiPainter::preRender(RenderScene* scene, uint32_t stage)
    {

    }
    void ImguiPainter::postRender()
    {

    }

}