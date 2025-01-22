#include <GFX/Vulkan/Internal/VulkanShaderGroup.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Data/ShaderInfo.h>

namespace Bolt { namespace vulkan
{
    VulkanShaderGroup::VulkanShaderGroup(Quaint::IMemoryContext* context)
    : Bolt::ShaderGroup(context, "Invalid", "", "", Quaint::QMap<Quaint::QName, ShaderAttribute>(context))
    , m_context(context)
    , m_vertShader(nullptr, Deleter<VulkanVertexShader>(context))
    , m_fragShader(nullptr, Deleter<VulkanFragmentShader>(context))
    , m_VIBs(context)
    , m_VIAs(context)
    {
    }

    VulkanShaderGroup::VulkanShaderGroup(Quaint::IMemoryContext* context, const Quaint::QPath& vertSprvPath, const Quaint::QPath& fragSpirvPath)
    : Bolt::ShaderGroup(context, "NoName", vertSprvPath, fragSpirvPath, Quaint::QMap<Quaint::QName, ShaderAttribute>(context))
    , m_context(context)
    , m_vertShader(nullptr, Deleter<VulkanVertexShader>(context))
    , m_fragShader(nullptr, Deleter<VulkanFragmentShader>(context))
    , m_VIBs(context)
    , m_VIAs(context)
    {
        m_vertShader.reset(QUAINT_NEW(context, Bolt::VulkanVertexShader, vertSprvPath));
        m_fragShader.reset(QUAINT_NEW(context, Bolt::VulkanFragmentShader, fragSpirvPath));
    }

    VulkanShaderGroup::VulkanShaderGroup(Quaint::IMemoryContext* context, const Quaint::QName& name
        , const Quaint::QPath& vertShaderPath, const Quaint::QPath& fragShaderPath
        , const Quaint::QMap<Quaint::QName, ShaderAttribute>&& vertexAttributes)
    : Bolt::ShaderGroup(context, name, vertShaderPath, fragShaderPath, Quaint::QMap<Quaint::QName, ShaderAttribute>(context))
    , m_context(context)
    , m_vertShader(nullptr, Deleter<VulkanVertexShader>(context))
    , m_fragShader(nullptr, Deleter<VulkanFragmentShader>(context))
    , m_VIBs(context)
    , m_VIAs(context)
    {
        m_vertShader.reset(QUAINT_NEW(context, Bolt::VulkanVertexShader, vertSprvPath));
        m_fragShader.reset(QUAINT_NEW(context, Bolt::VulkanFragmentShader, fragSpirvPath));
    }

    VulkanShaderGroup::~VulkanShaderGroup()
    {
        //TODO: Assert destroyed. It'll be an invalid state otherwise
    }

    void VulkanShaderGroup::destroy()
    {
        if(m_vertShader.get())
        {
            m_vertShader->destroy();
            m_vertShader.release();
        }
        if(m_fragShader.get())
        {
            m_fragShader->destroy();
            m_fragShader.release();
        }
    }

    bool VulkanShaderGroup::isValid() const
    {
        return m_vertShader.get() != nullptr && m_fragShader.get() != nullptr;
    }

    void VulkanShaderGroup::setupDescriptions()
    {

    }
    
    //TODO: Not sure there are necessary here. Remove if they aren't
    void VulkanShaderGroup::addVertexInputBindingDescription(const VkVertexInputBindingDescription& desc)
    {
        //TODO: Perform any validation checks here
        m_VIBs.pushBack(desc);
    }

    void VulkanShaderGroup::addAttributeInputAttributeDescription(const VkVertexInputAttributeDescription& desc)
    {
        m_VIAs.pushBack(desc);
    }
    
}}