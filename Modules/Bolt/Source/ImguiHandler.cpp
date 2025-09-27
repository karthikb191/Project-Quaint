#include <ImguiHandler.h>
#include <GFX/Window.h>
#include <GFX/Interface/IWindow_Impl.h>
#include <MemCore/GlobalMemoryOverrides.h>

//TODO: Make it platform agnostic
#include <imgui/imgui_impl_win32.h>

DEFINE_SINGLETON(Bolt::ImguiHandler);
namespace Bolt
{
    ImguiHandler::ImguiHandler()
    {
        
    }

    ImguiHandler::~ImguiHandler()
    {
        DeInitialize();
    }

    void ImguiHandler::Initialize(const Window& window)
    {
        ImGui::SetAllocatorFunctions(Imgui_Alloc, Imgui_Free);
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

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

    void* ImguiHandler::Imgui_Alloc(size_t sz, void* user_data)
    {
        IM_UNUSED(user_data);
        return QUAINT_ALLOC_MEMORY(ImguiHandler::Get()->GetMemoryContext(), sz);
    }
    void ImguiHandler::Imgui_Free(void* ptr, void* user_data)
    {
        IM_UNUSED(user_data);
        QUAINT_DEALLOC_MEMORY(ImguiHandler::Get()->GetMemoryContext(), ptr);
    }
}