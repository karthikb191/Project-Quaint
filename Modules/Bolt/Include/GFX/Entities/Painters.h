#ifndef _H_BOLT_PAINTERS
#define _H_BOLT_PAINTERS

#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>
#include <Types/QStaticString.h>
#include <imgui.h>
#include <GFX/Interface/IEntityInterfaces.h>

//TODO: Remove this from here
#include <GFX/Vulkan/Internal/Entities/VulkanTexture.h>

namespace Bolt
{
    class Pipeline;
    class Model;
    class RenderScene;
    class Painter 
    {
    public:
        Painter::Painter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline);
        //TODO: There should be a destroy function


        virtual void preRender(RenderScene* scene) = 0;
        virtual void postRender() = 0;
        virtual void render(RenderScene* scene) = 0;

        const Quaint::QName& getPipelineName() const { return m_pipelineName; }
        Pipeline* getPipeline() { return m_pipeline; }
        bool isCompatibleWithScene(const Quaint::QName& sceneName);

    protected:
        Quaint::IMemoryContext* m_context = nullptr;
        Quaint::QName m_pipelineName = "";
        Pipeline*   m_pipeline = nullptr;
    };

    // Lot of similarities with Geometry painter. Combine these somehow
    class ShadowPainter : public Painter
    {
    public:
        struct GeometryShaderInfo
        {
            Model* model;
            VkDescriptorSet set;
            TBufferImplPtr uniformBuffer;
        };

        ShadowPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline);
        virtual void render(RenderScene* scene) override;
        virtual void preRender(RenderScene* scene) override;
        virtual void postRender() override;

        void AddModel(Model* model);
        
    private:
        //TODO: have reference to lights here
        Quaint::QArray<GeometryShaderInfo> m_geoInfo;
        Quaint::QMat4x4 m_lightProjection;
    };

    /* Geometry pipeline doesn't own models. Calling code should ensure the model is not destroyed as it's being used by the pipeline*/
    class GeometryPainter : public Painter
    {
    public:
        struct GeometryShaderInfo
        {
            Model* model;
            VkDescriptorSet set;
            TBufferImplPtr uniformBuffer;
            TBufferImplPtr materialBuffer;
        };

        GeometryPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline);
        virtual void render(RenderScene* scene) override;
        virtual void preRender(RenderScene* scene) override;
        virtual void postRender() override;

        void AddModel(Model* model);
        void setupLightsData();

    private:
        //Quaint::QArray<Model*> m_models = nullptr;
        Quaint::QArray<GeometryShaderInfo> m_geoInfo;
        TBufferImplPtr m_lightsbuffer;
        VkSampler m_sampler;
    };

    class ImguiPainter : public Painter
    {
        //TODO: Should make these platform agnostic
        struct ImguiDescriptorSet
        {
            vulkan::VulkanTexture texture;
            VkDescriptorSet set;
        };
        struct BufferData
        {
            VkBuffer buffer;
            VkDeviceMemory memory;
            uint32_t size;
        };

    public:
        ImguiPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline);
        virtual void render(RenderScene* scene) override;
        virtual void preRender(RenderScene* scene) override;
        virtual void postRender() override;

    private:
        void ProcessTexture(RenderScene* scene, ImDrawData* data, ImTextureData* textureData);
        void setupRenderState(RenderScene* scene, ImDrawData* data, VkCommandBuffer cmdBuffer);

        Quaint::QArray<ImguiDescriptorSet> m_descriptorInfo;
        VkCommandBuffer m_oneTimeCommandBuffer;
        VkSampler m_sampler;
        BufferData m_vertexBuffer;
        BufferData m_indexBuffer;
    };

}

#endif //_H_BOLT_PAINTERS