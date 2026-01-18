#include <GFX/Materials/PBRMaterial.h>
#include <BoltMemoryProvider.h>
#include <imgui.h>
#include <GFX/ResourceBuilder.h>
#include <Types/QVector.h>

#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>

namespace Bolt
{
    const uint32_t PBR_PROPERTIES_SIZE = sizeof(PBRProperties);

    PBRMaterial::PBRMaterial()
    : Material(Bolt::G_BOLT_DEFAULT_MEMORY)
    , m_diffuseMap(nullptr, Quaint::Deleter<Image2d>(G_BOLT_DEFAULT_MEMORY))
    , m_normalMap(nullptr, Quaint::Deleter<Image2d>(G_BOLT_DEFAULT_MEMORY))
    , m_metallicMap(nullptr, Quaint::Deleter<Image2d>(G_BOLT_DEFAULT_MEMORY))
    , m_roughnessMap(nullptr, Quaint::Deleter<Image2d>(G_BOLT_DEFAULT_MEMORY))
    , m_propertiesBuffer(nullptr, Quaint::Deleter<IBufferImpl>(G_BOLT_DEFAULT_MEMORY))
    {
    }

    void PBRMaterial::construct()
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

        if(m_diffuseMap.get())
        {
            m_diffuseMap->construct();
        }
        if(m_normalMap.get())
        {
            m_normalMap->construct();
        }
        if(m_metallicMap.get())
        {
            m_metallicMap->construct();
        }
        if(m_roughnessMap.get())
        {
            m_roughnessMap->construct();
        }

        BufferResourceBuilder builder(Bolt::G_BOLT_DEFAULT_MEMORY);
        m_propertiesBuffer = std::move(
            builder.setBufferType(EBufferType::UNIFORM)
            .setDataSize(PBR_PROPERTIES_SIZE)
            .setDataOffset(0)
            .build()
        );

