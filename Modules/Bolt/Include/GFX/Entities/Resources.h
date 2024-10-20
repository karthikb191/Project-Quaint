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

    template<EBufferType _BufferType>
    class BufferResource;

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
//Buffer Resource Traits ================================================================
    template<EBufferType _ResourceType>
    class BufferResourceTraits
    {
    public:
        typedef BufferResource<_ResourceType> RESOURCE_TYPE;
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
        template<typename _T = GraphicsResource> 
        static _T* create(Quaint::IMemoryContext* context, EResourceType type, ResourceGPUProxy* gpuResource)
        {
            assert(gpuResource != nullptr && "Graphics resource needs a valid GPU resource");
            _T* resource = QUAINT_NEW(context, _T, type);
            resource->assignGpuProxyResource(gpuResource);
            //TODO: Add a log
            return resource;
        }
        template<typename _T = GraphicsResource, typename ...ARGS>
        static _T* create(Quaint::IMemoryContext* context, ResourceGPUProxy* gpuResource, ARGS... args)
        {
            assert(gpuResource != nullptr && "Graphics resource needs a valid GPU resource");
            _T* resource = QUAINT_NEW(context, _T, args...);
            resource->assignGpuProxyResource(gpuResource);
            //TODO: Add a log
            return resource;
        }
        ResourceGPUProxy* getGpuResourceProxy() { return m_gpuProxy; }

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

    class BufferResourceBase : public GraphicsResource
    {
    public:
        BufferResourceBase(const EBufferType type)
        : GraphicsResource(EResourceType::BUFFER)
        , m_type(type)
        {}

        const EBufferType getBufferType() { return m_type; }
        template<EBufferType _Type>
        typename BufferResourceTraits<_Type>::RESOURCE_TYPE* get()
        {
            assert(_Type == m_type && "Invalid type cast");
            return static_cast<typename BufferResourceTraits<_Type>::RESOURCE_TYPE*>(this);
        }

     private:
        const EBufferType       m_type;
    };

    template<EBufferType _BufferType>
    class BufferResource : public BufferResourceBase
    {
    public:
        BufferResource()
        : BufferResourceBase(_BufferType)
        {}

        template<EBufferType BufferType>
        typename BufferResource<BufferType>* get()
        {
            assert(_Type == m_type && "Invalid type cast");
            return static_cast<typename ShaderResourceTraits<_Type>::RESOURCE_TYPE*>(this);
        }
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