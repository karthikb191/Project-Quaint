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
            // Refers to the attachment defined in RenderInfo
            struct AttachmentRef
            {
                uint32_t        binding = ~0;
                Quaint::QName   attachmentName = "";
            };
            uint32_t                                index = ~0;
            uint32_t                                dependentStage = ~0;
            Quaint::QArray<AttachmentRef>           attachmentRefs; 
        };

        RenderScene(Quaint::IMemoryContext* context, Quaint::QName name, const RenderInfo& renderInfo);
        virtual ~RenderScene();
        void addRenderStage(const RenderStage& stage);
        
        /* Constructs all the GPU dependencies */
        virtual bool construct();
        virtual void destroy();
        virtual bool begin();
        virtual bool render();
        virtual bool end();

        const Quaint::QName& getName() const { return m_name; }
        const RenderInfo& getRenderInfo() const { return m_renderInfo; }
        const Quaint::QArray<RenderStage>& getRenderStages() const { return m_stages; }
        template<typename _T>
        _T* getRenderSceneImplAs() { return static_cast<_T*>(m_impl); }

    protected:
        Quaint::IMemoryContext*                 m_context = nullptr;
        Quaint::QName                           m_name = "";
        RenderInfo                              m_renderInfo = {};
        StageDependency                         m_dependency = {};
        Quaint::QArray<RenderStage>             m_stages;
        Quaint::QUniquePtr<RenderSceneImpl>     m_impl = nullptr;
        bool                                    m_isValid = false;
    };
}

#endif //_H_RENDER_SCENE