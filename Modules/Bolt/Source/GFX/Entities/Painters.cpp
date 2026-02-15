#include <GFX/Entities/Painters.h>
#include <GFX/Entities/Model.h>
#include <ImguiHandler.h>
#include <RenderModule.h>
#include <GFX/Entities/RenderScene.h>
#include <GFX/Entities/Pipeline.h>
#include <GFX/ResourceBuilder.h>
#include <GFX/Materials/SimpleMaterial.h>
#include <GFX/Data/MaterialData.h>
#include <GFX/Entities/Image.h>

// TODO: Remove this from here
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>
#include <GFX/Vulkan/Internal/Entities/VulkanPipeline.h>
#include <GFX/Vulkan/VulkanHelpers.h>
#include <GFX/Vulkan/VulkanResourceHelpers.h>
#include <Types/QVector.h>
#include <BoltMemoryProvider.h>
#include <stb/stb_image.h>
#include <iostream>

namespace Bolt
{
    class SkyboxVertexDataProvider : public IVertexDataProvider
    {
        struct CubemapDebugVertex
        {
            Quaint::QVec4    position;
            Quaint::QVec4    normal;

            constexpr static uint32_t getPositionOffset() { return offsetof(CubemapDebugVertex, position); }
            constexpr static uint32_t getNormalOffset() { return offsetof(CubemapDebugVertex, normal); }
        };

    public:

        SkyboxVertexDataProvider(Quaint::IMemoryContext* context, Model* model)
        : m_vertices(BOLT_ALLOCATOR)
        {
            auto& vertices = model->getVertices();
            for(const auto& vertex : vertices)
            {
                m_vertices.push_back({vertex.position, vertex.normal});
            }
        }

        virtual uint32_t getVertexBufferSize()
        {
            return m_vertices.size() * sizeof(CubemapDebugVertex);
        }
        virtual void* getVertexBufferData()
        {
            return m_vertices.data();
        }

    private:
        Quaint::QVector<CubemapDebugVertex> m_vertices;
    };

    Quaint::QMat4x4 G_lightView = Quaint::QMat4x4::Identity();
    Quaint::QMat4x4 G_lightProj = Quaint::QMat4x4::Identity();

    Painter::Painter(Quaint::IMemoryContext *context, const Quaint::QName &pipeline)
        : m_context(context), m_pipelineName(pipeline)
    {
        m_pipeline = VulkanRenderer::get()->getPipeline(pipeline);
    }

    bool Painter::isCompatibleWithScene(const Quaint::QName &sceneName)
    {
        if (m_pipeline == nullptr)
        {
            return false;
        }

        if (m_pipeline->getSceneName() == sceneName)
        {
            return true;
        }
        return false;
    }

    ShadowPainter::ShadowPainter(Quaint::IMemoryContext *context, const Quaint::QName &pipeline)
        : Painter(context, pipeline), m_geoInfo(context)
    {
        // m_lightProjection = Quaint::buildPerspectiveProjectionMatrix(0.1f, 100, 90, 1);
        m_lightProjection = Quaint::buildOrthographicProjectionMatrix(0.1f, 100.0f, 20, 1);
    }

    void ShadowPainter::prepare()
    {
        // TODO: Everything is hardcoded here for now. Populate thes from light structure
        // TODO: Light should not be accessed through a render scene.... There should be some other abstraction
        RenderScene *graphicsScene = VulkanRenderer::get()->getRenderScene("graphics");
        const GlobalLight &light = graphicsScene->getGlobalLight();
        Quaint::QVec3 direction = light.getDirection();
        Quaint::QVec4 location = direction * -10.f;
        QMat3x3 modelMatrix = Quaint::lookAt(Quaint::QVec4(0, 0, 0, 1), location, Quaint::QVec3(0, 1, 0));

        Quaint::UniformBufferObject mvp{};

        // VIEW MATRIX
        // Negate translation and tranpose rotation matrix
        Quaint::QMat3x3 invRotation = Quaint::transpose_mf(modelMatrix);
        // Quaint::QVec4 invTranslation = m_transform.col3 * -1.0f;
        Quaint::QVec4 invTranslation;
        invTranslation.x = -Quaint::dot_vf(modelMatrix.col0, location);
        invTranslation.y = -Quaint::dot_vf(modelMatrix.col1, location);
        invTranslation.z = -Quaint::dot_vf(modelMatrix.col2, location);
        invTranslation.w = 1.0f;

        mvp.view = Quaint::makeTransform(invTranslation, invRotation);
        m_lightView = mvp.view;
        G_lightView = mvp.view;

        // PROJECTION MATRIX
        mvp.proj = m_lightProjection;
        G_lightProj = m_lightProjection;
        for (size_t i = 0; i < m_geoInfo.getSize(); ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            VulkanBufferObjectResource *uniformBuffer = (VulkanBufferObjectResource *)m_geoInfo[i].uniformBuffer.get();

            void **region = uniformBuffer->getMappedRegion();

            mvp.model = m_geoInfo[i].model->getTransform();
            memcpy(*region, &mvp, uniformBuffer->getBufferInfo().size);
        }
    }
    
    void ShadowPainter::preRender(RenderScene* scene)
    {

    }

