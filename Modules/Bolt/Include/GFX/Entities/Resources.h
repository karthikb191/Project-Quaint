#ifndef _H_RESOURCES
#define _H_RESOURCES

#include "../Data/ResourceInfo.h"
#include "../Data/ShaderInfo.h"
#include "../Interface/IRenderer.h"
#include <assert.h>

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
    class ResourceTraits<EResourceType::SHADER>
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

        template<EResourceType _Type>
        constexpr typename ResourceTraits<_Type>::TYPE* get()
        {
            assert(_Type == m_resourceType && "Invalid type cast");
            return static_cast<typename ResourceTraits<_Type>::TYPE*>(this);
        }

    private:
        const EResourceType m_resourceType = EResourceType::Invalid;
    };

    class GraphicsResource : public Resource
    {
    public:
        GraphicsResource(EResourceType type)
        : Resource(type)
        {}

        //TODO: move these to a cpp file
        static GraphicsResource* create(Quaint::IMemoryContext* context, EResourceType type, ResourceGPUProxy* gpuResource)
        {
            assert(gpuResource != nullptr && "Graphics resource needs a valid GPU resource");
            GraphicsResource* resource = QUAINT_NEW(context, GraphicsResource, type);
            resource->assignGpuProxyResource(gpuResource);
            //TODO: Add a log
            return resource;
        }
        ResourceGPUProxy* getGpuResourcProxy() { return m_gpuProxy; }

        void destroy()
        {
            if(m_gpuProxy)
            {
                //TODO: Add a log
                m_gpuProxy-> destroy();
            }
        }

    protected:
        GraphicsResource() = delete;
        void assignGpuProxyResource(ResourceGPUProxy* gpuResource)
        {
            if(m_gpuProxy)
            {
                m_gpuProxy->destroy();
            }
            m_gpuProxy = gpuResource;
        }

        ResourceGPUProxy*   m_gpuProxy = nullptr;
    };

    class ShaderResourceBase : public GraphicsResource
    {
    public:
        ShaderResourceBase(EShaderResourceType type)
        : GraphicsResource(EResourceType::SHADER)
        , m_type(type)
        {}


        const EShaderResourceType getShaderResourceType() { return m_type; }
        
        template<EShaderResourceType _Type>
        typename ShaderResourceTraits<_Type>::RESOURCE_TYPE* get()
        {
            assert(_Type == m_type && "Invalid type cast");
            return static_cast<typename ShaderResourceTraits<_Type>::RESOURCE_TYPE*>(this);
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