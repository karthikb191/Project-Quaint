#include <GFX/ResourceBuilder.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>
#include <GFX/Entities/Pipeline.h>
#include <GFX/Entities/Model.h>
#include <GFX/Vulkan/Internal/Entities/VulkanPipeline.h>
#include <GFX/Vulkan/Internal/VulkanRenderObject.h>

//Vulkan Specific implementation of resource builder
//TODO: Surround with VULKAN_API macro
namespace Bolt {
    using namespace vulkan;

    //Image Resource builder
    TImageSamplerImplPtr CombinedImageSamplerTextureBuilder::buildFromPath(const char* path)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.unnormalizedCoordinates = VK_FALSE; // We are using normalized coordinates
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.magFilter = VK_FILTER_LINEAR;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        //TODO: Not entirely sure of the way this functions. Need to read more on this
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = 0;

        VkSampler sampler;
        VkResult res = vkCreateSampler(device, &samplerInfo, callbacks, &sampler);

        VulkanTexture* texture = QUAINT_NEW(m_context, VulkanTexture, m_context);
        //TODO: Hardcoding flag for now. Create a comming enum for that later
        VulkanRenderer::get()->createTextureFromFile(path, *texture, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        VulkanTextureRef textureRef(texture, Deleter<VulkanTexture>(m_context));

        VulkanCombinedImageSamplerResource* combinedImageSampler = QUAINT_NEW(m_context, VulkanCombinedImageSamplerResource, m_context, sampler, std::move(textureRef));
        
        return TImageSamplerImplPtr(combinedImageSampler, Deleter<VulkanCombinedImageSamplerResource>(m_context));
    }
    TImageSamplerImplPtr CombinedImageSamplerTextureBuilder::buildFromPixels(unsigned char* pixels, int width, int height)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.unnormalizedCoordinates = VK_FALSE; // We are using normalized coordinates
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.magFilter = VK_FILTER_LINEAR;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        //TODO: Not entirely sure of the way this functions. Need to read more on this
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = 0;

        VkSampler sampler;
        VkResult res = vkCreateSampler(device, &samplerInfo, callbacks, &sampler);

        VulkanTexture* texPtr = QUAINT_NEW(m_context, VulkanTexture, m_context);
        VulkanRenderer::get()->createShaderTextureFromPixels(*texPtr, pixels, width, height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        VulkanTextureRef texRef(texPtr, Deleter<VulkanTexture>(m_context));

        VulkanCombinedImageSamplerResource* combinedImageSampler = QUAINT_NEW(m_context, VulkanCombinedImageSamplerResource, m_context, sampler, std::move(texRef));
        return TImageSamplerImplPtr(combinedImageSampler, Deleter<VulkanCombinedImageSamplerResource>(m_context));
    }

    //UB resource builder
    TBufferImplPtr BufferResourceBuilder::build()
    {
        assert(m_dataSize > 0 && "invalid data");

        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        EBufferType resourcetype = EBufferType::INVALID;

        TBufferImplPtr bufferImplPtr(nullptr, Deleter<IBufferImpl>(m_context));
        IBufferImpl* impl = nullptr;
        switch(m_bufferType)
        {
            case EBufferType::VERTEX:
                impl = buildVertexBuffer();
            break;
            
            case EBufferType::INDEX:
                impl = buildIndexBuffer();
            break;
            
            case EBufferType::UNIFORM:
                impl = buildUniformBuffer();
            break;

            default:
                assert(false && "Unsupported buffer type provided");
            break;
        }
        bufferImplPtr.reset(impl);
        return std::move(bufferImplPtr);
    }

