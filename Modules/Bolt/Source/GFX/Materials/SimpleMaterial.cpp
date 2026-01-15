#include <GFX/Materials/SimpleMaterial.h>
#include <imgui.h>
#include <GFX/ResourceBuilder.h>

#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt
{
    SimpleMaterial::SimpleMaterial(Quaint::IMemoryContext* context)
    : Material(context)
    , m_materialBuffer(nullptr, Quaint::Deleter<IBufferImpl>(context))
    {
    }

    void SimpleMaterial::construct()
    {
        BufferResourceBuilder builder(m_context);
        const uint32_t materialDataSize = sizeof(Bolt::SimpleMaterialData);
        m_materialBuffer = std::move(
            builder.setBufferType(EBufferType::UNIFORM)
            .setDataSize(materialDataSize)
            .setDataOffset(0)
            .build()
        );

        m_materialBuffer->map();
    }

    void SimpleMaterial::destroy()
    {
        if(m_materialBuffer.get() != nullptr)
        {
            m_materialBuffer->destroy();
        }
        m_materialBuffer.reset(nullptr);
    }

    
    //TODO: Move this function to a API s pecific file
    void SimpleMaterial::write(VkDescriptorSet set, uint16_t offset)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();

        int idx = 0;
        vulkan::VulkanBufferObjectResource* materialResource = static_cast<vulkan::VulkanBufferObjectResource*>(m_materialBuffer.get());
        assert(materialResource != nullptr && "could not cast");

        VkDescriptorBufferInfo materialBufferInfo{};
        materialBufferInfo.buffer = materialResource->getBufferhandle();
        materialBufferInfo.offset = 0;
        materialBufferInfo.range = materialResource->getBufferInfo().size;

        VkWriteDescriptorSet materialWrite{};
        materialWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        materialWrite.descriptorCount = 1;
        materialWrite.dstSet = set;
        materialWrite.dstBinding = offset + idx;
        materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        materialWrite.pBufferInfo = &materialBufferInfo;

        VkWriteDescriptorSet writes[] = {materialWrite};
        const uint32_t writeCount = sizeof(writes) / sizeof(writes[0]);
        vkUpdateDescriptorSets(device, 1, writes, 0, nullptr);

        ++idx;
    }

    void SimpleMaterial::update()
    {
        void** region = m_materialBuffer->getMappedRegion();
        memcpy(*region, &m_data, getDataSize());
    }

    void SimpleMaterial::writeImgui()
    {
        float ambient[3] = {m_data.ambient.x, m_data.ambient.y, m_data.ambient.z};
        ImGui::SliderFloat3("ambient", ambient, 0, 1);
        m_data.ambient = Quaint::QVec3(ambient);

        float diffuse[3] = {m_data.diffuse.x, m_data.diffuse.y, m_data.diffuse.z};
        ImGui::SliderFloat3("diffuse", diffuse, 0, 1);
        m_data.diffuse = Quaint::QVec3(diffuse);

        float specular[3] = {m_data.specular.x, m_data.specular.y, m_data.specular.z};
        ImGui::SliderFloat3("specular", specular, 0, 1);
        m_data.specular = Quaint::QVec3(specular);

        ImGui::SliderFloat("Shininess", &m_data.shininess, 0, 256);
    }
}