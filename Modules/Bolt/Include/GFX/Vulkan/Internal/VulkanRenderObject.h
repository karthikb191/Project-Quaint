#ifndef _H_VULKAN_RENDER_OBJECT
#define _H_VULKAN_RENDER_OBJECT

#include <GFX/Interface/IRenderer.h>
#include <GFX/Vulkan/Internal/VulkanShaderGroup.h>
#include <memory>
#include <GFX/Helpers.h>

namespace Bolt{ namespace vulkan{

    class Bolt::RenderObject;
    class VulkanShaderGroupResource;

    class VulkanRenderObject : public IRenderObjectImpl
    {
    typedef std::unique_ptr<VulkanShaderGroup, Deleter<VulkanShaderGroup>> VulkanShaderGroupRef;

    public:
        VulkanRenderObject(RenderObject* ro);
        virtual void build(const GeometryRenderInfo& renderInfo) override;
        virtual void draw() override;
        void destroy();

    private:
        void createShaderGroup(const ShaderInfo& shaderinfo);
        void createDescriptorLayoutInformation(const ShaderInfo& shaderinfo);
        void allocateDescriptorPool(const ShaderInfo& shaderInfo);
        void createDescriptors(const ShaderInfo& shaderInfo);
        void writeDescriptorSets();
        void createPipeline();

        //VulkanShaderGroupRef                    m_shaderGroup;
        Quaint::QArray<VkDescriptorSetLayout>   m_setLayouts; //Represents layout for the respective set
        Quaint::QArray<VkDescriptorSet>         m_sets;
        VkPipelineLayout                        m_pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorPool                        m_descriptorPool = VK_NULL_HANDLE;
        VkPipeline                              m_pipeline = VK_NULL_HANDLE;
        VkSampler                               m_sampler = VK_NULL_HANDLE;
        
        VulkanShaderGroupResource*              m_shaderGroupResource = nullptr;
    };
    
}}
#endif