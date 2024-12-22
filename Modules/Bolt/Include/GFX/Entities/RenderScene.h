#ifndef _H_RENDER_SCENE
#define _H_RENDER_SCENE
#include <cstdint>
#include <Types/QArray.h>
#include <Types/QStaticString.h>
#include <Types/QUniquePtr.h>
#include "../Data/RenderInfo.h"
#include "../Helpers.h"

namespace Bolt
{
    class RenderSceneImpl;

    struct RenderStage
    {
        // Refers to the attachment defined in RenderInfo
        struct AttachmentRef
        {
            uint32_t        binding = ~0ul;
            Quaint::QName   attachmentName = "";
        };
        uint32_t                                index = ~0ul;
        uint32_t                                dependentStage = ~0ul;
        Quaint::QArray<AttachmentRef>           attachmentRefs; 
    };
    class RenderScene
    {
    public:
        typedef Quaint::QUniquePtr<RenderSceneImpl, Deleter<RenderSceneImpl>> TRenderScenImplPtr;
        /* Currently very limited */
        struct SceneDependency
        {
            Quaint::QName waitOn = "";
            Quaint::QName signalTo = "";
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
        _T* getRenderSceneImplAs() { return static_cast<_T*>(m_impl.get()); }

    protected:
        Quaint::IMemoryContext*                 m_context = nullptr;
        Quaint::QName                           m_name = "";
        RenderInfo                              m_renderInfo = {};
        SceneDependency                         m_dependency = {};
        Quaint::QArray<RenderStage>             m_stages;
        TRenderScenImplPtr                      m_impl;
        bool                                    m_isValid = false;
    };
}

#endif //_H_RENDER_SCENE