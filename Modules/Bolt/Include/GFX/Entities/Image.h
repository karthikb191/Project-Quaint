#ifndef _H_BOLT_IMAGE
#define _H_BOLT_IMAGE

#include "./Resources.h"
#include "../Helpers.h"
#include <Types/QUniquePtr.h>

namespace Bolt
{
    //Forward Declares
    class Image2d;
    using Image2dRef = Quaint::QUniquePtr<Image2d, Deleter<Image2d>>;

    class Image2d : public GraphicsResource
    {
        public:
        static Image2dRef LoadFromFile(Quaint::IMemoryContext* context, const Quaint::QPath& filePath, const Quaint::QName& name);
        
        Image2d(Quaint::IMemoryContext* context, const Quaint::QName& name);
        void destroy();

        inline int getWidth() { return m_width; }
        inline int getHeight() { return m_height; }

        /* Currently only generates combined sampler image that can be bound to a shader. Extend this! */
        virtual void bindToGpu();
        virtual void unbindFromGPU();

        private:
        void loadFromPath(const Quaint::QPath& path);

        /* TODO: Clear CPU-side data once bound to GPU */
        unsigned char* m_data = nullptr;
        int m_width = 0;
        int m_height = 0;
        bool boundToGPU = false;
    };
}

#endif //_H_BOLT_IMAGE