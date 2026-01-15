#ifndef _H_I_ENTITY_INTERFACES
#define _H_I_ENTITY_INTERFACES

#include <Types/QUniquePtr.h>

namespace Bolt
{
    //Not sure these things are needed. Remove if not necessary
    class IGFXEntity
    {
    public:
        IGFXEntity(Quaint::IMemoryContext* context)
        : m_context(context)
        {}

        Quaint::IMemoryContext* m_context = nullptr;
        Quaint::IMemoryContext* getMemoryContext() { return m_context; }

        virtual void construct() = 0;
        virtual void destroy() = 0;   
    };
    
    class IPipelineImpl : public IGFXEntity
    {
    public:
        IPipelineImpl(Quaint::IMemoryContext* context)
        : IGFXEntity(context)
        {}
    };
    typedef Quaint::QUniquePtr<IPipelineImpl, Quaint::Deleter<IPipelineImpl>> TPipelineImplPtr;

    class RenderScene;
    class IModelImpl : public IGFXEntity
    {
    public:
        IModelImpl(Quaint::IMemoryContext* context)
        : IGFXEntity(context)
        {}

        virtual void draw(RenderScene* scene) = 0;
    };
    typedef Quaint::QUniquePtr<IModelImpl, Quaint::Deleter<IModelImpl>> TModelImplPtr;

    class IBufferImpl : public IGFXEntity
    {
    public:
        IBufferImpl(Quaint::IMemoryContext* context)
        : IGFXEntity(context)
        {}
        virtual void construct(void* data) = 0;
        virtual void map() = 0;
        virtual void unmap() = 0;
        virtual void** getMappedRegion() = 0;
    };
    typedef Quaint::QUniquePtr<IBufferImpl, Quaint::Deleter<IBufferImpl>> TBufferImplPtr;

    class IImageImpl : public IGFXEntity
    {
    public:
        IImageImpl(Quaint::IMemoryContext* context)
        : IGFXEntity(context)
        {}
        virtual void construct() override {};
        virtual void constructFromPath(char* path) = 0;
        virtual void constructFromPixels(void* pixels, uint32_t width, uint32_t height) = 0;
    };
    typedef Quaint::QUniquePtr<IImageImpl, Quaint::Deleter<IImageImpl>> TImageImplPtr;

    class IImageSamplerImpl : public IImageImpl
    {
    public:
        IImageSamplerImpl(Quaint::IMemoryContext* context)
        : IImageImpl(context)
        {}
    };
    typedef Quaint::QUniquePtr<IImageSamplerImpl, Quaint::Deleter<IImageSamplerImpl>> TImageSamplerImplPtr;

    class IRenderScene
    {

    };

    class IPipeline
    {

    };

    class IFrameBuffer
    {

    };
}

#endif //_H_I_ENTITY_INTERFACES