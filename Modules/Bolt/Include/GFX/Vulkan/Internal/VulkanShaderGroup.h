#ifndef _H_VULKAN_SHADER_GROUP
#define _H_VULKAN_SHADER_GROUP

#include <memory>
#include <GFX/Entities/Resources.h>
#include <Types/QArray.h>
#include <GFX/Helpers.h>
#include "VulkanShader.h"
#include <GFX/Entities/ShaderGroup.h>

namespace Bolt { namespace vulkan{


    /*
    * Should take ownership of shaders.
    * Cant be copied or assigned
    */
    class VulkanShaderGroup : public Bolt::ResourceGPUProxy 
    {
    public:
        VulkanShaderGroup(Quaint::IMemoryContext* context);
        VulkanShaderGroup(Quaint::IMemoryContext* context, const Quaint::QPath& vertSprvPath, const Quaint::QPath& fragSpirvPath);
        //VulkanShaderGroup(Quaint::IMemoryContext* context, const Bolt::ShaderGroup& shaderGroup);

        virtual ~VulkanShaderGroup();
        void destroy();

        VulkanShaderGroup(const VulkanShaderGroup&) = delete;
        VulkanShaderGroup(VulkanShaderGroup&&) = delete;
        VulkanShaderGroup& operator=(const VulkanShaderGroup&) = delete;

        bool isValid() const;

        void moveFrom(VulkanShaderGroup& other)
        {
            m_vertShader.swap(other.m_vertShader);
            m_fragShader.swap(other.m_fragShader);
            m_VIAs = other.m_VIAs;
            m_VIBs = other.m_VIBs;
        }

        /*
        Vertex shader associates "Vertex Input Attribute" number to each variable in shader
        "Vertex Input Attributes" are associated to "Vertex Input Bindings" on a per-pipeline basis (basically VIAs are indexed as per the shader variables)
        "Vertex Input Bindings" are associated to specific buffer on per-draw basis (Basically VIBs are indexed based on buffers passed to vkCmdBindVertexBuffers)
        */
        void addVertexInputBindingDescription(const VkVertexInputBindingDescription& desc);
        void addAttributeInputAttributeDescription(const VkVertexInputAttributeDescription& desc);

        VulkanVertexShader* getVertexShader() { return m_vertShader.get(); }
        VulkanFragmentShader* getFragmentShader() { return m_fragShader.get(); }
    private:
        void setupDescriptions();

        Quaint::IMemoryContext* m_context = nullptr;
        //TODO: Expand to incorporate shaders dynamically
        //TODO: Use custom unique/shared ptrs
        std::unique_ptr<VulkanVertexShader, Deleter<VulkanVertexShader>>            m_vertShader;
        std::unique_ptr<VulkanFragmentShader, Deleter<VulkanFragmentShader>>        m_fragShader;
        
        Quaint::QArray<VkVertexInputBindingDescription> m_VIBs = Quaint::QArray<VkVertexInputBindingDescription>::GetInvalidPlaceholder();
        Quaint::QArray<VkVertexInputAttributeDescription> m_VIAs = Quaint::QArray<VkVertexInputAttributeDescription>::GetInvalidPlaceholder();
    
    };
}}

#endif // _H_VULKAN_SHADER_GROUP