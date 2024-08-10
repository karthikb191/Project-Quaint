#ifndef _H_VULKAN_SHADER_GROUP
#define _H_VULKAN_SHADER_GROUP

#include <memory>
#include <GFX/Interface/IShaderGroup.h>
#include <Types/QArray.h>
#include "VulkanShader.h"

namespace Bolt
{
    /*
    * Should take ownership of shaders.
    * Cant be copied or assigned
    */
    class VulkanShaderGroup : public IShaderGroup
    {
        VulkanShaderGroup();
        VulkanShaderGroup(const char* vertSprvPath, const char* fragSpirvPath);
        virtual ~VulkanShaderGroup();

        VulkanShaderGroup(const VulkanShaderGroup&) = delete;
        VulkanShaderGroup(VulkanShaderGroup&&) = delete;
        VulkanShaderGroup& operator=(const VulkanShaderGroup&) = delete;

        bool isValid() const;

        /*
        Vertex shader associates "Vertex Input Attribute" number to each variable in shader
        "Vertex Input Attributes" are associated to "Vertex Input Bindings" on a per-pipeline basis (basically VIAs are indexed as per the shader variables)
        "Vertex Input Bindings" are associated to specific buffer on per-draw basis (Basically VIBs are indexed based on buffers passed to vkCmdBindVertexBuffers)
        */
        void addVertexInputBindingDescription(const VkVertexInputBindingDescription& desc);
        void addAttributeInputAttributeDescription(const VkVertexInputAttributeDescription& desc);
    private:

        //TODO: Expand to incorporate shaders dynamically
        //TODO: Use custom unique/shared ptrs
        std::unique_ptr<VulkanVertexShader>         m_vertShader;
        std::unique_ptr<VulkanFragmentShader>       m_fragShader;
        
        Quaint::QArray<VkVertexInputBindingDescription> m_VIBs = Quaint::QArray<VkVertexInputBindingDescription>::GetInvalidPlaceholder();
        Quaint::QArray<VkVertexInputAttributeDescription> m_VIAs = Quaint::QArray<VkVertexInputAttributeDescription>::GetInvalidPlaceholder();
    
    };
}

#endif // _H_VULKAN_SHADER_GROUP