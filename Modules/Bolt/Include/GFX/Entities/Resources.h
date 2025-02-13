#ifndef _H_RESOURCES
#define _H_RESOURCES
#include "../Interface/IRenderer.h"
#include "../Data/ResourceInfo.h"
#include "../Data/ShaderInfo.h"

#include <assert.h>

namespace Bolt
{

// Forward Declarations ================================================================    
    class ShaderResourceBase;
    class BufferResourceBase;
    class ShaderGroupResource;
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
        typedef GraphicsResource TYPE;
    };

    template<>
    class ResourceTraits<EResourceType::SHADER>
    {
    public:
        typedef ShaderResourceBase TYPE;
    };

    template<>
    class ResourceTraits<EResourceType::SHADER_GROUP>
    {
    public:
        typedef ShaderGroupResource TYPE;
    };

    template<>
    class ResourceTraits<EResourceType::BUFFER>
    {
    public:
        typedef BufferResourceBase TYPE;
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

    //!!!! TODO: This probably has a terrible memory leak problem. Use unique, shared ptr structures later

    //Should be implemented for specific resources on API side
    class Resource
    {
    public:
        Resource(Quaint::IMemoryContext* context, EResourceType type)
        : m_resourceType(type)
        , m_context(context)
        {}

        const EResourceType getResourceType() { return m_resourceType; }

        template<EResourceType _Type>
        constexpr typename ResourceTraits<_Type>::TYPE* getAs()
        {
            assert(_Type == m_resourceType && "Invalid type cast");
            return static_cast<typename ResourceTraits<_Type>::TYPE*>(this);
        }
        Quaint::IMemoryContext* getMemoryContext() { return m_context; }

    private:
        const EResourceType m_resourceType = EResourceType::Invalid;
        Quaint::IMemoryContext* m_context = nullptr;
    };

    class GraphicsResource : public Resource
    {
    public:
        GraphicsResource(Quaint::IMemoryContext* context, EResourceType type)
        : Resource(context, type)
        {}

        //TODO: Remove all of these. None of these are required
        //TODO: move these to a cpp file
        template<typename _T = GraphicsResource> 
        static _T* create(Quaint::IMemoryContext* context, EResourceType type, ResourceGPUProxy* gpuResource)
        {
            assert(gpuResource != nullptr && "Graphics resource needs a valid GPU resource");
            _T* resource = QUAINT_NEW(context, _T, context, type);
            resource->assignGpuProxyResource(gpuResource);
            //TODO: Add a log
            return resource;
        }
        template<typename _T = GraphicsResource, typename ...ARGS>
        static _T* create(Quaint::IMemoryContext* context, ResourceGPUProxy* gpuResource, ARGS... args)
        {
            assert(gpuResource != nullptr && "Graphics resource needs a valid GPU resource");
            _T* resource = QUAINT_NEW(context, _T, context, args...);
            resource->assignGpuProxyResource(gpuResource);
            //TODO: Add a log
            return resource;
        }

        /* GPU Resource is created implictly. Expects a constructor that accepts an object of this type*/
        template<typename _T = GraphicsResource, typename _PROXY, typename ...ARGS>
        static _T* create(Quaint::IMemoryContext* context, ARGS... args)
        {
            _T* resource = QUAINT_NEW(context, _T, context, args...);
            m_gpuProxy = QUAINT_NEW(m_context, _PROXY, *resource);
            //TODO: Add a log
            return resource;
        }

        /* No GPU proxy is created*/
        template<typename _T = GraphicsResource, typename ...ARGS>
        static _T* create(Quaint::IMemoryContext* context, ARGS... args)
        {
            _T* resource = QUAINT_NEW(context, _T, context, args...);
            //TODO: Add a log
            return resource;
        }

        ResourceGPUProxy* getGpuResourceProxy() { return m_gpuProxy; }

        virtual void bindToGpu();
        virtual void unbindFromGPU();

        virtual void destroy(Quaint::IMemoryContext* context)
        {
            if(m_gpuProxy)
            {
                //TODO: Add a log
                m_gpuProxy->destroy();
                QUAINT_DELETE(m_gpuProxy->getMemoryContext(), m_gpuProxy);
            }
        }

    protected:
        GraphicsResource() = delete;
        void assignGpuProxyResource(ResourceGPUProxy* gpuResource)
        {
            if(m_gpuProxy)
            {
                m_gpuProxy->destroy();
                QUAINT_DELETE(m_gpuProxy->getMemoryContext(), m_gpuProxy);
            }
            m_gpuProxy = gpuResource;
        }

        ResourceGPUProxy*   m_gpuProxy = nullptr;
    };

    class BufferResourceBase : public GraphicsResource
    {
    public:
        BufferResourceBase(Quaint::IMemoryContext* context, const EBufferType type)
        : GraphicsResource(context, EResourceType::BUFFER)
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
        BufferResource(Quaint::IMemoryContext* context)
        : BufferResourceBase(context, _BufferType)
        {}

        template<EBufferType _Type>
        typename BufferResource<_Type>* get()
        {
            assert(_Type == m_type && "Invalid type cast");
            return static_cast<typename ShaderResourceTraits<_Type>::RESOURCE_TYPE*>(this);
        }
    };

    class ShaderGroupResource : public GraphicsResource
    {
    public:
        ShaderGroupResource(Quaint::IMemoryContext* context)
        : GraphicsResource(context, EResourceType::SHADER)
        {}

    };

    class ShaderResourceBase : public GraphicsResource
    {
    public:
        ShaderResourceBase(Quaint::IMemoryContext* context, EShaderResourceType type)
        : GraphicsResource(context, EResourceType::SHADER)
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
        ShaderResource(Quaint::IMemoryContext* context, typename Traits::INPUT_INFO_TYPE pInfo)
        : ShaderResourceBase(context, ResourceType)
        , m_info(pInfo)
        {}

        const typename Traits::INPUT_INFO_TYPE& getInfo() { return pInfo; }

    private:
        typename Traits::INPUT_INFO_TYPE     m_info;
    };

    class ShaderGroupBase : public GraphicsResource
    {
    public:
        ShaderGroupBase(Quaint::IMemoryContext* context)
        : GraphicsResource(context, EResourceType::SHADER_GROUP)
        {}

    private:

    };

    /* Implementation should be defined in API-Layer */
    class UniformBuffer : public GraphicsResource
    {

    };

    /* Implementation should be defined in API-Layer */
    class Texture : public GraphicsResource
    {
    public:

    private:
    };
}

#endif //_H_RESOURCES