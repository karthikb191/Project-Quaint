#ifndef _H_IMGUI_HANDLER
#define _H_IMGUI_HANDLER

#include <Singleton.h>
#include <imgui.h>

struct ImDrawData;
//TODO: Maybe this could be a different module
namespace Bolt
{
    class Window;
    class ImguiHandler : public Singleton<ImguiHandler>
    {
        DECLARE_SINGLETON(Bolt::ImguiHandler);
    public:
        struct PushConstant
        {
            ImVec2 uScale;
            ImVec2 uTranslate;
        };

        ImguiHandler();
        ~ImguiHandler();
        void Initialize(const Window& window);
        void StartFrame();
        ImDrawData* ImguiHandler::RenderAndGetDrawData();
        void EndFrame();
        void DeInitialize();

    private:
        Quaint::IMemoryContext* GetMemoryContext(){ return m_context; }

        static void* Imgui_Alloc(size_t sz, void* user_data);
        static void Imgui_Free(void* ptr, void* user_data);
    };
}

#endif //_H_IMGUI_HANDLER