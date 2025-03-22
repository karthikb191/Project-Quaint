#ifndef _H_VULKAN_RENDER_OBJECT
#define _H_VULKAN_RENDER_OBJECT

#include <GFX/Interface/IRenderer.h>
#include <GFX/Vulkan/Internal/VulkanShaderGroup.h>
#include <memory>
#include <GFX/Helpers.h>
#include <GFX/Vulkan/Internal/Entities/UniformBuffer.h>
#include <GFX/Vulkan/Internal/Entities/Descriptor.h>

namespace Bolt{ 
    
    class Model;

    namespace vulkan{

    class VulkanShaderGroupResource;

    class VulkanRenderObject : public IRenderObjectImpl
    {
    typedef std::unique_ptr<VulkanShaderGroup, Deleter<VulkanShaderGroup>> VulkanShaderGroupRef;
    //typedef Quaint::QUniquePtr<VulkanBufferObjectResource, Deleter<VulkanBufferObjectResource>> BufferResourceRef;
    
    typedef Quaint::QUniquePtr<ResourceGPUProxy, Deleter<ResourceGPUProxy>> ResourceGPUProxyPtr;

    public:
        VulkanRenderObject(Quaint::IMemoryContext* context);
        virtual void build(const GeometryRenderInfo& renderInfo) override;
        virtual void draw(RenderScene* scene) override;
        virtual void destroy() override;

    private:
        void createBuffersFromModel(Model* model);
        void createShaderGroup(const ShaderInfo& shaderinfo);
        void createDescriptorLayoutInformation(const ShaderInfo& shaderinfo);
        void allocateDescriptorPool(const ShaderInfo& shaderInfo);
        void createDescriptors(const ShaderInfo& shaderInfo);
        void writeDescriptorSets();
        void createPipeline();

        friend class RenderObjectBuilder;
        //VulkanShaderGroupRef                    m_shaderGroup;

        ResourceGPUProxyPtr                       m_vertexBuffer;
        ResourceGPUProxyPtr                       m_indexBuffer;


        Quaint::QArray<VkDescriptorSetLayout>   m_setLayouts; //Represents layout for the respective set
        Quaint::QArray<VkDescriptorSet>         m_sets;
        VkPipelineLayout                        m_pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorPool                        m_descriptorPool = VK_NULL_HANDLE;
        VkPipeline                              m_pipeline = VK_NULL_HANDLE;
        VkSampler                               m_sampler = VK_NULL_HANDLE;
        
        VulkanShaderGroupResource*              m_shaderGroupResource = nullptr;

        uint32_t                                m_indexSize = 0;
    };
    
}}
#endif