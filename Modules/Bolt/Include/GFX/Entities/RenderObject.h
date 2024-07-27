#ifndef _H_BOLT_RENDER_OBJECT
#define _H_BOLT_RENDER_OBJECT

namespace Bolt
{
    class RenderObject
    {
    public:
        virtual void draw() = 0;
        virtual void getVertexCount() = 0;
        virtual void getVertexBuffer() = 0;
        virtual void getIndexBuffer() = 0;
        virtual void getIndexCount() = 0;
        virtual void getShaderGroup() = 0;
    };

    //class RenderQuad : public RenderObject
    //{
//
    //};
}

#endif //_H_BOLT_RENDER_OBJECT