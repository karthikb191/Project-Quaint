#include <GFX/Entities/Image.h>
#include <stb/stb_image.h>
#include <GFX/ResourceBuilder.h>

namespace Bolt
{
    Image2dRef Image2d::LoadFromFile(Quaint::IMemoryContext* context, const Quaint::QPath& filePath, const Quaint::QName& name)
    {
        Image2d* image = QUAINT_NEW(context, Image2d, context, name);
        Image2dRef imageRef(nullptr, Quaint::Deleter<Image2d>(context));
        imageRef.reset(image);
        imageRef->loadFromPath(filePath);
        return std::move(imageRef);
    }

    Image2d::Image2d(Quaint::IMemoryContext* context, const Quaint::QName& name)
    : IGFXEntity(context)
    , m_imageImpl(nullptr, Quaint::Deleter<IImageImpl>(context))
    {
    }

    void Image2d::construct()
    {   
        if(m_imageImpl.get() != nullptr)
        {
            QLOG_W(RESOURCE, "Cannot bind to GPU. A resource is already bound. Free it first");
            return;
        }

        CombinedImageSamplerTextureBuilder builder(m_context);
        m_imageImpl = std::move(builder.buildFromPixels(m_data, m_width, m_height));
    }

    void Image2d::destroy()
    {
        if(m_imageImpl.get() != nullptr)
        {
            m_imageImpl->destroy();
            m_imageImpl.release();
        }
        if(m_data != nullptr)
        {
            stbi_image_free(m_data);
        }
    }

    void Image2d::loadFromPath(const Quaint::QPath& path)
    {
        int compsPerPixel = 0;
        m_data = stbi_load(path.getBuffer(), &m_width, &m_height, &compsPerPixel, STBI_rgb_alpha);
    }
}