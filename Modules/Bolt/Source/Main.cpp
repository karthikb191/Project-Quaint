#include <iostream>
#include <vulkan/vulkan.h>
#include <BoltRenderer.h>
#include <Types/QFastArray.h>
#include <Types/QArray.h>
#include <Types/QStaticString.h>
#include <Types/QStaticString_W.h>
#include <Types/QCTString.h>

#include <MemoryModule.h>
#include <LoggerModule.h>
#include <RenderModule.h>
#include <Core/Camera.h>
#include <GFX/Entities/RenderObject.h>
#include <GFX/Entities/RenderScene.h>
#include <GFX/Entities/Pipeline.h>

namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);
    
    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}

namespace Bolt
{
    CREATE_MODULE(RenderModule);
    INIT_MODULE(RenderModule);
}

int main()
{
    std::cout << "Hello Renderer!" << std::endl;

    constexpr auto str1 = Quaint::createCTString("Test String");
    constexpr auto str2 = Quaint::createCTString("Test Strigg");
    constexpr bool res = str1.compare(str2);

    constexpr auto concatStr = str1.concat(str2);

    constexpr bool res2 = concatStr.compare("Test StringTest Strigg");
    if(res2)
    {
        std::cout << "This is working";
    }
    else
    {
        std::cout << "This is not working!";
    }

    std::cout << concatStr.getBuffer();

    Quaint::QStaticString_W<64> testStr(L"Test Test");
    testStr.length();

    Quaint::IMemoryContext* context = Quaint::MemoryModule::get().getMemoryManager().getDefaultMemoryContext();
    Bolt::RenderModule::get().start(Quaint::MemoryModule::get().getMemoryManager().getDefaultMemoryContext());

    //TODO: construct swapchain
    const Bolt::IWindow_Impl_Win* window = Bolt::RenderModule::get().getBoltRenderer()->getWindow().getWindowsWindow();
    RECT rect;
    GetWindowRect(window->getWindowHandle(), &rect);
    uint32_t width = rect.right - rect.left;
    uint32_t height = rect.bottom - rect.top;

    Bolt::RenderQuad quad(Quaint::MemoryModule::get().getMemoryManager().getDefaultMemoryContext());

    Bolt::RenderInfo info;
    info.extents = Quaint::QVec2(-1, -1);
    info.offset = Quaint::QVec2({0, 0});
    info.attachments = Quaint::QArray<Bolt::AttachmentDefinition>(context);
    Bolt::AttachmentDefinition def;
    def.binding = 0;
    def.name = "swapchain";
    def.clearColor = Quaint::QVec4(.0f, 1.0f, 0.0f, 1.0f);
    def.clearImage = true;
    def.type = Bolt::AttachmentDefinition::Type::Swapchain;
    def.format = Bolt::EFormat::R8G8B8A8_SRGB;
    def.usage = Bolt::EImageUsage::COLOR_ATTACHMENT | Bolt::EImageUsage::COPY_DST; //Hardcoded the same as VulkanSwapchain for now
    info.attachments.pushBack(def);

    Quaint::QArray<Bolt::RenderStage> stages(context);
    Bolt::RenderStage stage;
    stage.attachmentRefs = Quaint::QArray<Bolt::RenderStage::AttachmentRef>(context);
    stage.index = 0;

    /* Attachment references in each sub-pass */
    Bolt::RenderStage::AttachmentRef ref{};
    ref.binding = 0;
    ref.attachmentName = "swapchain";
    stage.attachmentRefs.pushBack(ref);
    stage.dependentStage = ~0;
    stages.pushBack(stage);

    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addRenderScene("graphics", info, stages.getSize(), stages.getBuffer());
    
    def.clearColor = Quaint::QVec4(1.0f, 0.0f, 0.0f, 1.0f);
    info.extents = Quaint::QVec2(256, 560);
    info.offset = {40, 0};
    info.attachments.clear();
    info.attachments.pushBack(def);
    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addRenderScene("graphics2", info, stages.getSize(), stages.getBuffer());

    //TODO: Add render scene to vulkan renderer through bolt renderer and issue construction

    // Pipeline creation
    // Pipeline(name, scene, stageIndex, shaderDefinition, PrimitiveInformation, BlendInformation)
    // 1. Create a pipeline from shader

    //TODO: Loop through application module 
    //TODO: Move this to Application Module
    MSG msg = { };
    while (true)
    {
        bool quitApplication = false;

        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            case WM_QUIT:
                quitApplication = true;
                break;
            case WM_PAINT:
                std::cout << "Painting!!!!\n"; 
                break; 
            
            default:
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(quitApplication) break;

        Bolt::RenderModule::get().getBoltRenderer()->update();
    }

    Bolt::RenderModule::get().stop();
    Bolt::RenderModule::shutdown();
    
    //SHUTDOWN_MODULE(RenderModule);
    //Bolt::BoltRenderer::get()->shutdown();
    
    return 0;
}


/*
Some Learnings:
1. Dont hesitate to create a new structure or class. Obviously, it's not okay to create a class/struct for every single thing, but
the good practise is to be flexible.

*/