#ifndef _H_BOLT_IMAGE
#define _H_BOLT_IMAGE

#include "./Resources.h"
#include <Types/QUniquePtr.h>

namespace Bolt
{
    //Forward Declares
    class Image2d;
    using Image2dRef = Quaint::QUniquePtr<Image2d, Quaint::Deleter<Image2d>>;

    class Image2d : public IGFXEntity
    {
        public:
        static Image2dRef LoadFromFile(Quaint::IMemoryContext* context, const Quaint::QPath& filePath, const Quaint::QName& name, EFormat format = EFormat::R8G8B8A8_SRGB);
        static Image2dRef LoadHDRFromFile(Quaint::IMemoryContext* context, const Quaint::QPath& filePath, const Quaint::QName& name, EFormat format = EFormat::R8G8B8A8_SRGB);
        
        Image2d(Quaint::IMemoryContext* context, const Quaint::QName& name);
        Image2d(Quaint::IMemoryContext* context, const Quaint::QName& name, EFormat format);
        virtual ~Image2d(){}

        virtual void construct() override;
        virtual void destroy() override;

        inline int getWidth() { return m_width; }
        inline int getHeight() { return m_height; }

        template<typename T>
        T* getImplAs(){ return static_cast<T*>(m_imageImpl.get()); }

        const bool isBound() { return m_boundToGPU;}
        const unsigned char* const data() const { return m_data; }

        private:
        void loadFromPath(const Quaint::QPath& path);
        void loadHDRFromPath(const Quaint::QPath& path);

        /* TODO: Clear CPU-side data once bound to GPU */
        //Quaint::IMemoryContext* m_context;
        TImageImplPtr m_imageImpl;
        unsigned char* m_data = nullptr;
        int m_width = 0;
        int m_height = 0;
        bool m_boundToGPU = false;
        EFormat m_format = EFormat::R8G8B8A8_SRGB;
    };
}

#endif //_H_BOLT_IMAGE