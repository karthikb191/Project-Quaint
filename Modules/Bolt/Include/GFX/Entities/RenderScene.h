#ifndef _H_RENDER_SCENE
#define _H_RENDER_SCENE
#include <cstdint>
#include <Types/QArray.h>
#include <Types/QStaticString.h>
#include "../Data/RenderInfo.h"

namespace Bolt
{
    class RenderSceneImpl;

    class RenderScene
    {
    public:
        /* Currently very limited */
        struct StageDependency
        {
            Quaint::QName waitOn = "";
            Quaint::QName signalTo = "";
        };

        struct RenderStage
        {
            uint32_t    index = ~0;
            uint32_t    dependentStage = ~0;
        };

        RenderScene(Quaint::IMemoryContext* context, Quaint::QName name, const RenderInfo& renderInfo);
        void addRenderStage(const RenderStage& stage);
        
        /* Constructs all the GPU dependencies */
        virtual bool construct();
        virtual bool begin();
        virtual bool render();
        virtual bool end();

    protected:
        Quaint::IMemoryContext*         m_context = nullptr;
        Quaint::QName                   m_name = "";
        RenderInfo                      m_renderInfo = {};
        StageDependency                 m_dependency = {};
        Quaint::QArray<RenderStage>     m_stages;
        RenderSceneImpl*                m_impl;
    };
}

#endif //_H_RENDER_SCENE