#ifndef _H_VULKAN_SHADER
#define _H_VULKAN_SHADER

#include <vulkan/vulkan.h>
#include <Types/QStaticString.h>
#include <GFX/Data/ShaderInfo.h>

namespace Bolt
{
    /*
    * Should only have a single source of truth and Shouldn't be copied or moved. 
    */
   namespace vulkan
   {
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

            VulkanShader(EShaderStage stage, const char* spirvPath, const Quaint::QName entryFnName);
            virtual ~VulkanShader();
            VulkanShader(const VulkanShader&) = delete;
            VulkanShader(VulkanShader&&) = delete;
            VulkanShader& operator=(const VulkanShader&) = delete;

            void destroy();
            
            VkShaderModule getHandle() { return m_shaderModule; }
        private:
            void constructShaderModule(const char* spirvPath);
            // Vertex Input Descriptions that must be bound
            // Uniform objects that must be bound to a descriptor set
            EShaderStage                    m_stage;
            Quaint::QName                   m_entryFn;
            VkShaderModule                  m_shaderModule = VK_NULL_HANDLE;
        };

        class VulkanVertexShader : public VulkanShader
        {
        public:
            VulkanVertexShader(const char* spirvPath);
            VulkanVertexShader(const char* spirvPath, const Quaint::QName entryFunction);

        private:
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
}

#endif //_H_VULKAN_SHADER