#ifndef _H_VULKAN_SHADER_GROUP
#define _H_VULKAN_SHADER_GROUP

#include <memory>
#include <GFX/Interface/IShaderGroup.h>
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
    private:

        //TODO: Expand to incorporate shaders dynamically
        //TODO: Use custom unique/shared ptrs
        std::unique_ptr<VulkanVertexShader>         m_vertShader;
        std::unique_ptr<VulkanFragmentShader>       m_fragShader;
    };
}

#endif // _H_VULKAN_SHADER_GROUP