#ifndef _H_Vulkan_Pipeline
#define _H_Vulkan_Pipeline

#include <Vulkan/vulkan.h>
#include <Interface/IMemoryContext.h>
#include <GFX/Interface/IRenderer.h>
#include <GFX/Interface/IEntityInterfaces.h>
#include <GFX/Data/ShaderInfo.h>
#include <Types/QArray.h>
#include <GFX/Vulkan/Internal/ShaderInterface.h>

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
        VulkanGraphicsPipelineBuilder& setupRasterizationInfo(VkPolygonMode polyMode, VkCullModeFlags cullMode, VkFrontFace frontFace);
        VulkanGraphicsPipelineBuilder& addDynamicFeature(Quaint::QName feature);
        VulkanGraphicsPipelineBuilder& setBlendEnabled(bool enabled);
        VulkanGraphicsPipelineBuilder& enableDepth();
        
        //TODO: Add dynamic state support
        VulkanGraphicsPipeline* build();

    private:

        Quaint::IMemoryContext* m_context;
        VulkanGraphicsPipeline* m_pipeline;
    };
    
    class VulkanGraphicsPipeline : public IPipelineImpl
    {
        friend class VulkanGraphicsPipelineBuilder;
        
    public:
        VulkanGraphicsPipeline(Quaint::IMemoryContext* context);
        virtual void construct() override;
        virtual void destroy() override;
        VkPipeline getPipelineHandle() { return m_pipeline; }
        ShaderInterface::Interface& getShaderInterface() { return m_shaderInterface; }
        VkPipelineLayout getPipelineLayout() { return m_layout; }
        VkDescriptorPool getDescriptorPool() { return m_descriptorPool; }
        VkDescriptorSetLayout getDescriptorSetLayout() { return m_descritorSetLayout; }
        
    private:
        void buildShaders(const ShaderDefinition& definition);
        void setVertexAttributes(const ShaderDefinition& definition, VkVertexInputRate pInputRate);
        void setInputAssemblyState(const bool pRestartEnabled, const VkPrimitiveTopology pTopology);
        void setRenderStage(const RenderScene* const scene, const uint32_t stageIndex);
        /*Pipeline should own descriptors and pipeline layout. These should get destroyed with the pipeline*/
        void buildDescriptors(const ShaderDefinition& shaderDefinition);
        void addViewport(float x, float y, float width, float height, float minDepth, float maxDepth, VkOffset2D scissorOffset, VkExtent2D scissorExtent);
        void setRasterizationInfo(VkPolygonMode polyMode, VkCullModeFlags cullMode, VkFrontFace frontFace);
        void addDynamicFeature(Quaint::QName feature);
        void setBlendEnabled();
        void enableDepth();

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
        Quaint::QArray<Quaint::QName> m_dynamicFeatures;

        bool m_restartEnabled = false;
        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        //Rasterization Info
        VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags m_cullMode = VK_CULL_MODE_NONE;
        VkFrontFace m_frontFace = VK_FRONT_FACE_CLOCKWISE;
        bool m_blendEnabled = false;

        VkPipelineLayout m_layout = VK_NULL_HANDLE;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        uint32_t m_subPass = 0;

        //TODO: Move descriptor information to a dedicated descriptor generator
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE; /* Currently only supporting a single descriptor set */
        VkDescriptorSetLayout m_descritorSetLayout = VK_NULL_HANDLE;
        
        ShaderInterface m_totalShaderInterface;
        ShaderInterface::Interface m_shaderInterface;
        bool m_depthEnabled = false;
        // Does it need shader handle?
        // Need uniform and attribute info here for future validation
    };


    //TODO: Move these to a new file
    //Should be able to update descriptors
    /* Handles a single descriptor set */
    class DescriptorsHandler
    {
    public:
        DescriptorsHandler(Quaint::IMemoryContext* context, const Quaint::QName& setName, VulkanGraphicsPipeline* pipeline);
        
        void pushData(const Quaint::QName& descriptorName, const Quaint::QName& name /* Pass uniform buffer */);
        //void pushData(const Quaint::QName& name, Image2d* image);

        void updateDescriptors();
        void dispatch(VkCommandBuffer buffer, VkPipelineBindPoint bindPoint);

    private:
        Quaint::IMemoryContext* m_context;
        VulkanGraphicsPipeline* m_pipeline;
        VkDescriptorSet m_set = VK_NULL_HANDLE;
        ShaderInterface::DescriptorSetInfo m_descriptorInfo;
        Quaint::QArray<VkWriteDescriptorSet> m_writes;
    };

}}
#endif //_H_Vulkan_Pipeline