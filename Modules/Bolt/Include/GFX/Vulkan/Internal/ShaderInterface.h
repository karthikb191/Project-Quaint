#ifndef _H_SHADER_MANAGER
#define _H_SHADER_MANAGER
#include <Interface/IMemoryContext.h>
#include <GFX/Interface/IShaderGroup.h>
#include <GFX/Data/ShaderInfo.h>


namespace Bolt{ namespace vulkan{
    
    //TODO: update this to read form a json file or something better. This is damn ugly
    class ShaderInterface
    {
    public:
        struct UniformElement
        {
            Quaint::QName name = "";
            uint32_t offset = 0;
            uint32_t size = 0;
        };
        struct DescriptorElementInfo
        {
            Quaint::QName name = "";
            uint32_t location = 0;
            uint32_t count = 1;
            Quaint::QArray<UniformElement> elements;
        };
        struct DescriptorSetInfo
        {
            Quaint::QName name = "";
            uint32_t set = 0;
            Quaint::QArray<DescriptorElementInfo> items;
        };
        struct Interface
        {
            Quaint::QName name = "";
            Quaint::QArray<DescriptorSetInfo> descriptorSets;
        };

        
        void setup(Quaint::IMemoryContext* context);

        Quaint::QArray<Interface>& getShaderInterfaces(Quaint::IMemoryContext* context);
        Interface getShaderInterface(const Quaint::QName& name);
        
    private:
        Quaint::QArray<Interface> shaderInterfaces;
    };

}}

#endif //_H_SHADER_MANAGER