        m_propertiesBuffer->map();
    }
    void PBRMaterial::destroy()
    {
        if(m_diffuseMap.get() != nullptr)
        {
            m_diffuseMap->destroy();
            m_diffuseMap.reset(nullptr);
        }
        if(m_normalMap.get() != nullptr)
        {
            m_normalMap->destroy();
            m_normalMap.reset(nullptr);
        }
        if(m_metallicMap.get() != nullptr)
        {
            m_metallicMap->destroy();
            m_metallicMap.reset(nullptr);
        }
        if(m_roughnessMap.get() != nullptr)
        {
            m_roughnessMap->destroy();
            m_roughnessMap.reset(nullptr);
        }

        if(m_propertiesBuffer.get() != nullptr)
        {
            m_propertiesBuffer->destroy();
            m_propertiesBuffer.reset(nullptr);
        }
    }

    void PBRMaterial::write(VkDescriptorSet set, uint16_t offset)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();

        vulkan::VulkanCombinedImageSamplerResource* texture = nullptr;
        
        uint8_t bindingIdx = 0;
        uint8_t descImageIdx = 0;
        Quaint::QVector<VkWriteDescriptorSet> writes(BOLT_ALLOCATOR);
        VkDescriptorBufferInfo desc_buffer{};
        VkDescriptorImageInfo desc_image[4] = {0};

        if(m_propertiesBuffer.get() != nullptr)
        {
            vulkan::VulkanBufferObjectResource* bufferRes = static_cast<vulkan::VulkanBufferObjectResource*>(m_propertiesBuffer.get());
            desc_buffer.buffer = bufferRes->getBufferhandle();
            desc_buffer.offset = 0;
            desc_buffer.range = bufferRes->getBufferInfo().size;

            VkWriteDescriptorSet diffuseWrite = {};
            diffuseWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            diffuseWrite.dstSet = set;
            diffuseWrite.descriptorCount = 1;
            diffuseWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            diffuseWrite.pBufferInfo = &desc_buffer;
            diffuseWrite.dstBinding = offset + bindingIdx;
            writes.push_back(diffuseWrite);
        }

        ++bindingIdx;
        if(m_diffuseMap.get() != nullptr)
        {    
            texture = m_diffuseMap->getImplAs<vulkan::VulkanCombinedImageSamplerResource>();

            desc_image[descImageIdx].sampler = m_sampler;
            desc_image[descImageIdx].imageView = texture->getTexture()->getImageView();
            desc_image[descImageIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet diffuseWrite = {};
            diffuseWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            diffuseWrite.dstSet = set;
            diffuseWrite.descriptorCount = 1;
            diffuseWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            diffuseWrite.pImageInfo = desc_image + descImageIdx;
            diffuseWrite.dstBinding = offset + bindingIdx;
            writes.push_back(diffuseWrite);
        }

        ++bindingIdx;
        ++descImageIdx;
        if(m_normalMap.get() != nullptr)
        {    
            texture = m_normalMap->getImplAs<vulkan::VulkanCombinedImageSamplerResource>();

            desc_image[descImageIdx].sampler = m_sampler;
            desc_image[descImageIdx].imageView = texture->getTexture()->getImageView();
            desc_image[descImageIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet diffuseWrite = {};
            diffuseWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            diffuseWrite.dstSet = set;
            diffuseWrite.descriptorCount = 1;
            diffuseWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            diffuseWrite.pImageInfo = desc_image + descImageIdx;
            diffuseWrite.dstBinding = offset + bindingIdx;
            writes.push_back(diffuseWrite);
        }
        
        ++bindingIdx;
        ++descImageIdx;
        if(m_metallicMap.get() != nullptr)
        {    
            texture = m_metallicMap->getImplAs<vulkan::VulkanCombinedImageSamplerResource>();

            desc_image[descImageIdx].sampler = m_sampler;
            desc_image[descImageIdx].imageView = texture->getTexture()->getImageView();
            desc_image[descImageIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet diffuseWrite = {};
            diffuseWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            diffuseWrite.dstSet = set;
            diffuseWrite.descriptorCount = 1;
            diffuseWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            diffuseWrite.pImageInfo = desc_image + descImageIdx;
            diffuseWrite.dstBinding = offset + bindingIdx;
            writes.push_back(diffuseWrite);
        }
        
        ++bindingIdx;
        ++descImageIdx;
        if(m_roughnessMap.get() != nullptr)
        {    
            texture = m_roughnessMap->getImplAs<vulkan::VulkanCombinedImageSamplerResource>();

            desc_image[descImageIdx].sampler = m_sampler;
            desc_image[descImageIdx].imageView = texture->getTexture()->getImageView();
            desc_image[descImageIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet diffuseWrite = {};
            diffuseWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            diffuseWrite.dstSet = set;
            diffuseWrite.descriptorCount = 1;
            diffuseWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            diffuseWrite.pImageInfo = desc_image + descImageIdx;
            diffuseWrite.dstBinding = offset + bindingIdx;
            writes.push_back(diffuseWrite);
        }

        vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
    }
    void PBRMaterial::update()
    {
        void** region = m_propertiesBuffer->getMappedRegion();
        if(region != nullptr)
        {
            memcpy(*region, &m_properties, PBR_PROPERTIES_SIZE);
        }
    }
    
    void PBRMaterial::loadDiffuseMap(Quaint::QPath path)
    {
        m_diffuseMap = Image2d::LoadFromFile(Bolt::G_BOLT_DEFAULT_MEMORY, path, "diffuse");
    }
    void PBRMaterial::loadNormalMap(Quaint::QPath path)
    {
        m_normalMap = Image2d::LoadFromFile(Bolt::G_BOLT_DEFAULT_MEMORY, path, "normal");
    }
    void PBRMaterial::loadMetallicMap(Quaint::QPath path)
    {
        m_metallicMap = Image2d::LoadFromFile(Bolt::G_BOLT_DEFAULT_MEMORY, path, "metallic");
    }
    void PBRMaterial::loadRoughnessMap(Quaint::QPath path)
    {
        m_roughnessMap = Image2d::LoadFromFile(Bolt::G_BOLT_DEFAULT_MEMORY, path, "roughness");
    }

    void PBRMaterial::writeImgui()
    {
        float ambient[3] = {m_properties.albedo.x, m_properties.albedo.y, m_properties.albedo.z};
        ImGui::SliderFloat3("Albedo", ambient, 0, 1);
        m_properties.albedo = Quaint::QVec3(ambient);
        
        ImGui::SliderFloat("Metallic", &m_properties.metallic, 0, 1);
        ImGui::SliderFloat("Roughness", &m_properties.roughness, 0, 1);
    }
}