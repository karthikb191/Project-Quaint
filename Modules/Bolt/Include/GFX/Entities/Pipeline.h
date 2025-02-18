#ifndef _H_PIPELINE
#define _H_PIPELINE

#include <Interface/IMemoryContext.h>
#include "../Data/ShaderInfo.h"
#include "./Resources.h"

namespace Bolt
{
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

        Pipeline(Quaint::IMemoryContext* context, const ShaderDefinition& shader);
        
        //Uses builder to bind this pipeline to GPU
        virtual void bindToGpu() override { /*TODO:*/ }
        virtual void unbindFromGPU() override { /*TODO:*/ }
        
    private:
        PipelineInputInfo   m_piplelineInfo;
    };
}

#endif //_H_PIPELINE