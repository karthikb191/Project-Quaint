#include <GFX/Materials/SimpleMaterial.h>
#include <imgui.h>

namespace Bolt
{
    SimpleMaterial::SimpleMaterial(Quaint::IMemoryContext* context)
    : Material(context)
    {

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