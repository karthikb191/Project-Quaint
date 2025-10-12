#include <GFX/Entities/RenderScene.h>
#include <ImguiHandler.h>

namespace Bolt
{
    void RenderScene::update()
    {
        //Currently only writes to IMGUI
        char name[1024] = {'\0'};
        sprintf_s(name, "Render Scene: %s", m_name.getBuffer());
        if (ImGui::CollapsingHeader(name))
        {
            if (ImGui::TreeNode("Global Light"))
            {
                m_globalLight.writeImgui();  
                ImGui::TreePop();      
            }

            if (ImGui::TreeNode("Point Lights"))
            {   
                for(size_t i = 0; i < m_pointLights.getSize(); ++i)
                {
                    m_pointLights[i].writeImgui();
                }
                ImGui::TreePop();
            }
        }
    }
}