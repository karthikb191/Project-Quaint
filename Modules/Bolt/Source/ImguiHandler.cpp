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
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
        //main_scale = 2;

        ImGui::SetAllocatorFunctions(Imgui_Alloc, Imgui_Free);
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
        
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

        io.Fonts->Build();
        //ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Works\\Project-Quaint\\External\\Libs\\imgui-1.92.3\\misc\\fonts\\DroidSans.ttf"
        //, 0.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
        //ImGui::PushFont(font);

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
        //TODO: Free imgui context
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