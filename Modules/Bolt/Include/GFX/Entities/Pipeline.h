#ifndef _H_PIPELINE
#define _H_PIPELINE

#include <Interface/IMemoryContext.h>
#include <GFX/Interface/IEntityInterfaces.h>
#include "../Data/ShaderInfo.h"
#include "./Resources.h"

namespace Bolt
{
    class RenderScene;

    class GPUDataDispatcher
    {
    public:
        GPUDataDispatcher(Quaint::IMemoryContext*);

        template<typename T>
        void pushUniform(const Quaint::QName& name, const T& data)
        {
            //How to send this over to vulkan API?
        }
        void pushUniformBufferData(const Quaint::QName& name, void* data, const uint32_t size)
        {
            //Same question
        }

    private:
        Quaint::IMemoryContext* m_context;
    };

    /* Pipeline is an API specific resource and needs the respective implementation */
    class Pipeline : public IGFXEntity
    {
    public:
        struct BlendInfo
        {

        };

        struct PipelineInputInfo
        {

        };

        Pipeline(Quaint::IMemoryContext* context, const Quaint::QName& name, const Quaint::QName& renderScene, const uint32_t stageIdx, const ShaderDefinition& shaderDef);
        //Requires API specific implementation
        virtual void construct() override;
        virtual void destroy() override;
        
        template<typename T>
        T* GetPipelineImplAs(){ return static_cast<T*>(m_pipelineImpl.get()); }
        template<typename T>
        const T* GetPipelineImplAs() const { return static_cast<T*>(m_pipelineImpl.get()); }
        
        Quaint::QArray<Quaint::QName>& getDynamicStages() { return m_dyanmicStages; }
        void addDynamicStage(Quaint::QName name) { m_dyanmicStages.pushBack(name); } 
        
        const Quaint::QName& getName() { return m_name; }
        const Quaint::QName& getSceneName() { return m_sceneName; }
        uint32_t getStageIdx() { return m_stageIdx; }
        const ShaderDefinition& getShaderDefinition() { return m_shaderDefinition; }
        
    private:
        Quaint::QName       m_name = "";
        Quaint::QName       m_sceneName = "";
        uint32_t            m_stageIdx = 0;
        ShaderDefinition    m_shaderDefinition;
        Quaint::QArray<Quaint::QName> m_dyanmicStages;
        TPipelineImplPtr    m_pipelineImpl;

        //GPUDataDispatcher   m_dataDispatcher;
    };
}

#endif //_H_PIPELINE