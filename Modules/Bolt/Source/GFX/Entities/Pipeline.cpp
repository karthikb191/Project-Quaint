#include <GFX/Entities/Pipeline.h>
#include <GFX/ResourceBuilder.h>

namespace Bolt
{
    Pipeline::Pipeline(Quaint::IMemoryContext* context, const Quaint::QName& name, const Quaint::QName& renderScene, const uint32_t stageIdx, const ShaderDefinition& shaderDef)
    : GraphicsResource(context, EResourceType::PIPELINE)
    , m_name(name)
    , m_sceneName(renderScene)
    , m_stageIdx(stageIdx)
    , m_shaderDefinition(shaderDef)
    {
        
    }

    void Pipeline::bindToGpu()
    {
        PipelineResourceBuilder builder = ResourceBuilderFactory::createBuilder<PipelineResourceBuilder>(m_context);
        builder.setPipelineRef(this);
        assignGpuProxyResource(std::move(builder.build()));
    }
}