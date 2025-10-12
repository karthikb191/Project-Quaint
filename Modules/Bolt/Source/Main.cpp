#include <iostream>
#include <fstream>
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
#include <GFX/Entities/Model.h>
#include <GFX/Entities/Image.h>
#include <GFX/Entities/Painters.h>
#include <EASTL/vector.h>
#include <imgui.h>
#include <ImguiHandler.h>
#include <GFX/Data/LightData.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

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

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main()
{
    //constexpr size_t offset = offsetof(SimpleTriShader, descriptors);

    //constexpr int tt = sizeof("ubo", "sampler", "tete", "trttrsfsdgsdfsdgdg");
    //constexpr int sz = GetCount<int>({1, 2});
    //constexpr Quaint::QName name("test");

    //constexpr int sz = GetCount<Quaint::QCTString<64>>({Quaint::QCTString<64>("tetete"), Quaint::QCTString<64>("tettte")});
    //constexpr int sz = GetCount<Quaint::QName>({Quaint::QName("ubo"), Quaint::QName("sampler")});

    //constexpr auto data = test.descriptors;
    
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

    //Bolt::RenderQuad quad(Quaint::MemoryModule::get().getMemoryManager().getDefaultMemoryContext());

    Bolt::RenderInfo info;
    info.extents = Quaint::QVec2(~0, ~0);
    info.offset = Quaint::QVec2({0, 0});
    info.attachments = Quaint::QArray<Bolt::AttachmentDefinition>(context);
    Bolt::AttachmentDefinition def;
    def.binding = 0;
    def.name = "swapchain";
    def.clearColor = Quaint::QVec4(0.0f, 0.01f, 0.15f, 1.0f);
    def.clearImage = true;
    def.type = Bolt::AttachmentDefinition::Type::Swapchain;
    def.format = Bolt::EFormat::R8G8B8A8_SRGB;
    def.usage = Bolt::EImageUsage::COLOR_ATTACHMENT | Bolt::EImageUsage::COPY_DST; //Hardcoded the same as VulkanSwapchain for now
    info.attachments.pushBack(def);
    def.storePrevious = false;

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

    //TODO: Maybe move construction to a separate function

    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addRenderScene("graphics", info, stages.getSize(), stages.getBuffer());
    
    //This is fine for now, but the structure of this should probably change
    Bolt::RenderScene* scene = Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->getRenderScene("graphics");
    Bolt::GlobalLight globalLight("Simple Global");
    globalLight.setColor({1.0f, 0.0f, 0.0f, 1.0f});
    globalLight.setDirection({0.5f, -0.5f, 0.0f});

    scene->addGlobalLight(globalLight);

    
    def.clearColor = Quaint::QVec4(1.0f, 0.0f, 0.0f, 1.0f);
    def.storePrevious = false;
    //info.extents = Quaint::QVec2(~0, ~0);
    //info.offset = Quaint::QVec2({0, 0});
    info.extents = Quaint::QVec2(256, 560);
    info.offset = {40, 0};
    info.attachments.clear();
    info.attachments.pushBack(def);
    //Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addRenderScene("TESTTEST", info, stages.getSize(), stages.getBuffer());

    //TODO: Add render scene to vulkan renderer through bolt renderer and issue construction

    // Pipeline creation
    Bolt::ShaderDefinition shaderDef{};
    shaderDef.shaders = Quaint::QArray<Bolt::ShaderFileInfo>(context);
    shaderDef.uniforms = Quaint::QArray<Bolt::ShaderUniform>(context);
    shaderDef.pushConstants = Quaint::QArray<Bolt::PushConstant>(context);
    shaderDef.attributeSets = Quaint::QArray<Quaint::QArray<Bolt::ShaderAttributeInfo>>(context);

    shaderDef.shaders.pushBack({"simpleTri.vert", "C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.vert.spv"
        , "main", Bolt::EShaderStage::VERTEX});
    shaderDef.shaders.pushBack({"simpleTri.frag", "C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.frag.spv"
        , "main", Bolt::EShaderStage::FRAGMENT});

    shaderDef.uniforms.pushBack({"Buffer_MVP", Bolt::EShaderResourceType::UNIFORM_BUFFER, Bolt::EShaderStage::VERTEX, 1});
    //shaderDef.uniforms.pushBack({"CIS_TestTexture", Bolt::EShaderResourceType::COMBINED_IMAGE_SAMPLER, Bolt::EShaderStage::FRAGMENT, 1});
    
    shaderDef.uniforms.pushBack({"Lights", Bolt::EShaderResourceType::UNIFORM_BUFFER, Bolt::EShaderStage::FRAGMENT, 1});
    
    
    Quaint::QArray<Bolt::ShaderAttributeInfo> attributes(context);

    attributes.pushBack({"position", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"normal", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"texcoord", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"color", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});

    shaderDef.attributeSets.pushBack(attributes);

    /*Creates pipeline and generate a graphic API specific object*/
    Bolt::Pipeline* pipeline = QUAINT_NEW(context, Bolt::Pipeline, context, Quaint::QName("GeoPipeline"), Quaint::QName("graphics"), 0, shaderDef);
    pipeline->cullBack();
    pipeline->construct();
    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addPipeline(pipeline);

    // IMGUI SETUP. TODO: Should be moved to a different file
    shaderDef = Bolt::ShaderDefinition();
    shaderDef.shaders = Quaint::QArray<Bolt::ShaderFileInfo>(context);
    shaderDef.uniforms = Quaint::QArray<Bolt::ShaderUniform>(context);
    shaderDef.pushConstants = Quaint::QArray<Bolt::PushConstant>(context);
    shaderDef.attributeSets = Quaint::QArray<Quaint::QArray<Bolt::ShaderAttributeInfo>>(context);

    shaderDef.shaders.pushBack({"glsl_shader.vert.vert", "C:\\Works\\Project-Quaint\\Data\\Shaders\\Imgui\\glsl_shader.vert.spv"
        , "main", Bolt::EShaderStage::VERTEX});
    shaderDef.shaders.pushBack({"glsl_shader.frag", "C:\\Works\\Project-Quaint\\Data\\Shaders\\Imgui\\glsl_shader.frag.spv"
        , "main", Bolt::EShaderStage::FRAGMENT});

    shaderDef.uniforms.pushBack({"sTexture", Bolt::EShaderResourceType::COMBINED_IMAGE_SAMPLER, Bolt::EShaderStage::FRAGMENT, 1});
        
    Quaint::QArray<Bolt::ShaderAttributeInfo> attributes2(context);

    attributes2.pushBack({"aPos", 8, Bolt::EFormat::R32G32_SFLOAT});
    attributes2.pushBack({"aCol", 8, Bolt::EFormat::R32G32_SFLOAT});
    attributes2.pushBack({"aColor", 4, Bolt::EFormat::R8G8B8A8_UNORM});
    shaderDef.attributeSets.pushBack(attributes2);

    shaderDef.pushConstants.pushBack({"imgui_pushConstant", Bolt::EShaderStage::VERTEX, sizeof(Bolt::ImguiHandler::PushConstant), 0});
    //TODO: Add support for push constants
    

    //IMGUI pipeline uses the same stage and subpass
    Bolt::Pipeline* imguiPipleline = QUAINT_NEW(context, Bolt::Pipeline, context, Quaint::QName("IMGUIPipeline"), Quaint::QName("graphics"), 0, shaderDef);
    imguiPipleline->enableBlend();
    imguiPipleline->addDynamicStage("viewport");
    imguiPipleline->addDynamicStage("scissor");
    imguiPipleline->construct();
    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addPipeline(imguiPipleline);


    //Creating a painter for a specific pipeline and adding model to it
    Bolt::GeometryPainter* geoPainter = QUAINT_NEW(context, Bolt::GeometryPainter, context, Quaint::QName("GeoPipeline"));
    geoPainter->setupLightsData();


    //TODO: Create IMGUI handler
    Bolt::ImguiHandler::Create(context);
    Bolt::ImguiHandler::Get()->Initialize(Bolt::RenderModule::get().getBoltRenderer()->getWindow());

    //TODO: Create IMGUI painter
    Bolt::ImguiPainter* imguiPainter = QUAINT_NEW(context, Bolt::ImguiPainter, context, Quaint::QName("IMGUIPipeline"));


    //Goal: To create concrete representations of combined image samplers and UniformBuffer

    //Had Horrible memory leak
    Quaint::QArray<Bolt::ModelRef> modelHolder(context);
    auto AddModelFunc = [&]()
    {
        Bolt::MeshRef meshRef(QUAINT_NEW(context, Bolt::QuadMesh, context), Bolt::Deleter<Bolt::Mesh>(context));
        Bolt::Model* modelPtr = QUAINT_NEW(context, Bolt::Model, context, std::move(meshRef));
        Bolt::ModelRef model(modelPtr, Bolt::Deleter<Bolt::Model>(context));
        model->construct();
        geoPainter->AddModel(model.get());
        modelHolder.pushBack(std::move(model));
    };

    for(int i = 0; i < 100; ++i)
    {
        //AddModelFunc();
    }

    //Loading an object
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error;

    //std::fstream stream("C:\\Works\\Project-Quaint\\Data\\Models\\box.obj", ios_base::in | ios_base::binary);
    std::fstream stream("C:\\Works\\Project-Quaint\\Data\\Models\\box.obj", ios_base::in | ios_base::binary);

    bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &error, &stream);
    if(!result)
    {
        std::cout << "Failed to load mesh\n";
    }

    for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
        printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
            static_cast<const double>(attrib.vertices[3 * v + 0]),
            static_cast<const double>(attrib.vertices[3 * v + 1]),
            static_cast<const double>(attrib.vertices[3 * v + 2]));
    }


    for(size_t i = 0; i < shapes.size(); ++i)
    {
        tinyobj::mesh_t loadedMesh = shapes[i].mesh;
        //std::cout << "mesh:" << shapes[i].name << "\n";

        std::vector<int> indices;
        std::vector<Quaint::QVec3> normals;
        std::vector<float> fNormals;

        normals.resize(attrib.vertices.size() / 3, Quaint::QVec3(0, 0, 0));
        for(size_t j = 0; j < loadedMesh.indices.size(); ++j)
        {
            //std::cout << loadedMesh.indices[j].vertex_index << "\n";
            indices.push_back(loadedMesh.indices[j].vertex_index);

            int normalIdx = loadedMesh.indices[j].normal_index;
            
            if(normalIdx != -1)
            {
                normals[loadedMesh.indices[j].vertex_index].x += attrib.normals[normalIdx];
                normals[loadedMesh.indices[j].vertex_index].y += attrib.normals[normalIdx + 1];
                normals[loadedMesh.indices[j].vertex_index].z += attrib.normals[normalIdx + 2];
            }
        }

        for(size_t j = 0; j < normals.size(); ++j)
        {
            if(normals[j].sqrMagnitude() > 0)
            {
                normals[j].normalize();
            }
            fNormals.push_back(normals[j].x);
            fNormals.push_back(normals[j].y);
            fNormals.push_back(normals[j].z);
        }

        //TODO: vertices wont change across meshes for a model. Handle this
        Bolt::Mesh* mesh = QUAINT_NEW(context, Bolt::Mesh, context
            , attrib.vertices.data(), attrib.vertices.size()
            , fNormals.data(), fNormals.size()
            , indices.data(), indices.size()
            , attrib.texcoords.data(), attrib.texcoords.size()
            , 400.f);
            
        Bolt::MeshRef meshRef(mesh, Bolt::Deleter<Bolt::Mesh>(context));
        Bolt::Model* modelPtr = QUAINT_NEW(context, Bolt::Model, context, std::move(meshRef));
        Bolt::ModelRef model(modelPtr, Bolt::Deleter<Bolt::Model>(context));
        
        model->construct();
        geoPainter->AddModel(model.get());
        modelHolder.pushBack(std::move(model));
    }


    attrib.vertices.clear();
    attrib.normals.clear();
    attrib.texcoords.clear();
    shapes.clear();
    materials.clear();

    Bolt::RenderModule::get().getBoltRenderer()->addPainter(geoPainter);
    Bolt::RenderModule::get().getBoltRenderer()->addPainter(imguiPainter);

    //TODO: Loop through application module 
    //TODO: Move this to Application Module
    MSG msg = { };
    bool done = false;
    while (true)
    {
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        //TODO: update IMGUI Handler
        Bolt::ImguiHandler::Get()->StartFrame();

        //ImGui::ShowDemoWindow();
        ImGui::LabelText("TESTSTEST", "It Works!!!");
        ImGui::LabelText("TESTSTEST", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        ImGui::LabelText("TESTSTEST", "abcdefghijklmnopqrstuvwxyz 123456789");
        ImGui::ShowDemoWindow();

        Bolt::RenderModule::get().getBoltRenderer()->update();
        Bolt::ImguiHandler::Get()->EndFrame();
    }

    //Clear all models
    for(int i = 0; i < modelHolder.getSize(); ++i)
    {
        modelHolder[i]->destroy();
        modelHolder[i].release();
    }

    //TODO: VERY BAD. Convert to RAII
    QUAINT_DELETE(context, geoPainter);
    QUAINT_DELETE(context, imguiPainter);

    pipeline->destroy();
    imguiPipleline->destroy();

    Bolt::ImguiHandler::Get()->Shutdown();

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