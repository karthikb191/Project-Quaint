#ifndef _H_IMGUI_HANDLER
#define _H_IMGUI_HANDLER

#include <Singleton.h>

struct ImDrawData;
//TODO: Maybe this could be a different module
namespace Bolt
{
    class Window;
    class ImguiHandler : public Singleton<ImguiHandler>
    {
        DECLARE_SINGLETON(Bolt::ImguiHandler);
    public:
        ImguiHandler(Quaint::IMemoryContext* context);
        ~ImguiHandler();
        void Initialize(const Window& window);
        void StartFrame();
        ImDrawData* ImguiHandler::RenderAndGetDrawData();
        void EndFrame();
        void DeInitialize();

    private:
        Quaint::IMemoryContext* m_context;
    };
}

#endif //_H_IMGUI_HANDLER