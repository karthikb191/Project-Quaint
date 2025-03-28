#ifndef _H_PIPELINE
#define _H_PIPELINE

#include <Interface/IMemoryContext.h>
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

    /* Would not actually construct or own any shader resources */
    class Pipeline : public GraphicsResource
    {
    public:
        struct BlendInfo
        {

        };

        struct PipelineInputInfo
        {

        };

        //TODO: Add more constructor flavors for with primitive and blend information
        Pipeline(Quaint::IMemoryContext* context, const Quaint::QName& name, const Quaint::QName& renderScene, const uint32_t stageIdx, const ShaderDefinition& shaderDef);
        
        //Uses builder to bind this pipeline to GPU
        virtual void bindToGpu() override;
        virtual void unbindFromGPU() override { /*TODO:*/ }

        const Quaint::QName& getName() { return m_name; }
        const Quaint::QName& getSceneName() { return m_sceneName; }
        uint32_t getStageIdx() { return m_stageIdx; }
        const ShaderDefinition& getShaderDefinition() { return m_shaderDefinition; }
        
    private:
        Quaint::QName       m_name = "";
        Quaint::QName       m_sceneName = "";
        uint32_t            m_stageIdx = 0;
        ShaderDefinition    m_shaderDefinition;

        //GPUDataDispatcher   m_dataDispatcher;
    };
}

#endif //_H_PIPELINE