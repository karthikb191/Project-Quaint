#include <GFX/Materials/Material.h>
#include <GFX/Data/MaterialData.h>
#include <GFX/Entities/Image.h>

namespace Bolt
{
    struct alignas(16) PBRProperties
    {
        Quaint::QVec4 albedo = {1.0f, 1.0f, 1.0f, 1.0f};
        float metallic = 0.f;
        float roughness = 0.5f;
        
        float pad1 = 0;
        float pas2 = 0;
    };

    class PBRMaterial : public Material
    {
    public:
        PBRMaterial();

        void loadDiffuseMap(Quaint::QPath path);
        void loadNormalMap(Quaint::QPath path);
        void loadMetallicMap(Quaint::QPath path);
        void loadRoughnessMap(Quaint::QPath path);

        virtual void construct() override;
        virtual void destroy() override;
        virtual void write(VkDescriptorSet set, uint16_t offset) override;
        virtual void update() override;
        virtual void writeImgui();

    private:
        Image2dRef          m_diffuseMap;
        Image2dRef          m_normalMap;
        Image2dRef          m_metallicMap;
        Image2dRef          m_roughnessMap;
        PBRProperties       m_properties;

        //TODO: Move it to API specific file
        VkSampler           m_sampler;
        TBufferImplPtr      m_propertiesBuffer;
    };
}