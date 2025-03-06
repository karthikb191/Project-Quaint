#ifndef _H_Vulkan_Pipeline
#define _H_Vulkan_Pipeline

#include <Vulkan/vulkan.h>
#include <Interface/IMemoryContext.h>
#include <GFX/Interface/IRenderer.h>
#include <GFX/Data/ShaderInfo.h>
#include <Types/QArray.h>

namespace Bolt{ 
    
    class RenderScene;
    class RenderStage;

    namespace vulkan {

    class VulkanGraphicsPipeline;
    class VulkanShader;
    class VulkanGraphicsPipelineBuilder
    {
    public:
        VulkanGraphicsPipelineBuilder(Quaint::IMemoryContext* context);
        VulkanGraphicsPipelineBuilder& setupShaders(const ShaderDefinition& definition);
        VulkanGraphicsPipelineBuilder& setupPrimitiveTopology(bool primitiveRestartEnabled = false, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        VulkanGraphicsPipelineBuilder& setupRenderStageInfo(const RenderScene* const scene, const uint32_t stageIndex, const bool addViewport);
        //TODO: Add viewport override
        VulkanGraphicsPipelineBuilder& setupRasterizationInfo(VkPolygonMode polyMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace);
        
        //TODO: Add dynamic state support
        VulkanGraphicsPipeline* build();

    private:

        Quaint::IMemoryContext* m_context;
        VulkanGraphicsPipeline* m_pipeline;
    };
    
    class VulkanGraphicsPipeline : public ResourceGPUProxy
    {
        friend class VulkanGraphicsPipelineBuilder;
        
    public:
        VulkanGraphicsPipeline(Quaint::IMemoryContext* context);
        void init();
        virtual void destroy() override;
        VkPipeline getPipelineHandle() { return m_pipeline; }
        
    private:
        void buildShaders(const ShaderDefinition& definition);
        void setVertexAttributes(const ShaderDefinition& definition, VkVertexInputRate pInputRate);
        void setInputAssemblyState(const bool pRestartEnabled, const VkPrimitiveTopology pTopology);
        void setRenderStage(const RenderScene* const scene, const uint32_t stageIndex);
        /*Pipeline should own descriptors and pipeline layout. These should get destroyed with the pipeline*/
        void buildDescriptors(const ShaderDefinition& shaderDefinition);
        void addViewport(float x, float y, float width, float height, float minDepth, float maxDepth, VkOffset2D scissorOffset, VkExtent2D scissorExtent);
        void setRasterizationInfo(VkPolygonMode polyMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace);

        VkPipeline  m_pipeline = VK_NULL_HANDLE;

        ShaderDefinition m_shaderDefinition;
        //TODO: Move these to cache data or something
        Quaint::QArray<VulkanShader*> m_shaders;
        Quaint::QArray<VkPipelineShaderStageCreateInfo> m_shaderStageInfos;
        Quaint::QArray<VkVertexInputBindingDescription> m_bindingDescs;
        Quaint::QArray<VkVertexInputAttributeDescription> m_attrDescs;
        Quaint::QArray<VkPipelineColorBlendAttachmentState> m_blendAttachments;
        Quaint::QArray<VkViewport> m_viewports;
        Quaint::QArray<VkRect2D> m_scissors;

        bool m_restartEnabled = false;
        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        //Rasterization Info
        VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlagBits m_cullMode = VK_CULL_MODE_NONE;
        VkFrontFace m_frontFace = VK_FRONT_FACE_CLOCKWISE;

        VkPipelineLayout m_layout = VK_NULL_HANDLE;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        uint32_t m_subPass = 0;

        //TODO: Move descriptor information to a dedicated descriptor generator
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE; /* Currently only supporting a single descriptor set */
        VkDescriptorSetLayout m_descritorSetLayout = VK_NULL_HANDLE;

        // Does it need shader handle?
        // Need uniform and attribute info here for future validation
    };

}}
#endif //_H_Vulkan_Pipeline