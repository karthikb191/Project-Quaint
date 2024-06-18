#include <GFX/Vulkan/Internal/VulkanShader.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QArray.h>
#include <fstream>

namespace Bolt
{
    template<int ... ARGS>
    struct Res
    {
        static constexpr int res[sizeof...(ARGS)] = { ARGS... };

        constexpr Res(const char* c)
        : val { c[ARGS]... }
        {
        }
        const char val[sizeof...(ARGS)];
    };

    template<int N, int ITR, int ... ARGS>
    struct Itr
    {
        typedef typename Itr<N - 1, ITR + 1, ARGS..., ITR>::res res;
    };

    template<int ITR, int ... ARGS>
    struct Itr<0, ITR, ARGS...>
    {
        typedef typename Res<ARGS...> res;
    };

    template<int N>
    struct GetItr
    {
        typedef typename Itr<N, 0>::res res;
    };

    template<int SZ>
    constexpr typename GetItr<SZ>::res helper(const char(&str)[SZ])
    {
        return GetItr<SZ>::res(str);
    }

    void getShaderCode(const char* path, Quaint::QArray<char>& outCode)
    {
        //TODO: Remove this from here
        //constexpr auto tr = helper("hello babyyyyyyyyyyy");
        //constexpr char c = tr.val[0];

        //std::cout << filePath.getBuffer() << "\n";
        //open file as binary and read from end
        std::ifstream stream(path, std::ios::ate | std::ios::binary);
        assert(stream.is_open() && "Given file could not be opened");
        size_t fileSize = (size_t)stream.tellg();
        outCode.resize(fileSize);
        stream.seekg(0);    //Go to beginning of file
        stream.read(outCode.getBuffer_NonConst(), fileSize);
        stream.close();
    }

    VulkanShader::VulkanShader(EType type, const char* spirvPath, const Quaint::QName entryFnName)
    : m_type(type)
    , m_shaderModule(VK_NULL_HANDLE)
    , m_entryFn(entryFnName)
    {
        constructShaderModule(spirvPath);
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
    : VulkanShader(EType::Vertex, spirvPath, "main")
    {

    }
    VulkanVertexShader::VulkanVertexShader(const char* spirvPath, const Quaint::QName entryFunction)
    : VulkanShader(EType::Vertex, spirvPath, entryFunction)
    {
        
    }
    
    VulkanFragmentShader::VulkanFragmentShader(const char* spirvPath)
    : VulkanShader(EType::Fragment, spirvPath, "main")
    {

    }
    VulkanFragmentShader::VulkanFragmentShader(const char* spirvPath, const Quaint::QName entryFunction)
    : VulkanShader(EType::Vertex, spirvPath, entryFunction)
    {

    }
}