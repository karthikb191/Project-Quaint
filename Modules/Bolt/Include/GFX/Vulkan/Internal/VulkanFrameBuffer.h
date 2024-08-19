#ifndef _H_VULKAN_FRAME_BUFFER
#define _H_VULKAN_FRAME_BUFFER

#include "Interface/IMemoryContext.h"

namespace Bolt{ namespace vulkan{

    /* Creates and Owns any attachments provided */
    class FrameBuffer
    {

    public:
        FrameBuffer(Quaint::IMemoryContext* context);

        void addColorAttachment();
        void addDepthAttachment();

    private:
        Quaint::IMemoryContext*     m_context;
        
    };

} }

#endif