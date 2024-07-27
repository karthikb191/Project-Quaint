#ifndef _H_I_RENDERER
#define _H_I_RENDERER

#include <Interface/IMemoryContext.h>
#include "IShaderGroup.h"

namespace Bolt
{
    class IRenderer
    {
    public:
       virtual void init() = 0;
       virtual void shutdown() = 0;
       virtual void render() = 0;
       virtual ~IRenderer(){}

       virtual IShaderGroupConstructor* getShaderGroupConstructor() = 0;
    };
}

#endif //_H_I_RENDERER