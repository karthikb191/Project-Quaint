#ifndef _H_VULKAN_GRAPHICS_CONTEXT
#define _H_VULKAN_GRAPHICS_CONTEXT

#include <vulkan/vulkan.h>
#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>
#include <GFX/Vulkan/Internal/DeviceManager.h>
#include <GFX/Vulkan/Internal/VulkanRenderPass.h>

namespace Bolt{namespace vulkan
{
    /* Encapsulates Renderpass, framebuffer, Command Buffer and attachments used by renderpass and framebuffer.
    * Idea is that this would be self-contained rendering construct 
    * ? When creating/binding a pipeline, its compatibility should be checked against the renderpass
    */
    class RenderScene;
    class GraphicsContext;

    struct CommandPool
    {
        bool supportsGraphics(){ return queueDef.supportsGraphicsOps(); }
        bool supportsTransfer(){ return queueDef.supportsTransferOps(); }
        const QueueDefinition& getQueueDefinition() const { return queueDef; }
        bool isValid() const { return commandPool != VK_NULL_HANDLE; }

        VkCommandPool getHandle() { return commandPool; }

    private:
        CommandPool(){}
        CommandPool(VkCommandPool pPool, const VkCommandPoolCreateFlags pFlags, const QueueDefinition& pQueueDef);
        
        friend class GraphicsContext;
        VkCommandPool                           commandPool = VK_NULL_HANDLE;
        VkCommandPoolCreateFlags                flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        QueueDefinition                         queueDef = {};
    };

    class GraphicsContext
    {
    public:
        GraphicsContext(Quaint::IMemoryContext* context);
        void construct();
        void destroy();
        
        CommandPool& buildCommandPool(const VkCommandPoolCreateFlags flags, const EQueueTypeFlags supportedQueues);
        
        //TODO: Maybe add and remove could be handled better
        /* Requested code is free to use the buffer as it likes */
        void addCommandBuffer(const VkCommandBufferLevel level, const uint32_t count, VkCommandBuffer* buffers = nullptr);
        void removeCommandBuffer(const VkCommandBuffer* buffers, const uint32_t count);

        inline bool isValid() const { return m_valid; }
        CommandPool& getCommandPool() { return m_commandPool; }
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
        Quaint::IMemoryContext*         m_context = nullptr;
        CommandPool                     m_commandPool = {};
        bool                            m_valid = false;
    };
}}

#endif //_H_VULKAN_GRAPHICS_CONTEXT
