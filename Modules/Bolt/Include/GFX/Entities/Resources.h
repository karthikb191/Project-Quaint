#ifndef _H_RESOURCES
#define _H_RESOURCES

#include "../Data/ResourceInfo.h"
#include "../Data/ShaderInfo.h"

namespace Bolt
{
// Forward Declarations ================================================================    
    class ShaderResourceBase;
    class Resource;

    template<EShaderResourceType ResourceType>
    class ShaderResourceTraits;
    
    template<EShaderResourceType ResourceType
    , typename Traits = ShaderResourceTraits<ResourceType>>
    class ShaderResource;

//=======================================================================================
// Resource traits =====================================================================
    template<EResourceType Resource>
    class ResourceTraits
    {
    public:
        typedef void TYPE;
    };

    template<>
    class ResourceTraits<EResourceType::ShaderResource>
    {
    public:
        typedef ShaderResourceBase TYPE;
    };
//=======================================================================================
//Shader Resource Traits ================================================================
    template<EShaderResourceType ResourceType>
    class ShaderResourceTraits
    {
    public:
        typedef void INPUT_INFO_TYPE;
        typedef void RESOURCE_TYPE;
    };

    template<>
    class ShaderResourceTraits<EShaderResourceType::COMBINED_IMAGE_SAMPLER>
    {
    public:
        typedef CombinedImageSamplerInfo INPUT_INFO_TYPE;
        typedef ShaderResource<EShaderResourceType::COMBINED_IMAGE_SAMPLER> RESOURCE_TYPE;
    };

    template<>
    class ShaderResourceTraits<EShaderResourceType::UNIFORM_BUFFER>
    {
    public:
        typedef UniformBufferInputInfo INPUT_INFO_TYPE;
        typedef ShaderResource<EShaderResourceType::UNIFORM_BUFFER> RESOURCE_TYPE;
    };

//=======================================================================================
//=======================================================================================

    //Should be implemented for specific resources on API side
    class Resource
    {
    public:
        Resource(EResourceType type)
        : m_resourceType(type)
        {}

        const EResourceType getResourceType() { return m_resourceType; }

        template<EResourceType Type>
        typename ResourceTraits<Type>::TYPE* get()
        {
            return static_cast<typename ResourceTraits<Type>::TYPE*>(this);
        }

    private:
        const EResourceType m_resourceType = EResourceType::Invalid;
    };

    class ShaderResourceBase : public Resource
    {
    public:
        ShaderResourceBase(EShaderResourceType type)
        : Resource(EResourceType::ShaderResource)
        , m_type(type)
        {}

        const EShaderResourceType getShaderResourceType() { return m_type; }
        
        template<EShaderResourceType Type>
        typename ShaderResourceTraits<Type>::RESOURCE_TYPE* get()
        {
            return static_cast<typename ShaderResourceTraits<Type>::RESOURCE_TYPE*>(this);
        }

    private:
        const EShaderResourceType m_type = EShaderResourceType::INVALID;
    };

    template<EShaderResourceType ResourceType
    , typename Traits>
    class ShaderResource : public ShaderResourceBase
    {
    public:
        ShaderResource(typename Traits::INPUT_INFO_TYPE pInfo)
        : ShaderResourceBase(ResourceType)
        , m_info(pInfo)
        {}

        const typename Traits::INPUT_INFO_TYPE& getInfo() { return pInfo; }

    private:
        typename Traits::INPUT_INFO_TYPE     m_info;
    };
}

#endif //_H_RESOURCES