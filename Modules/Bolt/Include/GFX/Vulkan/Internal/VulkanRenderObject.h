#ifndef _H_VULKAN_RENDER_OBJECT
#define _H_VULKAN_RENDER_OBJECT

#include <GFX/Interface/IRenderer.h>
#include <GFX/Vulkan/Internal/VulkanShaderGroup.h>
#include <memory>
#include <GFX/Interface/IEntityInterfaces.h>
#include <GFX/Vulkan/Internal/Entities/UniformBuffer.h>
#include <GFX/Vulkan/Internal/Entities/Descriptor.h>

namespace Bolt{ 
    
    class Model;

    namespace vulkan{

    class VulkanShaderGroupResource;
    
    //TODO: This currently represents a mesh. Maybe it's better to rename this?
    class VulkanRenderObject : public IModelImpl
    {
    typedef std::unique_ptr<VulkanShaderGroup, Quaint::Deleter<VulkanShaderGroup>> VulkanShaderGroupRef;
    //typedef Quaint::QUniquePtr<VulkanBufferObjectResource, Deleter<VulkanBufferObjectResource>> BufferResourceRef;
    
    typedef Quaint::QUniquePtr<ResourceGPUProxy, Quaint::Deleter<ResourceGPUProxy>> ResourceGPUProxyPtr;

    public:
        VulkanRenderObject(Quaint::IMemoryContext* context);
        //virtual void build(const GeometryRenderInfo& renderInfo) override;
        virtual void construct() override;
        virtual void destroy() override;
        virtual void draw(RenderScene* scene) override;

        Model* getModel() { return m_model; }

    private:
        void addModelRef(Model* model);
        void createBuffersFromModel(Model* model);
        void createShaderGroup(const ShaderInfo& shaderinfo);
        void createDescriptorLayoutInformation(const ShaderInfo& shaderinfo);
        void allocateDescriptorPool(const ShaderInfo& shaderInfo);
        void createDescriptors(const ShaderInfo& shaderInfo);
        void writeDescriptorSets();
        void createPipeline();

        friend class RenderObjectBuilder;
        //VulkanShaderGroupRef                    m_shaderGroup;
        Model*                                  m_model;
        TBufferImplPtr                          m_vertexBuffer;
        TBufferImplPtr                          m_indexBuffer;

        GeometryRenderInfo                      m_renderInfo;
        Quaint::QArray<VkDescriptorSetLayout>   m_setLayouts; //Represents layout for the respective set
        Quaint::QArray<VkDescriptorSet>         m_sets;
        VkPipelineLayout                        m_pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorPool                        m_descriptorPool = VK_NULL_HANDLE;
        VkPipeline                              m_pipeline = VK_NULL_HANDLE;
        VkSampler                               m_sampler = VK_NULL_HANDLE;
        
        VulkanShaderGroupResource*              m_shaderGroupResource = nullptr;

        uint32_t                                m_indexSize = 0;
        VkIndexType                             m_indexBufferType = VK_INDEX_TYPE_UINT32;
    };
    
}}
#endif