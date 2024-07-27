#ifndef _H_SHADER_MANAGER
#define _H_SHADER_MANAGER
#include <Interface/IMemoryContext.h>
#include <GFX/Interface/IShaderGroup.h>
#include <GFX/Data/ShaderInfo.h>
#include <Singleton.h>

namespace Bolt
{
    class ShaderManager : public Singleton<ShaderManager>
                        , public IShaderGroupConstructor
    {
    public:
        DECLARE_SINGLETON(Bolt::ShaderManager);

        void constructShaderGroup();
    protected:


    private:

    };
}

#endif //_H_SHADER_MANAGER