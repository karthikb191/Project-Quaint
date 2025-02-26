#include <GFX/Vulkan/Internal/VulkanShader.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QArray.h>
#include <Types/QCTString.h>
#include <fstream>

namespace Bolt
{

    void getShaderCode(const char* path, Quaint::QArray<char>& outCode)
    {
        //open file as binary and read from end
        std::ifstream stream(path, std::ios::ate | std::ios::binary);
        assert(stream.is_open() && "Given file could not be opened");
        uint32_t fileSize = (uint32_t)stream.tellg();
        outCode.resize(fileSize);
        stream.seekg(0);    //Go to beginning of file
        stream.read(outCode.getBuffer_NonConst(), fileSize);
        stream.close();
    }

    VulkanShader::VulkanShader(EShaderStage stage, const char* spirvPath, const Quaint::QName entryFnName)
    : m_stage(stage)
    , m_shaderModule(VK_NULL_HANDLE)
    , m_entryFn(entryFnName)
    {
        constructShaderModule(spirvPath);
    }
    VulkanShader::~VulkanShader()
    {
        assert(m_shaderModule == VK_NULL_HANDLE && "Shader module not destroyed. Resource leak possible");
    }

    void VulkanShader::destroy()
    {
        if (m_shaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(VulkanRenderer::get()->getDeviceManager()->getDeviceDefinition().getDevice()
            , m_shaderModule
            , VulkanRenderer::get()->getAllocationCallbacks());
        }
        m_shaderModule = VK_NULL_HANDLE;
    }

    void VulkanShader::constructShaderModule(const char* spirvPath)
    {
        const VkDevice device = VulkanRenderer::get()->getDeviceManager()->getDeviceDefinition().getDevice();
        Quaint::QArray<char> outCode(VulkanRenderer::get()->getMemoryContext());
        getShaderCode(spirvPath, outCode);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = outCode.getSize();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(outCode.getBuffer());

        VkResult res = vkCreateShaderModule(device, &createInfo 
                            , VulkanRenderer::get()->getAllocationCallbacks()
                            , &m_shaderModule);
        ASSERT_SUCCESS(res, "Could not create a shader module");
    }
    

    VulkanVertexShader::VulkanVertexShader(const char* spirvPath)
    : VulkanShader(EShaderStage::VERTEX, spirvPath, "main")
    {
    }
    VulkanVertexShader::VulkanVertexShader(const char* spirvPath, const Quaint::QName entryFunction)
    : VulkanShader(EShaderStage::VERTEX, spirvPath, entryFunction)
    {
        
    }

    VulkanFragmentShader::VulkanFragmentShader(const char* spirvPath)
    : VulkanShader(EShaderStage::FRAGMENT, spirvPath, "main")
    {

    }
    VulkanFragmentShader::VulkanFragmentShader(const char* spirvPath, const Quaint::QName entryFunction)
    : VulkanShader(EShaderStage::FRAGMENT, spirvPath, entryFunction)
    {

    }
}