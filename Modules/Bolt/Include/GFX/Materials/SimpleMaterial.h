#ifndef _H_BOLT_SIMPLE_MATERIAL
#define _H_BOLT_SIMPLE_MATERIAL

#include <GFX/Materials/Material.h>
#include <GFX/Data/MaterialData.h>

namespace Bolt
{
    class SimpleMaterial : public Material
    {
    public:
        SimpleMaterial(Quaint::IMemoryContext* context);
        virtual void construct() {};
        virtual void destroy() {};

        void setAmbientColor(const Quaint::QVec3& ambient) { m_data.ambient = ambient; }
        void setDiffuseColor(const Quaint::QVec3& diffuse) { m_data.diffuse = diffuse; }
        void setSpecularColor(const Quaint::QVec3& specular) { m_data.specular = specular; }

        const SimpleMaterialData& getData() const { return m_data;}
        const uint32_t getDataSize() const { return sizeof(SimpleMaterialData); }

        virtual void writeImgui() override;
    private:
        SimpleMaterialData m_data;
    };
};

#endif //_H_BOLT_SIMPLE_MATERIAL