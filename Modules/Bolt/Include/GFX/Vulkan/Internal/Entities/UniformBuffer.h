#ifndef _H_UNIFORM_BUFFER
#define _H_UNIFORM_BUFFER
#include <vulkan/vulkan.h>
#include <Interface/IMemoryContext.h>

namespace Bolt{ namespace vulkan {

    class UniformBuffer
    {
    public:
        UniformBuffer(Quaint::IMemoryContext* context, uint32_t size);
        UniformBuffer(Quaint::IMemoryContext* context, void* data, uint32_t size);

        void destroy();

    private:
        void* map();
        void unmap();

        Quaint::IMemoryContext* m_context;

        VkDeviceMemory m_gpuMemory = VK_NULL_HANDLE; 
        VkBuffer m_buffer = VK_NULL_HANDLE;
        uint32_t m_size = 0;
    };
}}


#endif //_H_UNIFORM_BUFFER