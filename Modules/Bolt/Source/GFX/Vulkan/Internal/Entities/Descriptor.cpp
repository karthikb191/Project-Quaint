#include <GFX/Vulkan/Internal/Entities/Descriptor.h>
#include <GFX/Vulkan/Internal/Entities/UniformBuffer.h>

namespace Bolt{ namespace vulkan {
  
    Descriptor::Descriptor(Quaint::IMemoryContext* context)
    : m_context(context)
    {}
    

    uint32_t Descriptor::getIdOf(const Quaint::QName& name)
    {
        for(uint32_t i = 0; i < m_info.elements.getSize(); ++i)
        {
            if(m_info.elements[i].name == name)
            {
                return i;
            }
        }
        return ~0u;
    }

    UniformDescriptor::UniformDescriptor(Quaint::IMemoryContext* context)
    : Descriptor(context)
    , m_uniformBuffer(nullptr, Deleter<UniformBuffer>(context))
    {
    }

    bool UniformDescriptor::update(const ShaderInterface::DescriptorSetInfo& setInfo, const Quaint::QName& descriptorName)
    {
        bool found = false;

        ShaderInterface::DescriptorElementInfo newInfo;
        for(auto& descriptor : setInfo.items)
        {
            if(descriptor.name == descriptorName)
            {
                m_setIdx = setInfo.set;
                newInfo = descriptor;
                if(descriptor.elements.getSize() > 0)
                {
                    auto& element = descriptor.elements[descriptor.elements.getSize() - 1];
                    m_size = element.offset + element.size;
                }
                found = true;
            }
        }

        if(!found || !m_size)
        {
            return false;
        }

        //Data has changed. Destroy old uniform buffer and create a new one
        if(newInfo.name != m_info.name || newInfo.count != m_info.count || newInfo.location != m_info.location)
        {
            if(m_uniformBuffer.get())
            {
                m_uniformBuffer->destroy();
            }

            m_uniformBuffer.reset( QUAINT_NEW(m_context, UniformBuffer, m_context, m_size));
        }

        return true;
    }
}}