#ifndef _H_BOLT_TEXTURE
#define _H_BOLT_TEXTURE

namespace Bolt
{
    class BoltTextureBase
    {
    };

    class BoltRenderTexture : public BoltTextureBase
    {
    public:
        virtual void draw() = 0;
    };
}

#endif //_H_BOLT_TEXTURE