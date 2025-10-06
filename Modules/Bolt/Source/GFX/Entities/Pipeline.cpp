#include <GFX/Entities/Pipeline.h>
#include <GFX/ResourceBuilder.h>

namespace Bolt
{
    Pipeline::Pipeline(Quaint::IMemoryContext* context, const Quaint::QName& name, const Quaint::QName& renderScene, const uint32_t stageIdx, const ShaderDefinition& shaderDef)
    : IGFXEntity(context)
    , m_name(name)
    , m_sceneName(renderScene)
    , m_stageIdx(stageIdx)
    , m_shaderDefinition(shaderDef)
    , m_dyanmicStages(context)
    , m_pipelineImpl(nullptr, Deleter<IPipelineImpl>(context))
    {
        
    }

    void Pipeline::construct()
    {
        PipelineResourceBuilder builder = ResourceBuilderFactory::createBuilder<PipelineResourceBuilder>(m_context);
        builder.setPipelineRef(this);
        if(m_cullFront)
        {
            builder.setCullFront();
        }
        if(m_cullBack)
        {
            builder.setCullBack();
        }
        if(m_blendEnabled)
        {
            builder.enableBlend();
        }

        m_pipelineImpl = std::move(builder.build());
    }
    void Pipeline::destroy()
    {
        if(m_pipelineImpl.get() != nullptr)
        {
            m_pipelineImpl->destroy();
        }
    }

    //void Pipeline::bindToGpu()
    //{
    //    PipelineResourceBuilder builder = ResourceBuilderFactory::createBuilder<PipelineResourceBuilder>(m_context);
    //    builder.setPipelineRef(this);
    //    assignGpuProxyResource(std::move(builder.build()));
    //}
}