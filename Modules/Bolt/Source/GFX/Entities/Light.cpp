#include <GFX/Entities/Light.h>
#include <imgui.h>

namespace Bolt
{
    LightBase::LightBase(ELightType type)
    : m_lightType(type)
    {
    }

    GlobalLight::GlobalLight(Quaint::QName name)
    : LightBase(ELightType::Global)
    {
        setName(name);
    }

    void GlobalLight::setColor(const QVec4& color)
    {
        m_data.color[0] = color.x;
        m_data.color[1] = color.y;
        m_data.color[2] = color.z;
    }

    void GlobalLight::setDirection(const QVec3& direction)
    {
        m_data.direction[0] = direction.x;
        m_data.direction[1] = direction.y;
        m_data.direction[2] = direction.z;
    }

    PointLight::PointLight(Quaint::QName name)
    : LightBase(ELightType::Point)
    {
        setName(name);
    }


    void GlobalLight::writeImgui()
    {
        ImGui::Text("Name: %s", m_name.getBuffer());
        ImGui::SliderFloat4("Color: ", m_data.color, 0.0f, 1.0f, "%.5f");
        ImGui::SliderFloat3("Direction", m_data.direction, 0.0f, 1.0f);
    }

    void PointLight::writeImgui()
    {
        ImGui::Text("Name: %s", m_name.getBuffer());
        //ImGui::SliderFloat4("Color: ", m_data.color, 0.0f, 1.0f, "%.5f");
        //ImGui::SliderFloat3("Direction", m_data.direction, 0.0f, 1.0f);
    }
}