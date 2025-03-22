#include <GFX/Vulkan/Internal/ShaderInterface.h>

namespace Bolt{namespace vulkan{

    struct Element
    {
        const uint32_t offset = 0;
        const uint32_t size = 0;
    };
    struct SimpleTriShader
    {
        /*descriptor set*/
        struct UBO
        {
            static const uint32_t set = 0;
            static const uint32_t location = 0;

            struct Data
            {
                const Element model{0, 10};
                const Element view{10, 10};
                const Element proj{20, 20};
            };

            const Data data{};
        };
    };

    void ShaderInterface::setup(Quaint::IMemoryContext* context)
    {
        static bool built = false;
        if (built) return;

        shaderInterfaces = Quaint::QArray<Interface>(context);
        Quaint::QArray<DescriptorSetInfo> descriptorSets(context);
        Quaint::QArray<DescriptorElementInfo> descriptorItems(context);

        Quaint::QArray<UniformElement> uniformElements(context);
        uniformElements = {
            {"model", 0, 32},
            {"view", 32, 64},
            {"proj", 64, 96}
        };
        descriptorItems.pushBack({"ubo", 0, 1, uniformElements});
        uniformElements.clear();
        descriptorItems.pushBack({"sampleTex", 1, 1, uniformElements});

        descriptorSets.pushBack({"model_params", 0, descriptorItems});
        
        shaderInterfaces.pushBack({"simpleTri", descriptorSets});
    }

    Quaint::QArray<ShaderInterface::Interface>& ShaderInterface::getShaderInterfaces(Quaint::IMemoryContext* context)
    {
        setup(context);
        return shaderInterfaces;
    }
    ShaderInterface::Interface ShaderInterface::getShaderInterface(const Quaint::QName& name)
    {
        for(auto& interface : shaderInterfaces)
        {
            if(interface.name == name)
            {
                return interface;
            }
        }

        return Interface{};
    }
}
}