    void ShadowPainter::render(RenderScene *scene)
    {
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        for (auto &info : m_geoInfo)
        {
            // TODO: Should probably bind descriptor sets for each mesh in the model

            // Maybe it's better if this is moved to object that's rendering
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &info.set, 0, nullptr);

            // TODO: Should probably draw commands for individual meshes as they could have different materials
            info.model->draw(scene);
        }
    }

    void ShadowPainter::postRender(RenderScene *scene)
    {
        // Once the shadow rendering is done, transition it's layout so that it can be sampled in the fragment shaders
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;

        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        // TODO: Transition depth texture to SHADE_READ_ONLY_OPTIMAL
        VkImageMemoryBarrier barr{};
        barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barr.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barr.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkImageSubresourceRange range{};
        range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        range.layerCount = 1;
        range.baseArrayLayer = 0;
        range.levelCount = 1;
        range.baseMipLevel = 0;
        barr.subresourceRange = range;
        barr.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barr.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barr.image = vulkanScene->getAttachment("depthBuffer")->As<DepthAttachment>()->texture.getHandle();

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barr);
    }

    void ShadowPainter::AddModel(Model *model)
    {
        // TODO: It's best if model's update function updates the actual buffer data before rendering starts

        // m_models.pushBack(model);
        VkDevice device = VulkanRenderer::get()->getDevice();
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        // Create a buffer and map it
        const uint32_t size = sizeof(Bolt::UniformBufferObject);
        BufferResourceBuilder builder(m_context);
        TBufferImplPtr bufferPtr = std::move(builder.setBufferType(EBufferType::UNIFORM)
                                                 .setDataSize(size)
                                                 .setDataOffset(0)
                                                 .build());

        VulkanBufferObjectResource *mvpResource = static_cast<VulkanBufferObjectResource *>(bufferPtr.get());
        mvpResource->map();

        // Creates descriptor set
        VkDescriptorSetLayout layouts[] = {pipeline->getDescriptorSetLayout()};
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pipeline->getDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        VkDescriptorSet set = VK_NULL_HANDLE;
        VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &set);
        ASSERT_SUCCESS(res, "Failed to allocate desctiptor sets");
        m_geoInfo.pushBack({model, set, std::move(bufferPtr)});

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mvpResource->getBufferhandle();
        bufferInfo.offset = 0;
        bufferInfo.range = mvpResource->getBufferInfo().size;
        VkWriteDescriptorSet mvpWrite{};
        mvpWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpWrite.descriptorCount = 1;
        mvpWrite.dstSet = set;
        mvpWrite.dstBinding = 0;
        mvpWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpWrite.pBufferInfo = &bufferInfo;

        VkWriteDescriptorSet writes[] = {mvpWrite};
        const uint32_t writeCount = sizeof(writes) / sizeof(writes[0]);
        vkUpdateDescriptorSets(device, writeCount, writes, 0, nullptr);
    }

    GeometryPainter::GeometryPainter(Quaint::IMemoryContext *context, const Quaint::QName &pipeline)
        : Painter(context, pipeline), m_geoInfo(context), m_lightsbuffer(nullptr, Deleter<IBufferImpl>(context)), m_lightsMvpBuffer(nullptr, Deleter<IBufferImpl>(context))
    {
    }

    void GeometryPainter::setupLightsData()
    {
        uint32_t size = 0;

        // TODO: This might not be the best way to do it. Maybe it's better to let <something else> update the data?
        size += sizeof(Bolt::GlobalLightData);

        // Plus whatever point light sources we may have
        RenderScene *scene = VulkanRenderer::get()->getRenderScene(m_pipeline->getSceneName());
        auto &pointLights = scene->getPointLights();
        for (int i = 0; pointLights.getSize(); ++i)
        {
            size += sizeof(Bolt::PointLightData);
        }

        assert(size != 0 && "Invalid lights buffer size");

        BufferResourceBuilder builder(m_context);
        m_lightsbuffer = std::move(builder.setBufferType(EBufferType::UNIFORM)
                                       .setDataSize(size)
                                       .setDataOffset(0)
                                       .build());

        VulkanBufferObjectResource *lightBufferResource = static_cast<VulkanBufferObjectResource *>(m_lightsbuffer.get());
        lightBufferResource->map();

        // Lights MVP Buffer
        const uint32_t mvpBufferSize = sizeof(Bolt::UniformBufferObject);
        BufferResourceBuilder lightMvpBuilder(m_context);
        m_lightsMvpBuffer = std::move(lightMvpBuilder.setBufferType(EBufferType::UNIFORM)
                                          .setDataSize(mvpBufferSize)
                                          .setDataOffset(0)
                                          .build());
        VulkanBufferObjectResource *lightMvpbufferResource = static_cast<VulkanBufferObjectResource *>(m_lightsMvpBuffer.get());
        lightMvpbufferResource->map();

        // ShadowMap ref binding
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        VkResult res = vkCreateSampler(device, &info, VulkanRenderer::get()->getAllocationCallbacks(), &m_sampler);
        ASSERT_SUCCESS(res, "[IMGUI]: Failed to create sampler");
    }

    void GeometryPainter::prepare()
    {
        for (auto &info : m_geoInfo)
        {
            info.model->writeImgui();
        }

        // Update descriptors here
        // TODO: Get the binding information from a shader link
        VkDevice device = VulkanRenderer::get()->getDevice();
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
        Bolt::RenderScene* graphicsScene = Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->getRenderScene("graphics");

        VulkanBufferObjectResource *lightMvpbufferResource = static_cast<VulkanBufferObjectResource *>(m_lightsMvpBuffer.get());
        assert(lightMvpbufferResource->isMapped() && "Lights buffer is not mapped. Cannot update the data");
        if (lightMvpbufferResource->isMapped())
        {
            uint32_t offset = 0;
            void **region = lightMvpbufferResource->getMappedRegion();
            Quaint::UniformBufferObject lightMvp{};

            // TODO: Get rid of globals and get this information from the light itself
            lightMvp.model = Quaint::QMat4x4::Identity();
            lightMvp.view = G_lightView;
            lightMvp.proj = G_lightProj;

            memcpy(*region, &lightMvp, lightMvpbufferResource->getBufferInfo().size);
        }

        // Updates lights buffer data
        VulkanBufferObjectResource *lightsBuffer = (VulkanBufferObjectResource *)m_lightsbuffer.get();
        assert(lightsBuffer->isMapped() && "Lights buffer is not mapped. Cannot update the data");

        // TODO: Only update if data changes
        if (lightsBuffer->isMapped())
        {
            uint32_t offset = 0;
            void **region = lightsBuffer->getMappedRegion();
            const void *globalData = graphicsScene->getGlobalLight().getData();
            memcpy(*region, graphicsScene->getGlobalLight().getData(), graphicsScene->getGlobalLight().getDataSize());
            offset += graphicsScene->getGlobalLight().getDataSize();

            auto &pointLights = graphicsScene->getPointLights();
            for (size_t i = 0; i < pointLights.getSize(); ++i)
            {
                void *target = ((char *)(*region) + offset);
                memcpy(target, pointLights[i].getData(), pointLights[i].getDataSize());
                offset += pointLights[i].getDataSize();
            }
        }

        const Camera &camera = RenderModule::get().getBoltRenderer()->getCamera();
        Quaint::UniformBufferObject mvp{};
        mvp.view = camera.getViewMatrix();
        mvp.proj = camera.getProjectionMatrix();
        for (size_t i = 0; i < m_geoInfo.getSize(); ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            VulkanBufferObjectResource *uniformBuffer = (VulkanBufferObjectResource *)m_geoInfo[i].uniformBuffer.get();

            void **region = uniformBuffer->getMappedRegion();

            mvp.model = m_geoInfo[i].model->getTransform();
            memcpy(*region, &mvp, uniformBuffer->getBufferInfo().size);

            // TODO: Add support for multiple materials
            if (m_geoInfo[i].model->getMaterials().getSize() > 0)
            {

                Bolt::MaterialRef material = m_geoInfo[i].model->getMaterials()[0];
                material->update();

                // SimpleMaterial* simpleMaterial = static_cast<SimpleMaterial*>(material.get());

                // VulkanBufferObjectResource* materialBuffer = (VulkanBufferObjectResource*)m_geoInfo[i].materialBuffer.get();
                // region = materialBuffer->getMappedRegion();
                // memcpy(*region, &simpleMaterial->getData(), simpleMaterial->getDataSize());
            }

            bufferInfo.buffer = uniformBuffer->getBufferhandle();
            bufferInfo.offset = 0;
            bufferInfo.range = uniformBuffer->getBufferInfo().size;

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorCount = 1;
            write.dstSet = m_geoInfo[i].set;
            write.dstBinding = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.pBufferInfo = &bufferInfo;

            // vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
    }

    void GeometryPainter::preRender(RenderScene* scene)
    {

    }

    void GeometryPainter::render(RenderScene *scene)
    {
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;

        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        // TODO: Transition depth texture to SHADE_READ_ONLY_OPTIMAL
        //  VkImageMemoryBarrier barr{};
        //  barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        //  barr.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        //  barr.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        //  VkImageSubresourceRange range{};
        //  range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        //  range.layerCount = 1;
        //  range.baseArrayLayer = 0;
        //  range.levelCount = 1;
        //  range.baseMipLevel = 0;
        //  barr.subresourceRange = range;
        //  barr.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        //  barr.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        //  barr.image = vulkanScene->getAttachment("depthBuffer")->As<DepthAttachment>()->texture.getHandle();

        // vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        //     , VK_DEPENDENCY_BY_REGION_BIT
        //     , 0, nullptr
        //     , 0, nullptr
        //     , 1, &barr);

        // TODO: Update lights descriptor here

        for (auto &info : m_geoInfo)
        {
            // TODO: Should probably bind descriptor sets for each mesh in the model

            // Maybe it's better if this is moved to object that's rendering
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &info.set, 0, nullptr);

            // TODO: Should probably draw commands for individual meshes as they could have different materials
            info.model->draw(scene);
        }
    }
    void GeometryPainter::postRender(RenderScene *scene)
    {
    }

    void GeometryPainter::AddModel(Model *model)
    {
        // TODO: It's best if model's update function updates the actual buffer data before rendering starts

        // m_models.pushBack(model);
        VkDevice device = VulkanRenderer::get()->getDevice();
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        // Create a buffer and map it
        const uint32_t size = sizeof(Bolt::UniformBufferObject);
        BufferResourceBuilder builder(m_context);
        TBufferImplPtr bufferPtr = std::move(builder.setBufferType(EBufferType::UNIFORM)
                                                 .setDataSize(size)
                                                 .setDataOffset(0)
                                                 .build());

        VulkanBufferObjectResource *mvpResource = static_cast<VulkanBufferObjectResource *>(bufferPtr.get());
        mvpResource->map();

        // Creates descriptor set

        VkDescriptorSetLayout layouts[] = {pipeline->getDescriptorSetLayout()};
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pipeline->getDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        VkDescriptorSet set = VK_NULL_HANDLE;
        VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &set);
        ASSERT_SUCCESS(res, "Failed to allocate desctiptor sets");
        m_geoInfo.pushBack({model, set, std::move(bufferPtr)});

        // MVP in binding: 0
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mvpResource->getBufferhandle();
        bufferInfo.offset = 0;
        bufferInfo.range = mvpResource->getBufferInfo().size;
        VkWriteDescriptorSet mvpWrite{};
        mvpWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpWrite.descriptorCount = 1;
        mvpWrite.dstSet = set;
        mvpWrite.dstBinding = 0;
        mvpWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpWrite.pBufferInfo = &bufferInfo;

        // Light MVP in binding: 1
        VulkanBufferObjectResource *lightsMvpResource = static_cast<VulkanBufferObjectResource *>(m_lightsMvpBuffer.get());
        VkDescriptorBufferInfo lightMvpBufferInfo{};
        lightMvpBufferInfo.buffer = lightsMvpResource->getBufferhandle();
        lightMvpBufferInfo.offset = 0;
        lightMvpBufferInfo.range = lightsMvpResource->getBufferInfo().size;
        VkWriteDescriptorSet lightMvpWrite{};
        lightMvpWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lightMvpWrite.descriptorCount = 1;
        lightMvpWrite.dstSet = set;
        lightMvpWrite.dstBinding = 1;
        lightMvpWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightMvpWrite.pBufferInfo = &lightMvpBufferInfo;

        // Map the common lights info to binding:2
        VulkanBufferObjectResource *lightsBuffer = static_cast<VulkanBufferObjectResource *>(m_lightsbuffer.get());
        VkDescriptorBufferInfo lightsBufferInfo{};
        lightsBufferInfo.buffer = lightsBuffer->getBufferhandle();
        lightsBufferInfo.offset = 0;
        lightsBufferInfo.range = lightsBuffer->getBufferInfo().size;
        VkWriteDescriptorSet lightsWrite{};
        lightsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lightsWrite.descriptorCount = 1;
        lightsWrite.dstSet = set;
        lightsWrite.dstBinding = 2;
        lightsWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightsWrite.pBufferInfo = &lightsBufferInfo;

        // Shadow map attachment binding: 3
        // TODO: There should be a better way to fetch the shadow map.....
        RenderScene *scene = VulkanRenderer::get()->getRenderScene("scene_lightmap");
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkDescriptorImageInfo desc_image[1] = {};
        desc_image[0].sampler = m_sampler;
        desc_image[0].imageView = vulkanScene->getAttachment("depthBuffer")->As<DepthAttachment>()->texture.getImageView();
        desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet shadowMapWrite = {};
        shadowMapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        shadowMapWrite.dstSet = set;
        shadowMapWrite.descriptorCount = 1;
        shadowMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadowMapWrite.pImageInfo = desc_image;
        shadowMapWrite.dstBinding = 3;

        // TODO: writes for material data

        VkWriteDescriptorSet writes[] = {mvpWrite, lightMvpWrite, lightsWrite, shadowMapWrite};
        const uint32_t writeCount = sizeof(writes) / sizeof(writes[0]);
        vkUpdateDescriptorSets(device, writeCount, writes, 0, nullptr);

        // Material data starts from: 4
        // TODO: Currently only supporting a single material
        auto &materials = model->getMaterials();
        if (materials.getSize() > 0)
        {
            materials[0]->write(set, 4);
        }
    }

    ToneMapPainter::ToneMapPainter(Quaint::IMemoryContext *context, const Quaint::QName &pipeline)
        : Painter(context, pipeline)
        , m_tempCubemap(nullptr, Quaint::Deleter<IImageSamplerImpl>(context))
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        VkResult res = vkCreateSampler(device, &info, VulkanRenderer::get()->getAllocationCallbacks(), &m_sampler);

        Image2dRef img = Image2d::LoadFromFile(Bolt::G_BOLT_DEFAULT_MEMORY, "C:\\Works\\Project-Quaint\\Data\\Textures\\PBR_Floor\\granite_albedo.png", "diffuse");
        Image2dRef img2 = Image2d::LoadFromFile(Bolt::G_BOLT_DEFAULT_MEMORY, "C:\\Works\\Project-Quaint\\Data\\Textures\\PBR_Floor\\granite_normal_ogl.png", "diffuse");
        const unsigned char* dataArray[6] = {img->data(), img2->data(), img->data(), img2->data(), img->data(), img2->data()};

        //TODO: Remove all of this stuff
        CombinedImageSamplerTextureBuilder cubemapBuilder(context);
        m_tempCubemap = cubemapBuilder.buildCubemapFromPixels(dataArray, img->getWidth(), img->getHeight());
        

        ASSERT_SUCCESS(res, "[IMGUI]: Failed to create sampler");
    }

    void ToneMapPainter::UpdateRenderTarget(RenderScene *scene, const Quaint::QName &renderTargetName)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        vulkan::Attachment *renderTargetAttachment = vulkanScene->getAttachment(renderTargetName);

        VkDescriptorSetLayout layouts[] = {pipeline->getDescriptorSetLayout()};
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pipeline->getDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        m_set = VK_NULL_HANDLE;
        VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &m_set);
        ASSERT_SUCCESS(res, "Failed to allocate desctiptor sets");

        VkDescriptorImageInfo desc_image = {};
        desc_image.sampler = m_sampler;
        desc_image.imageView = renderTargetAttachment->As<ImageAttachment>()->texture.getImageView();
        desc_image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet renderTargetWrite = {};
        renderTargetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        renderTargetWrite.dstSet = m_set;
        renderTargetWrite.descriptorCount = 1;
        renderTargetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        renderTargetWrite.pImageInfo = &desc_image;
        renderTargetWrite.dstBinding = 0;

        //TODO: Remove this
        VulkanCombinedImageSamplerResource* cubemapRes = static_cast<VulkanCombinedImageSamplerResource*>(m_tempCubemap.get());
        VkDescriptorImageInfo cubeMapImageInfo = {};
        cubeMapImageInfo.sampler = m_sampler;
        cubeMapImageInfo.imageView = cubemapRes->getTexture()->getImageView();
        cubeMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet cubemapWrite = {};
        cubemapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        cubemapWrite.dstSet = m_set;
        cubemapWrite.descriptorCount = 1;
        cubemapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cubemapWrite.pImageInfo = &cubeMapImageInfo;
        cubemapWrite.dstBinding = 1;

        VkWriteDescriptorSet writes[] = {renderTargetWrite, cubemapWrite};
        const uint32_t writeCount = sizeof(writes) / sizeof(writes[0]);
        vkUpdateDescriptorSets(device, writeCount, writes, 0, nullptr);
    }


    SkyboxPainter::SkyboxPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipelineName)
    : Painter(context, pipelineName)
    , m_uniformbuffer(nullptr, Quaint::Deleter<Bolt::IBufferImpl>(context))
    , m_tempCubemap(nullptr, Quaint::Deleter<Bolt::IImageSamplerImpl>(context))
    , m_model(nullptr, Quaint::Deleter<Bolt::Model>(context))
    , m_dataProviderRef(nullptr, Quaint::Deleter<Bolt::IVertexDataProvider>(context))
    {
        //Create model and it's MVP buffer

        VkDevice device = VulkanRenderer::get()->getDevice();
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
        
        RenderScene* envMapCaptureScene = VulkanRenderer::get()->getRenderScene("scene_envmap_capture");
        VulkanRenderScene *vulkanGraphicsScene = envMapCaptureScene->getRenderSceneImplAs<VulkanRenderScene>();
        CubemapAttachment* attachment = vulkanGraphicsScene->getAttachment("renderTarget")->As<CubemapAttachment>();

        VkCommandBuffer buffer = VulkanRenderer::get()->beginOneTimeCommands(VulkanRenderer::get()->getGraphicsCommanPool());
        VulkanRenderer::get()->transitionImageLayout(attachment->texture.getHandle()
                    , attachment->texture.getCreateInfo().imageInfo.format
                    , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    , VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    , buffer
                    , 0, 1
                    , 0, 6);
        VulkanRenderer::get()->endOneTimeCommands(buffer, VulkanRenderer::get()->getGraphicsCommanPool(), VulkanRenderer::get()->getGraphicsQueue());

        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        VkResult res = vkCreateSampler(device, &info, VulkanRenderer::get()->getAllocationCallbacks(), &m_sampler);

        // Create a buffer for MVP matrices and map it
        const uint32_t size = sizeof(Bolt::UniformBufferObject);
        BufferResourceBuilder builder(m_context);
        m_uniformbuffer = std::move(builder.setBufferType(EBufferType::UNIFORM)
                                                 .setDataSize(size)
                                                 .setDataOffset(0)
                                                 .build());

        VulkanBufferObjectResource *mvpResource = static_cast<VulkanBufferObjectResource *>(m_uniformbuffer.get());
        mvpResource->map();

        // Creates descriptor set
        VkDescriptorSetLayout layouts[] = {pipeline->getDescriptorSetLayout()};
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pipeline->getDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        m_set = VK_NULL_HANDLE;
        res = vkAllocateDescriptorSets(device, &allocInfo, &m_set);
        
        VkDescriptorBufferInfo desc_mvpBuffer = {};
        desc_mvpBuffer.buffer = mvpResource->getBufferhandle();
        desc_mvpBuffer.offset = 0;
        desc_mvpBuffer.range = mvpResource->getBufferInfo().size;

        VkWriteDescriptorSet mvpBufferWrite = {};
        mvpBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpBufferWrite.dstSet = m_set;
        mvpBufferWrite.descriptorCount = 1;
        mvpBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpBufferWrite.pBufferInfo = &desc_mvpBuffer;
        mvpBufferWrite.dstBinding = 0;

        VkDescriptorImageInfo desc_image = {};
        desc_image.sampler = m_sampler;
        desc_image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        desc_image.imageView = attachment->texture.getImageView();

        VkWriteDescriptorSet radianceMapWrite = {};
        radianceMapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        radianceMapWrite.dstSet = m_set;
        radianceMapWrite.descriptorCount = 1;
        radianceMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        radianceMapWrite.pImageInfo = &desc_image;
        radianceMapWrite.dstBinding = 1;

        VkWriteDescriptorSet writes[] = {mvpBufferWrite, radianceMapWrite};
        const uint32_t writeCount = sizeof(writes) / sizeof(writes[0]);
        vkUpdateDescriptorSets(device, writeCount, writes, 0, nullptr);


        //Finally create the model
        CubeModel* cube = QUAINT_NEW(context, CubeModel, context);
        SkyboxVertexDataProvider* dataProvider = QUAINT_NEW(context, SkyboxVertexDataProvider, context, cube);
        m_dataProviderRef.reset(dataProvider); 
        m_model.reset(cube);
        m_model->overrideVertexDataProvider(dataProvider);
        m_model->construct();
    }
    void SkyboxPainter::prepare()
    {
        const Camera &camera = RenderModule::get().getBoltRenderer()->getCamera();

        VkDescriptorBufferInfo bufferInfo{};
        VulkanBufferObjectResource *uniformBuffer = (VulkanBufferObjectResource *)m_uniformbuffer.get();

        {
            void **region = uniformBuffer->getMappedRegion();

            Quaint::UniformBufferObject mvp{};
            mvp.view = camera.getViewMatrix();
            mvp.view.col3 = Quaint::QVec4(); //Effectively places the camera at the center of the world within the cube 
            mvp.proj = camera.getProjectionMatrix();
            mvp.model = Quaint::QMat4x4::Identity();
            //mvp.model = m_model->getTransform();

            memcpy(*region, &mvp, uniformBuffer->getBufferInfo().size);
        }
    }
    void SkyboxPainter::preRender(RenderScene* scene)
    {

    }
    void SkyboxPainter::render(RenderScene* scene)
    {
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;
        
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
        
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &m_set, 0, nullptr);
        
        m_model->draw(scene);
    }
    void SkyboxPainter::postRender(RenderScene* scene)
    {

    }


    CubemapCapturePainter::CubemapCapturePainter(Quaint::IMemoryContext* context, const Quaint::QName& pipelineName)
    : Painter(context, pipelineName)
    , m_uniformbuffer(nullptr, Quaint::Deleter<Bolt::IBufferImpl>(context))
    //, m_tempCubemap(nullptr, Quaint::Deleter<Bolt::IImageSamplerImpl>(context))
    , m_model(nullptr, Quaint::Deleter<Bolt::Model>(context))
    , m_dataProviderRef(nullptr, Quaint::Deleter<Bolt::IVertexDataProvider>(context))
    , m_envMap(nullptr, Quaint::Deleter<Bolt::Image2d>(context))
    {
        //Create model and it's MVP buffer

        VkDevice device = VulkanRenderer::get()->getDevice();
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        m_envMap = Image2d::LoadHDRFromFile(context
                                        , "C:\\Works\\Project-Quaint\\Data\\Textures\\Environment\\autumn_hilly_field_1k.hdr"
                                        , "environmentMap"
                                        , EFormat::R32G32B32A32_SFLOAT);
        m_envMap->construct();

        // Create a buffer for MVP matrices and map it
        const uint32_t size = sizeof(Bolt::UniformBufferObject);
        BufferResourceBuilder builder(m_context);
        m_uniformbuffer = std::move(builder.setBufferType(EBufferType::UNIFORM)
                                                 .setDataSize(size)
                                                 .setDataOffset(0)
                                                 .build());

        VulkanBufferObjectResource *mvpResource = static_cast<VulkanBufferObjectResource *>(m_uniformbuffer.get());
        mvpResource->map();

        // Creates descriptor set
        VkDescriptorSetLayout layouts[] = {pipeline->getDescriptorSetLayout()};
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pipeline->getDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        m_set = VK_NULL_HANDLE;
        VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &m_set);
        
        VkDescriptorBufferInfo desc_mvpBuffer = {};
        // desc_mvpBuffer.buffer = mvpResource->getBufferhandle();
        // desc_mvpBuffer.offset = 0;
        // desc_mvpBuffer.range = mvpResource->getBufferInfo().size;

        // VkWriteDescriptorSet mvpBufferWrite = {};
        // mvpBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // mvpBufferWrite.dstSet = m_set;
        // mvpBufferWrite.descriptorCount = 1;
        // mvpBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // mvpBufferWrite.pBufferInfo = &desc_mvpBuffer;
        // mvpBufferWrite.dstBinding = 0;

        VkWriteDescriptorSet mvpBufferWrite = vulkan::getUniformBufferDescriptorWrite(m_set, &desc_mvpBuffer, mvpResource, 1, 0);

        VulkanCombinedImageSamplerResource* envResource = m_envMap->getImplAs<vulkan::VulkanCombinedImageSamplerResource>();
        VkDescriptorImageInfo imgInfo = {};
        // imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // imgInfo.sampler = envResource->getSampler();
        // imgInfo.imageView = envResource->getTexture()->getImageView();

        // VkWriteDescriptorSet envMapWrite = {};
        // envMapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // envMapWrite.dstSet = m_set;
        // envMapWrite.descriptorCount = 1;
        // envMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // envMapWrite.pImageInfo = &imgInfo;
        // envMapWrite.dstBinding = 1;
        VkWriteDescriptorSet envMapWrite = vulkan::getCombinedImageSamplerDescriptorWrite(m_set, &imgInfo, envResource, 1, 1);

        VkWriteDescriptorSet writes[] = {mvpBufferWrite, envMapWrite};
        const uint32_t writeCount = sizeof(writes) / sizeof(writes[0]);
        vkUpdateDescriptorSets(device, writeCount, writes, 0, nullptr);

        //Finally create the model
        CubeModel* cube = QUAINT_NEW(context, CubeModel, context);
        SkyboxVertexDataProvider* dataProvider = QUAINT_NEW(context, SkyboxVertexDataProvider, context, cube);
        m_dataProviderRef.reset(dataProvider); 
        m_model.reset(cube);
        m_model->overrideVertexDataProvider(dataProvider);
        m_model->construct();

        m_mvp.proj = Quaint::buildPerspectiveProjectionMatrix(0.1f, 100, 90, 1.0f);
        m_mvp.model = Quaint::QMat4x4::Identity();
    }

    CubemapCapturePainter::~CubemapCapturePainter()
    {
        m_model->destroy();
        m_dataProviderRef.release();
        //m_uniformbuffer->destroy();
        //TODO: Free vulkan resources
    }

    void CubemapCapturePainter::prepare()
    {
        const Camera &camera = RenderModule::get().getBoltRenderer()->getCamera();

        VkDescriptorBufferInfo bufferInfo{};
        VulkanBufferObjectResource *uniformBuffer = (VulkanBufferObjectResource *)m_uniformbuffer.get();

        {
            void **region = uniformBuffer->getMappedRegion();

            //Quaint::UniformBufferObject mvp{};
            //mvp.view = camera.getViewMatrix();
            //mvp.proj = camera.getProjectionMatrix();
            //mvp.model = m_model->getTransform();

            memcpy(*region, &m_mvp, uniformBuffer->getBufferInfo().size);
        }
    }
    void CubemapCapturePainter::preRender(RenderScene* scene)
    {

    }
    void CubemapCapturePainter::render(RenderScene* scene)
    {
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;
        
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
        
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &m_set, 0, nullptr);
        
        m_model->draw(scene);
    }
    void CubemapCapturePainter::postRender(RenderScene* scene)
    {

    }
    void CubemapCapturePainter::lookAt(const Quaint::QVec3 direction, const Quaint::QVec3 up)
    {
        m_mvp.view = Quaint::lookAt_CorrectVersion(direction, Quaint::QVec4(), up);

        // Quaint::QVec3 top(0, 1, 0);
        // Quaint::QVec3 topView = m_mvp.view * top;
        // Quaint::QVec3 topRes = m_mvp.proj * topView;
        
        // Quaint::QVec3 bottom(0, -1, 0);
        // Quaint::QVec3 bottomView = m_mvp.view * bottom;
        // Quaint::QVec3 bottomRes = m_mvp.proj * bottomView;
        // bottomRes = m_mvp.proj * top;
    }




    void ToneMapPainter::prepare()
    {

    }

    void ToneMapPainter::preRender(RenderScene *scene)
    {
        RenderScene *graphicsScene = VulkanRenderer::get()->getRenderScene("graphics");
        VulkanRenderScene *vulkanGraphicsScene = graphicsScene->getRenderSceneImplAs<VulkanRenderScene>();

        VulkanRenderScene* currentScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = currentScene->getSceneParams().commandBuffer;

        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        // TODO: Transition depth texture to SHADE_READ_ONLY_OPTIMAL
        VkImageMemoryBarrier barr{};
        barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barr.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barr.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkImageSubresourceRange range{};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.layerCount = 1;
        range.baseArrayLayer = 0;
        range.levelCount = 1;
        range.baseMipLevel = 0;
        barr.subresourceRange = range;
        barr.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barr.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        barr.image = vulkanGraphicsScene->getAttachment("renderTarget")->As<ImageAttachment>()->texture.getHandle();

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            , 0
            , 0, nullptr
            , 0, nullptr
            , 1, &barr);
    }

    void ToneMapPainter::render(RenderScene *scene)
    {
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;

        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &m_set, 0, nullptr);

        vkCmdDraw(cmdBuffer, 6, 1, 0, 0);
    }
    void ToneMapPainter::postRender(RenderScene *scene)
    {
    }

    // IMGGUI Painter
    const int ALIGNMENT = 64;

    ImguiPainter::ImguiPainter(Quaint::IMemoryContext *context, const Quaint::QName &pipeline)
        : Painter(context, pipeline), m_descriptorInfo(context)
    {
        m_oneTimeCommandBuffer = VulkanRenderer::get()->getGraphicsCommandBuffers(1)[0];

        VkDevice device = VulkanRenderer::get()->getDevice();
        // TODO: Move this somewhere else
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        VkResult res = vkCreateSampler(device, &info, VulkanRenderer::get()->getAllocationCallbacks(), &m_sampler);
        ASSERT_SUCCESS(res, "[IMGUI]: Failed to create sampler");

        // TODO: Maybe lazy load
        assert(m_pipeline != nullptr && "Pipeline should be build before assigning to painter");
    }

    void ImguiPainter::ProcessTexture(RenderScene *scene, ImDrawData *data, ImTextureData *textureData)
    {
        ImTextureStatus status = textureData->Status;

        DeviceManager *deviceManager = VulkanRenderer::get()->getDeviceManager();
        VkDevice device = VulkanRenderer::get()->getDevice();

        // Only supporting this operation in render queue
        uint32_t family = deviceManager->getDeviceDefinition().getQueueOfType(EQueueType::Graphics).getQueueFamily();
        VkQueue queue = deviceManager->getDeviceDefinition().getQueueOfType(EQueueType::Graphics).getVulkanQueueHandle();

        if (status == ImTextureStatus::ImTextureStatus_WantCreate)
        {
            // TODO: Use generic Image.h
            // TODO: Use helper functions in Vulkan renderer after getting back the understandings

            // TODO: Create Texuture
            vulkan::VulkanTextureBuilder builder(m_context);

            VkImageViewCreateInfo imageViewInfo{};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = VK_NULL_HANDLE;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            // components field allows to swizzle color channels around. For eg, you can map all channels to red for a monochromatic view
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            // subresource range selects mipmap levels and array layers to be accessible to the view
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;

            // Create texture, back memory and create it's image view
            VulkanTexture texture = builder.setHeight(textureData->Height)
                                        .setWidth(textureData->Width)
                                        .setFormat(VK_FORMAT_R8G8B8A8_UNORM)
                                        .setQueueFamilies(1, &family)
                                        .setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                        .setMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                                        .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                                        .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
                                        .setTiling(VK_IMAGE_TILING_OPTIMAL)
                                        .setImageViewInfo(imageViewInfo)
                                        .setBuildImage()
                                        .setBackingMemory()
                                        .setBuildImageView()
                                        .build();

            // Create descriptor set for it
            VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
            VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

            // Only a single set is supported per layout;
            VkDescriptorSetLayout layouts[] = {pipeline->getDescriptorSetLayout()};

            VkDescriptorSetAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            info.descriptorPool = pipeline->getDescriptorPool();
            info.descriptorSetCount = 1;
            info.pSetLayouts = layouts;

            VkDescriptorSet set = VK_NULL_HANDLE;
            VkResult res = vkAllocateDescriptorSets(VulkanRenderer::get()->getDevice(), &info, &set);
            assert(res == VK_SUCCESS && "Failed to allocate descriptor sets");

            // Update the descriptor set with actual image view
            VkDescriptorImageInfo desc_image[1] = {};
            desc_image[0].sampler = m_sampler;
            desc_image[0].imageView = texture.getImageView();
            desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            VkWriteDescriptorSet write_desc[1] = {};
            write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc[0].dstSet = set;
            write_desc[0].descriptorCount = 1;
            write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_desc[0].pImageInfo = desc_image;
            vkUpdateDescriptorSets(device, 1, write_desc, 0, nullptr);

            ImguiDescriptorSet setInfo{texture, set};
            m_descriptorInfo.pushBack(setInfo);
            textureData->SetTexID((ImTextureID)(*texture.getImageRef()));
        }

        if (status == ImTextureStatus::ImTextureStatus_WantCreate || status == ImTextureStatus::ImTextureStatus_WantDestroy)
        {
            // Update the descriptors
            // Should move the pixel data to staging buffer and then to image view
            const int upload_x = (textureData->Status == ImTextureStatus_WantCreate) ? 0 : textureData->UpdateRect.x;
            const int upload_y = (textureData->Status == ImTextureStatus_WantCreate) ? 0 : textureData->UpdateRect.y;
            const int upload_w = (textureData->Status == ImTextureStatus_WantCreate) ? textureData->Width : textureData->UpdateRect.w;
            const int upload_h = (textureData->Status == ImTextureStatus_WantCreate) ? textureData->Height : textureData->UpdateRect.h;

            VkDeviceMemory stagingGpuMemory;
            VkBuffer stagingBuffer;
            // VulkanRenderer::get()->createBuffer(textureData->GetSizeInBytes(), textureData->GetPixels(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            //, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingGpuMemory, stagingBuffer);

            const uint32_t alignment = 64;
            size_t upload_pitch = upload_w * textureData->BytesPerPixel;
            size_t upload_size = (upload_h * upload_pitch + alignment - 1) & ~(alignment - 1); // TODO: Align data?????

            // Create buffer and backing memory
            VulkanRenderer::get()->createBuffer(upload_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingGpuMemory, stagingBuffer);

            // Copy image data to buffer
            char *map;
            VkResult res = vkMapMemory(device, stagingGpuMemory, 0, upload_size, 0, (void **)(&map));
            ASSERT_SUCCESS(res, "Failed to map memory");
            for (int y = 0; y < upload_h; y++)
            {
                memcpy(map + upload_pitch * y, textureData->GetPixelsAt(upload_x, upload_y + y), (size_t)upload_pitch);
            }
            VkMappedMemoryRange range[1] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = stagingGpuMemory;
            range[0].size = upload_size;
            res = vkFlushMappedMemoryRanges(device, 1, range);
            ASSERT_SUCCESS(res, "[IMGUI]Failed to flush memory ranges to GPU");
            vkUnmapMemory(device, stagingGpuMemory);

            // Move image data from buffer to image view
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(m_oneTimeCommandBuffer, &info);

            VkBufferMemoryBarrier bBar{};
            bBar.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bBar.buffer = stagingBuffer;
            bBar.size = upload_size;
            bBar.offset = 0;
            bBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bBar.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            bBar.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            VkImageMemoryBarrier iBar{};
            iBar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            iBar.image = (VkImage)textureData->GetTexID();
            iBar.oldLayout = (textureData->Status == ImTextureStatus_WantCreate) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            iBar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            iBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.srcAccessMask = (textureData->Status == ImTextureStatus_WantCreate) ? VK_ACCESS_NONE : VK_ACCESS_SHADER_READ_BIT;
            iBar.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            iBar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            iBar.subresourceRange.levelCount = 1;
            iBar.subresourceRange.layerCount = 1;

            // Transition image to receive data from buffer
            vkCmdPipelineBarrier(m_oneTimeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &bBar, 1, &iBar);

            // Perform Copy
            VkBufferImageCopy cpy{};
            cpy.imageOffset.x = upload_x;
            cpy.imageOffset.y = upload_y;
            cpy.imageExtent.width = upload_w;
            cpy.imageExtent.height = upload_h;
            cpy.imageExtent.depth = 1;
            cpy.imageSubresource.layerCount = 1;
            cpy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            vkCmdCopyBufferToImage(m_oneTimeCommandBuffer, stagingBuffer, (VkImage)textureData->GetTexID(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);

            iBar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            iBar.image = (VkImage)textureData->GetTexID();
            iBar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            iBar.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            iBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            iBar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            iBar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            iBar.subresourceRange.levelCount = 1;
            iBar.subresourceRange.layerCount = 1;

            // transition image to shader read
            vkCmdPipelineBarrier(m_oneTimeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &iBar);

            // Finally submit commands
            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &m_oneTimeCommandBuffer;
            res = vkEndCommandBuffer(m_oneTimeCommandBuffer);
            ASSERT_SUCCESS(res, "[IMGUI]: Failed to end command buffer");
            res = vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE);
            ASSERT_SUCCESS(res, "[IMGUI]: Failed to submit queue");

            // Not optimal. Address this once everything starts to fall in place
            vkDeviceWaitIdle(device);

            // Cleanup buffer resources
            vkDestroyBuffer(device, stagingBuffer, VulkanRenderer::get()->getAllocationCallbacks());
            vkFreeMemory(device, stagingGpuMemory, VulkanRenderer::get()->getAllocationCallbacks());
            textureData->SetStatus(ImTextureStatus_OK);
        }

        // If it hasn't been used for arbitraty amount of time
        if (textureData->Status == ImTextureStatus_WantDestroy && textureData->UnusedFrames >= 3)
        {
            for (size_t i = 0; i < m_descriptorInfo.getSize(); ++i)
            {
                ImguiDescriptorSet &info = m_descriptorInfo[i];
                if (info.texture.getHandle() == (VkImage)textureData->GetTexID())
                {
                    VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
                    info.texture.destroy();
                    vkFreeDescriptorSets(device, pipeline->getDescriptorPool(), 1, &info.set);
                    break;
                }
            }
        }
    }

    void ImguiPainter::setupRenderState(RenderScene *scene, ImDrawData *data, VkCommandBuffer cmdBuffer)
    {
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        if (data->TotalVtxCount > 0)
        {
            VkBuffer vertex_buffers[1] = {m_vertexBuffer.buffer};
            VkDeviceSize vertex_offset[1] = {0};
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertex_buffers, vertex_offset);
            vkCmdBindIndexBuffer(cmdBuffer, m_indexBuffer.buffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
        }

        // Setup viewport:
        {
            VkViewport viewport;
            viewport.x = (float)vulkanScene->getRenderOffset().x;
            viewport.y = (float)vulkanScene->getRenderOffset().y;
            viewport.width = (float)vulkanScene->getRenderExtent().width;
            viewport.height = (float)vulkanScene->getRenderExtent().height;
            viewport.minDepth = 0;
            viewport.maxDepth = 1;
            vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
        }

        // Setup scale and translation:
        // Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
        {
            VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
            float scale[2];
            scale[0] = 2.0f / data->DisplaySize.x;
            scale[1] = 2.0f / data->DisplaySize.y;
            float translate[2];
            translate[0] = -1.0f - data->DisplayPos.x * scale[0];
            translate[1] = -1.0f - data->DisplayPos.y * scale[1];
            vkCmdPushConstants(cmdBuffer, pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
            vkCmdPushConstants(cmdBuffer, pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
        }
    }

    size_t align(size_t size)
    {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }
    
    void ImguiPainter::prepare()
    {

    }

    void ImguiPainter::render(RenderScene *scene)
    {
        VulkanGraphicsPipeline *pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        VkDevice device = VulkanRenderer::get()->getDevice();
        // TODO: Move all of this to platform agnostic code
        ImDrawData *data = ImguiHandler::Get()->RenderAndGetDrawData();

        int fb_width = (int)(data->DisplaySize.x * data->FramebufferScale.x);
        int fb_height = (int)(data->DisplaySize.y * data->FramebufferScale.y);
        if (fb_width <= 0 || fb_height <= 0)
            return;

        // Prepares the textures
        for (int i = 0; i < data->Textures->size(); ++i)
        {
            ImTextureData *textureData = (*data->Textures)[i];
            ProcessTexture(scene, data, textureData);
        }

        // TODO: Handle binding data to vertex/fragment buffers
        VulkanRenderScene *vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;

        if (data->TotalVtxCount > 0)
        {
            // Create or resize the vertex/index buffers
            size_t vertex_size = align(data->TotalVtxCount * sizeof(ImDrawVert));
            size_t index_size = align(data->TotalIdxCount * sizeof(ImDrawIdx));
            if (m_vertexBuffer.buffer == VK_NULL_HANDLE || m_vertexBuffer.size < vertex_size)
            {
                VulkanRenderer::get()->createBuffer(vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_vertexBuffer.memory, m_vertexBuffer.buffer);
                m_vertexBuffer.size = vertex_size;
            }
            if (m_indexBuffer.buffer == VK_NULL_HANDLE || m_indexBuffer.size < index_size)
            {
                VulkanRenderer::get()->createBuffer(index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_indexBuffer.memory, m_indexBuffer.buffer);
                m_indexBuffer.size = index_size;
            }

            // Upload vertex/index data into a single contiguous GPU buffer
            ImDrawVert *vtx_dst = nullptr;
            ImDrawIdx *idx_dst = nullptr;
            VkResult err = vkMapMemory(device, m_vertexBuffer.memory, 0, vertex_size, 0, (void **)&vtx_dst);
            ASSERT_SUCCESS(err, "Failed to map vertex memory");
            err = vkMapMemory(device, m_indexBuffer.memory, 0, index_size, 0, (void **)&idx_dst);
            ASSERT_SUCCESS(err, "Failed to map index memory");

            for (const ImDrawList *draw_list : data->CmdLists)
            {
                memcpy(vtx_dst, draw_list->VtxBuffer.Data, draw_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, draw_list->IdxBuffer.Data, draw_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += draw_list->VtxBuffer.Size;
                idx_dst += draw_list->IdxBuffer.Size;
            }
            VkMappedMemoryRange range[2] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = m_vertexBuffer.memory;
            range[0].size = VK_WHOLE_SIZE;
            range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[1].memory = m_indexBuffer.memory;
            range[1].size = VK_WHOLE_SIZE;
            err = vkFlushMappedMemoryRanges(device, 2, range);
            ASSERT_SUCCESS(err, "failed to flush vertex/index buffers");

            vkUnmapMemory(device, m_vertexBuffer.memory);
            vkUnmapMemory(device, m_indexBuffer.memory);
        }

        setupRenderState(scene, data, cmdBuffer);

        ImVec2 clip_off = data->DisplayPos;         // (0,0) unless using multi-viewports
        ImVec2 clip_scale = data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        // Render command lists
        // (Because we merged all buffers into a single one, we maintain our own offset into them)
        VkDescriptorSet last_desc_set = VK_NULL_HANDLE;
        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        for (const ImDrawList *draw_list : data->CmdLists)
        {
            for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd *pcmd = &draw_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != nullptr)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        setupRenderState(scene, data, cmdBuffer);
                    else
                        pcmd->UserCallback(draw_list, pcmd);
                    last_desc_set = VK_NULL_HANDLE;
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    if (clip_min.x < 0.0f)
                    {
                        clip_min.x = 0.0f;
                    }
                    if (clip_min.y < 0.0f)
                    {
                        clip_min.y = 0.0f;
                    }
                    if (clip_max.x > fb_width)
                    {
                        clip_max.x = (float)fb_width;
                    }
                    if (clip_max.y > fb_height)
                    {
                        clip_max.y = (float)fb_height;
                    }
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;

                    // Apply scissor/clipping rectangle
                    VkRect2D scissor;
                    scissor.offset.x = (int32_t)(clip_min.x);
                    scissor.offset.y = (int32_t)(clip_min.y);
                    scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
                    scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
                    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

                    // Bind DescriptorSet with font or user texture
                    VkDescriptorSet desc_set = VK_NULL_HANDLE;

                    // TODO: Get rid of the loop
                    for (size_t i = 0; i < m_descriptorInfo.getSize(); ++i)
                    {
                        if (m_descriptorInfo[i].texture.getHandle() == (VkImage)pcmd->GetTexID())
                        {
                            desc_set = m_descriptorInfo[i].set;
                        }
                    }

                    if (desc_set != VK_NULL_HANDLE && desc_set != last_desc_set)
                        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &desc_set, 0, nullptr);
                    last_desc_set = desc_set;

                    // Draw
                    vkCmdDrawIndexed(cmdBuffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
                }
            }
            global_idx_offset += draw_list->IdxBuffer.Size;
            global_vtx_offset += draw_list->VtxBuffer.Size;
        }

        // Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
        // Our last values will leak into user/application rendering IF:
        // - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
        // - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitly set that state.
        // If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
        // In theory we should aim to backup/restore those values but I am not sure this is possible.
        // We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
        VkRect2D scissor = {{0, 0}, {(uint32_t)fb_width, (uint32_t)fb_height}};
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }

    void ImguiPainter::preRender(RenderScene *scene)
    {
    }
    void ImguiPainter::postRender(RenderScene *scene)
    {
    }

}