    IBufferImpl* BufferResourceBuilder::buildVertexBuffer()
    {   
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VulkanBufferObjectResource* proxy = QUAINT_NEW(m_context, VulkanBufferObjectResource, m_context);
        VulkanBufferObjectResource::BufferInfo info{};
        info.memFlags = memFlags;
        info.usageFlags = usage;
        info.size = m_dataSize;
        info.offset = m_dataOffset;
        proxy->setBufferInfo(info);
        proxy->construct(m_data);

        return proxy;
    }
    IBufferImpl* BufferResourceBuilder::buildIndexBuffer()
    {
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VulkanRenderer::get()->createBuffer(m_dataSize, m_data, usage, memFlags, deviceMemory, buffer);

        VulkanBufferObjectResource* proxy = QUAINT_NEW(m_context, VulkanBufferObjectResource, m_context);
        VulkanBufferObjectResource::BufferInfo info{};
        info.memFlags = memFlags;
        info.usageFlags = usage;
        info.size = m_dataSize;
        info.offset = m_dataOffset;
        proxy->setBufferInfo(info);
        proxy->construct(m_data);

        return proxy;
    }
    IBufferImpl* BufferResourceBuilder::buildUniformBuffer()
    {
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VulkanRenderer::get()->createBuffer(m_dataSize, usage, memFlags, deviceMemory, buffer);

        VulkanBufferObjectResource* proxy = QUAINT_NEW(m_context, VulkanBufferObjectResource, m_context);
        VulkanBufferObjectResource::BufferInfo info{};
        info.memFlags = memFlags;
        info.usageFlags = usage;
        info.size = m_dataSize;
        info.offset = m_dataOffset;
        proxy->setBufferInfo(info);
        proxy->construct(m_data);

        //TODO:
        if(m_initiallyMapped)
        {

        }

        return proxy;
    }

    // ShaderGroupResourceBuilder& ShaderGroupResourceBuilder::addAttchmentRef(const ShaderAttachmentInfo& info)
    // {
    //     m_attachmentsRefs.pushBack(info);
    //     return *this;
    // }

    // ShaderGroupResourceBuilderPtr&& ShaderGroupResourceBuilder::build()
    // {
    //     ShaderGroup* shaderGroup = 
    //     GraphicsResource::create<ShaderGroup, VulkanShaderGroup>(m_context
    //                         , m_name
    //                         , m_fragShaderPath
    //                         , m_vertShaderPath
    //                         , std::move(m_attributeMap));
    //     m_ptr.reset(shaderGroup);
    //     return std::move(m_ptr);
    // }

    PipelineResourceBuilder& PipelineResourceBuilder::setPipelineRef(Pipeline* pipeline)
    {
        m_pipeline = pipeline;
        return *this;
    }
    TPipelineImplPtr PipelineResourceBuilder::build()
    {
        using namespace vulkan;
        //TODO: Depending on pipeline type, this would be the place to branch out

        RenderScene* scene = VulkanRenderer::get()->getRenderScene(m_pipeline->getSceneName());

        VulkanGraphicsPipelineBuilder builder(m_context);
        builder.setupShaders(m_pipeline->getShaderDefinition())
        .setupRenderStageInfo(scene, m_pipeline->getStageIdx(), true)
        .setupPrimitiveTopology(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .setupRasterizationInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        auto& features = m_pipeline->getDynamicStages();
        for(auto& feature : features)
        {
            builder.addDynamicFeature(feature);
        }
    
        VulkanGraphicsPipeline* pipeline =  builder.build();

        m_ptr.reset(pipeline);
        return std::move(m_ptr);
    }

//====================================================================================
//============================ RENDER OBJECT BUILDER =================================

    TModelImplPtr RenderObjectBuilder::buildFromModel(Model* model)
    {
        TModelImplPtr impl(nullptr, Deleter<IModelImpl>(m_context));
        VulkanRenderObject* proxy = QUAINT_NEW(m_context, VulkanRenderObject, m_context);
        
        // Building Vertex and Index buffers
        //TODO: Support multiple meshes
        proxy->addModelRef(model);
        proxy->construct();

        impl.reset(proxy);
        return std::move(impl);
    }

//====================================================================================
//====================================================================================

}