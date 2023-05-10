#ifndef _H_I_RENDERER
#define _H_I_RENDERER

#include <Interface/IMemoryContext.h>

namespace Bolt
{
    class IRenderer
    {
    public:
       virtual void init(Quaint::IMemoryContext*) = 0;
    };
}

#endif //_H_I_RENDERER