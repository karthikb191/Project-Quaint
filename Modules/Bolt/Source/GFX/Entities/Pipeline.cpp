#include <GFX/Entities/Pipeline.h>

namespace Bolt
{
    Pipeline::Pipeline(Quaint::IMemoryContext* context, const ShaderDefinition& shader)
    :GraphicsResource(context, EResourceType::PIPELINE)
    {
        
    }
}