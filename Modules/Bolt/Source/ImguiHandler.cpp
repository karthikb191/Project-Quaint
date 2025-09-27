#include <ImguiHandler.h>
#include <GFX/Window.h>
#include <GFX/Interface/IWindow_Impl.h>

//TODO: Make it platform agnostic
#include <imgui/imgui_impl_win32.h>

DEFINE_SINGLETON(Bolt::ImguiHandler);
namespace Bolt
{
    ImguiHandler::ImguiHandler(Quaint::IMemoryContext* context)
    : m_context(context)
    {
        
    }

    ImguiHandler::~ImguiHandler()
    {
        DeInitialize();
    }

    void ImguiHandler::Initialize(const Window& window)
    {
        IWindow_Impl_Win* widowsWindow = window.getWindowsWindow();
        bool result = ImGui_ImplWin32_Init(widowsWindow->getWindowHandle());

        assert(result && "Could not initialize IMGUI");
    }

    void ImguiHandler::StartFrame()
    {
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    ImDrawData* ImguiHandler::RenderAndGetDrawData()
    {
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        return drawData;
    }

    void ImguiHandler::EndFrame()
    {
        //Unused
    }

    void ImguiHandler::DeInitialize()
    {
        ImGui_ImplWin32_Shutdown();
    }


}