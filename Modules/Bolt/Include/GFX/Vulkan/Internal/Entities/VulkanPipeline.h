#ifndef _H_Vulkan_Pipeline
#define _H_Vulkan_Pipeline

#include <Vulkan/vulkan.h>

namespace Bolt{ namespace vulkan {

    class VulkanGraphicsPipeline;
    class VulkanGraphicsPipelineBuilder
    {
    public:
        VulkanGraphicsPipelineBuilder(Quaint::IMemoryContext* context);
        VulkanGraphicsPipelineBuilder& setupShaders(const ShaderDefinition& definition);
        VulkanGraphicsPipelineBuilder& setupBlendState();
        VulkanGraphicsPipelineBuilder& setupViewport();
        VulkanGraphicsPipelineBuilder& setupPrimitiveTopology();
        

        VulkanGraphicsPipeline* build();

    private:

        Quaint::IMemoryContext* m_context;
        VkGraphicsPipelineCreateInfo m_info = {};
        const ShaderDefinition* m_shaderDefinition;
        VulkanGraphicsPipeline* m_pipeline;
    };
    
    class VulkanGraphicsPipeline
    {
        friend class VulkanGraphicsPipelineBuilder;
    public:
        VulkanGraphicsPipeline(Quaint::IMemoryContext* context)
        : m_context(context){}
        void init();
        void destroy();
        
    private:
        void buildShaders(const ShaderDefinition& definition);

        Quaint::IMemoryContext* m_context;
        VkPipeline  m_pipeline = VK_NULL_HANDLE;

        ShaderDefinition m_shaderDefinition;
        Quaint::QArray<VulkanShader*> m_shaders;
        Quaint::QArray<VkPipelineShaderStageCreateInfo> m_shaderStageInfos;
        // Does it need shader handle?
        // Need uniform and attribute info here for future validation
    };

}}
#endif //_H_Vulkan_Pipeline