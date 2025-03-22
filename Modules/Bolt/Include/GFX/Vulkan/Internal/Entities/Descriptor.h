#ifndef _H_DESCRIPTOR
#define _H_DESCRIPTOR

#include<Types/QUniquePtr.h>
#include "../ShaderInterface.h"
#include <GFX/Helpers.h>

namespace Bolt{ namespace vulkan {

    class UniformBuffer;
    typedef Quaint::QUniquePtr<UniformBuffer, Deleter<UniformBuffer>> UniformBufferRef;
    class Descriptor
    {
    public:
        Descriptor(Quaint::IMemoryContext* context);

        uint32_t getlocation() { return m_info.location; }
        bool isArray() { return m_info.count > 1; }
        uint32_t getNumArrayElements() { return m_info.count; }
        uint32_t getIdOf(const Quaint::QName& name);
        uint32_t getSize() { return m_size; }
        
    protected:
        Quaint::IMemoryContext* m_context;
        uint32_t m_setIdx = 0;
        uint32_t m_size = 0;
        ShaderInterface::DescriptorElementInfo m_info;
    };

    class UniformDescriptor : public Descriptor
    {
    public:
        //TODO: Modify this to use the VulkanPipeline
        UniformDescriptor(Quaint::IMemoryContext* context);

        bool update(const ShaderInterface::DescriptorSetInfo& setInfo, const Quaint::QName& descriptorName); 

    private:
        Quaint::QName m_name;
        UniformBufferRef m_uniformBuffer;
    };

}}


#endif //_H_DESCRIPTOR