#ifndef _H_VULKAN_SHADER
#define _H_VULKAN_SHADER

#include <vulkan/vulkan.h>
#include <Types/QStaticString.h>

namespace Bolt
{
    /*
    * Should only have a single source of truth and Shouldn't be copied or moved. 
    */
    class VulkanShader
    {
    public:
        enum EType
        {
            Invalid = 0,
            Vertex = 1 << 0,
            Fragment = 1 << 1,
            Compute = 1 << 2
        };

        VulkanShader(EType type, const char* spirvPath, const Quaint::QName entryFnName);
        virtual ~VulkanShader();
        VulkanShader(const VulkanShader&) = delete;
        VulkanShader(VulkanShader&&) = delete;
        VulkanShader& operator=(const VulkanShader&) = delete;
        
    private:
        void constructShaderModule(const char* spirvPath);
        // Vertex Input Descriptions that must be bound
        // Uniform objects that must be bound to a descriptor set
        const EType                     m_type;
        Quaint::QName                   m_entryFn;
        VkShaderModule                  m_shaderModule = VK_NULL_HANDLE;
    };

    class VulkanVertexShader : public VulkanShader
    {
    public:
        VulkanVertexShader(const char* spirvPath);
        VulkanVertexShader(const char* spirvPath, const Quaint::QName entryFunction);
    };

    class VulkanFragmentShader : public VulkanShader
    {
    public:
        VulkanFragmentShader(const char* spirvPath);
        VulkanFragmentShader(const char* spirvPath, const Quaint::QName entryFunction);
    };

    using VulkanShaderRef = VulkanShader*;
    using VulkanVertexShaderRef = VulkanVertexShader*;
    using VulkanFragmentShaderRef = VulkanFragmentShader*;
}

#endif //_H_VULKAN_SHADER