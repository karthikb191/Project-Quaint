#ifndef _H_RESOURCES
#define _H_RESOURCES
#include "../Interface/IRenderer.h"
#include "../Data/ResourceInfo.h"
#include "../Data/ShaderInfo.h"
#include "../Helpers.h"
#include <Types/QUniquePtr.h>

#include <assert.h>
#include <QuaintLogger.h>

namespace Bolt
{
    DECLARE_LOG_CATEGORY(RESOURCE);
    DEFINE_LOG_CATEGORY(RESOURCE);
    
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


    //TODO: Use this to extend
    class IGPUBindable
    {
        public:
        
    };
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

    protected:
        const EResourceType m_resourceType = EResourceType::Invalid;
        Quaint::IMemoryContext* m_context = nullptr;
        Quaint::QName m_name = "";
    };

    class GraphicsResource : public Resource
    {
        public:
        typedef Quaint::QUniquePtr<ResourceGPUProxy, Deleter<ResourceGPUProxy>> ResourceGPUProxyPtr;

        GraphicsResource(Quaint::IMemoryContext* context, EResourceType type)
        : Resource(context, type)
        , m_gpuProxyPtr(nullptr, Deleter<ResourceGPUProxy>(context))
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
            _PROXY* gpuResource = QUAINT_NEW(context, _PROXY, context, std::ref(*resource));
            resource->assignGpuProxyResource(gpuResource);
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

        //TODO: Remove raw pointer m_gpuProxy
        ResourceGPUProxy* getGpuResourceProxy() 
        {
            if(m_gpuProxyPtr.get())
            {
                return m_gpuProxyPtr.get();
            }
            return m_gpuProxy; 
        }

        virtual void bindToGpu() { assert(false && "Need API specific implementation"); }
        virtual void unbindFromGPU() { assert(false && "Need API specific implementation"); }

        virtual void destroy(Quaint::IMemoryContext* context)
        {
            if(m_gpuProxy)
            {
                //TODO: Add a log
                m_gpuProxy->destroy();
                QUAINT_DELETE(m_gpuProxy->getMemoryContext(), m_gpuProxy);
            }
            if(m_gpuProxyPtr.get())
            {
                m_gpuProxyPtr->destroy();
                m_gpuProxyPtr.release();
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
        void assignGpuProxyResource(ResourceGPUProxyPtr&& gpuResource)
        {
            if(m_gpuProxyPtr.get())
            {
                Quaint::QString256 logBuf;
                sprintf_s(logBuf.getBuffer_NonConst(), logBuf.size(), "Destroying and replacing gpu resource in: %s", m_name.getBuffer());
                QLOG_W(RESOURCE, logBuf.getBuffer());
                m_gpuProxyPtr->destroy();
                m_gpuProxyPtr.release();
            }
            m_gpuProxyPtr = std::move(gpuResource);
        }

        ResourceGPUProxy*   m_gpuProxy = nullptr;
        ResourceGPUProxyPtr m_gpuProxyPtr;
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