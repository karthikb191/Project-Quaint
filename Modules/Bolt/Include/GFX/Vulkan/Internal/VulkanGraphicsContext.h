#ifndef _H_VULKAN_GRAPHICS_CONTEXT
#define _H_VULKAN_GRAPHICS_CONTEXT

#include <Interface/IMemoryContext.h>

namespace Bolt
{
    /* Encapsulates Renderpass, framebuffer, Command Buffer and attachments used by renderpass and framebuffer.
    * Idea is that this would be self-contained rendering construct 
    * ? When creating/binding a pipeline, its compatibility should be checked against the renderpass
    */
   namespace vulkan
   {
        class GraphicsContext
        {
        public:
            GraphicsContext(Quaint::IMemoryContext* context);
            //TODO: For creation
            // Create Graphics Context for (Renderpass) function parameter
            // Set list of attachments to be used and create their views
            // Set RenderPass 
            // Set Framebuffer

            // Begin:
            // Begin command buffer
            // Begin Render pass

            // Let shaders or models bind their descriptor sets
            // Draw

            // End:
            // End Render pass
            // End Command Buffer

            // Submit to queue

            // Expose a fence 
            
        private:
            Quaint::IMemoryContext*         m_context;
        };
   }
}

#endif //_H_VULKAN_GRAPHICS_CONTEXT
