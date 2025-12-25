#include <iostream>
#include <fstream>
#include <vulkan/vulkan.h>
#include <BoltRenderer.h>
#include <Types/QFastArray.h>
#include <Types/QArray.h>
#include <Types/QStaticString.h>
#include <Types/QStaticString_W.h>
#include <Types/QCTString.h>

#include <BoltMemoryProvider.h>
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
#include <GFX/Materials/SimpleMaterial.h>

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


void LoadModelWithSmoothNormals(Quaint::IMemoryContext* context, tinyobj::attrib_t& attrib
    , std::vector<tinyobj::shape_t>& shapes
    , std::vector<tinyobj::material_t>& materials
    , Bolt::GeometryPainter* geoPainter
    , Quaint::QArray<Bolt::ModelRef>& modelHolder
    , const Quaint::QVec4& translation
    , const float scale)
{
    Bolt::Model* modelPtr = QUAINT_NEW(context, Bolt::Model, context);
    Bolt::ModelRef model(modelPtr, Bolt::Deleter<Bolt::Model>(context));

    for(size_t i = 0; i < shapes.size(); ++i)
    {
        tinyobj::mesh_t loadedMesh = shapes[i].mesh;
        //std::cout << "mesh:" << shapes[i].name << "\n";

        std::vector<int> indices;
        std::vector<Quaint::QVec3> normals;
        std::vector<float> fNormals;
        std::vector<float> vertices;

        normals.resize(attrib.vertices.size() / 3, Quaint::QVec3(0, 0, 0));

        //TODO: Get a new model with decent normals :(
        std::map<int, std::vector<int>> normalMap;
        for(size_t j = 0; j < loadedMesh.indices.size(); ++j)
        {
            //std::cout << loadedMesh.indices[j].vertex_index << "\n";
            indices.push_back(loadedMesh.indices[j].vertex_index);

            int normalIdx = loadedMesh.indices[j].normal_index;

            if(normalIdx != -1)
            {
                int vertexIdx = loadedMesh.indices[j].vertex_index;
                if(normalMap.count(normalIdx) == 0)
                {
                    normalMap.insert({vertexIdx, {}});
                }

                normalIdx *= 3;
                if(std::find(normalMap[vertexIdx].begin(), normalMap[vertexIdx].end(), normalIdx) == normalMap[vertexIdx].end())
                {
                    normalMap[vertexIdx].push_back(normalIdx);
                    normals[vertexIdx].x += attrib.normals[normalIdx];
                    normals[vertexIdx].y += attrib.normals[normalIdx + 1];
                    normals[vertexIdx].z += attrib.normals[normalIdx + 2];
                }
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
        model->addMesh(attrib.vertices.data(), attrib.vertices.size()
            , fNormals.data(), fNormals.size()
            , indices.data(), indices.size()
            , attrib.texcoords.data(), attrib.texcoords.size()
            , scale);
    }

    model->setTranslation(translation);
    model->construct();
    geoPainter->AddModel(model.get());
    modelHolder.pushBack(std::move(model));
}

void LoadModelWithPerFaceNormals(Quaint::IMemoryContext* context, tinyobj::attrib_t& attrib
    , std::vector<tinyobj::shape_t>& shapes
    , std::vector<tinyobj::material_t>& materials
    , Bolt::GeometryPainter* geoPainter
    , Quaint::QArray<Bolt::ModelRef>& modelHolder
    , const Quaint::QVec4& translation
    , const float scale
    , const Quaint::QName& name = "")
{
    Bolt::Model* modelPtr = QUAINT_NEW(context, Bolt::Model, context);
    Bolt::ModelRef model(modelPtr, Bolt::Deleter<Bolt::Model>(context));
    model->setName(name);

    Bolt::MaterialRef simpleMaterial = Quaint::makeShared<Bolt::SimpleMaterial>(context);
    simpleMaterial.reset(QUAINT_NEW(context, Bolt::SimpleMaterial, context));

    size_t totalIndexOffset = 0;
    for(size_t i = 0; i < shapes.size(); ++i)
    {
        tinyobj::mesh_t loadedMesh = shapes[i].mesh;
        //std::cout << "mesh:" << shapes[i].name << "\n";

        std::vector<int> indices;
        std::vector<Quaint::QVec3> normals;
        std::vector<float> fNormals;
        std::vector<float> vertices;

        size_t index_offset = 0;
        for (size_t f = 0; f < loadedMesh.num_face_vertices.size(); f++) {
            size_t fnum = loadedMesh.num_face_vertices[f];

            bool needNormalCalculation = false;
            Quaint::QVec3 verts[3];
            for(size_t v = 0; v < fnum; v++)
            {
                tinyobj::index_t idx = loadedMesh.indices[index_offset + v];
                
                //Get the actual vertex from this
                uint32_t vertexIdx = idx.vertex_index * 3;
                vertices.push_back(attrib.vertices[vertexIdx]);
                vertices.push_back(attrib.vertices[vertexIdx + 1]);
                vertices.push_back(attrib.vertices[vertexIdx + 2]);

                verts[v].x = attrib.vertices[vertexIdx];
                verts[v].y = attrib.vertices[vertexIdx + 1];
                verts[v].z = attrib.vertices[vertexIdx + 2];

                if(idx.normal_index != -1)
                {
                    uint32_t normalIdx = idx.normal_index * 3;
                    fNormals.push_back(attrib.normals[normalIdx]);
                    fNormals.push_back(attrib.normals[normalIdx + 1]);
                    fNormals.push_back(attrib.normals[normalIdx + 2]);
                }
                else
                {
                    needNormalCalculation =  true;
                }
            }

            if(needNormalCalculation)
            {
                Quaint::QVec3 edge1 = verts[1] - verts[0];
                Quaint::QVec3 edge2 = verts[2] - verts[1];

                Quaint::QVec3 normal = Quaint::cross_vf(edge1, edge2);

                for(size_t v = 0; v < fnum; v++)
                {
                    fNormals.push_back(normal.x);
                    fNormals.push_back(normal.y);
                    fNormals.push_back(normal.z);
                }
            }
            
            indices.push_back(index_offset + 0);
            indices.push_back(index_offset + 1);
            indices.push_back(index_offset + 2);

            index_offset += fnum;

            //totalIndexOffset += fnum;
            //fNormals.push_back()
        }

        model->addMesh(vertices.data(), vertices.size()
            , fNormals.data(), fNormals.size()
            , indices.data(), indices.size()
            , attrib.texcoords.data(), attrib.texcoords.size()
            , scale
            , simpleMaterial);
    }

    model->setTranslation(translation);

    model->construct();
    geoPainter->AddModel(model.get());
    modelHolder.pushBack(std::move(model));
}



// Delete after setting up testing framework for core
#include <Types/QAllocator.h>
#include <Types/QVector.h>
int main()
{
    using namespace Quaint;
    Quaint::IMemoryContext* context = Bolt::G_BOLT_DEFAULT_MEMORY;

    Quaint::QAllocator<int> intAllocator(context, "testIntAllocator");


    //Check validity of allocator
    cout << "QVector(size) default construction test: " << endl;
    Quaint::QVector<int> defVector;
    std::cout << "Size: " << defVector.size() << " : Expected 0" << endl;

    
    cout << "QVector(size) constructor test: " << endl;
    Quaint::QVector<int> intVector(10);
    for(size_t i = 0; i < intVector.size(); ++i)
    {
        cout << intVector[i] << " ";
    }
    cout << endl;
    cout << "QVector(size) assignment test: " << endl;
    for(size_t i = 0; i < intVector.size(); ++i)
    {
        intVector[i] = i;
    }
    for(size_t i = 0; i < intVector.size(); ++i)
    {
         cout << intVector[i] << " ";
    }
    cout << endl;

    //Constructor with allocator
    cout << endl;
    QVector<int, QAllocator<int>> intWithAllocator(intAllocator);
    std::cout << "Size: " << intWithAllocator.size() << " : Expected 0" << endl;
    
    cout << "QVector(allocator, size) assignment test: " << endl;
    QVector<int, QAllocator<int>> intWithAllocator2(intAllocator, 20);
    for(size_t i = 0; i < intWithAllocator2.size(); ++i)
    {
        intWithAllocator2[i] = i;
    }
    for(size_t i = 0; i < intWithAllocator2.size(); ++i)
    {
         cout << intWithAllocator2[i] << " ";
    }
    
    cout << "End of tests\n";
}

int main_SHOULD_REUSE_AFTER_TESTING()
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


    Quaint::IMemoryContext* context = Bolt::G_BOLT_DEFAULT_MEMORY;
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
    Bolt::AttachmentDefinition swapchainDef;
    swapchainDef.binding = 0;
    swapchainDef.name = "swapchain";
    swapchainDef.clearColor = Quaint::QVec4(0.01f, 0.01f, 0.01f, 1.0f);
    swapchainDef.clearImage = true;
    swapchainDef.type = Bolt::AttachmentDefinition::Type::Swapchain;
    swapchainDef.format = Bolt::EFormat::R8G8B8A8_SRGB;
    swapchainDef.usage = Bolt::EImageUsage::COLOR_ATTACHMENT | Bolt::EImageUsage::COPY_DST; //Hardcoded the same as VulkanSwapchain for now
    info.attachments.pushBack(swapchainDef);

    Bolt::AttachmentDefinition depthDef;
    depthDef.binding = 1;
    depthDef.name = "depthBuffer";
    depthDef.clearColor = Quaint::QVec4(0, 0, 0, 1);
    depthDef.clearImage = true;
    depthDef.type = Bolt::AttachmentDefinition::Type::Depth;
    depthDef.format = Bolt::EFormat::D32_SFLOAT;
    depthDef.usage = Bolt::EImageUsage::DEPTH_ATTACHMENT;
    depthDef.extents = info.extents;
    info.attachments.pushBack(depthDef);

    depthDef.storePrevious = false;

    /* Attachment references in each sub-pass */
    Bolt::RenderStage::AttachmentRef swapchainRef{};
    swapchainRef.binding = 0;
    swapchainRef.attachmentName = "swapchain";

    Bolt::RenderStage::AttachmentRef depthRef{};
    depthRef.binding = 1;
    depthRef.attachmentName = "depthBuffer";

    Quaint::QArray<Bolt::RenderStage> stages(context);
    Bolt::RenderStage shadowStage;
    shadowStage.attachmentRefs = Quaint::QArray<Bolt::RenderStage::AttachmentRef>(context);
    shadowStage.index = 0;
    shadowStage.dependentStage = ~0;
    // TODO: This currently uses a global depth buffer to store shadow from a single light. Modify this to use proper shadow maps coming from lights
    //shadowStage.attachmentRefs.pushBack(swapchainRef);
    shadowStage.attachmentRefs.pushBack(depthRef);
    stages.pushBack(shadowStage);

    
    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addRenderScene("scene_lightmap", info, stages.getSize(), stages.getBuffer());


    stages.clear();

    Bolt::RenderStage geoStage;
    geoStage.attachmentRefs = Quaint::QArray<Bolt::RenderStage::AttachmentRef>(context);
    geoStage.index = 0;

    geoStage.attachmentRefs.pushBack(swapchainRef);
    geoStage.attachmentRefs.pushBack(depthRef);

    geoStage.dependentStage = 0;
    stages.pushBack(geoStage);

    //new stage for IMGUI that doesn't need depth buffer
    Bolt::RenderStage imguiStage;
    imguiStage.attachmentRefs = Quaint::QArray<Bolt::RenderStage::AttachmentRef>(context);
    imguiStage.index = 1;
    imguiStage.dependentStage = 0;

    imguiStage.attachmentRefs.pushBack(swapchainRef);
    stages.pushBack(imguiStage);

    //TODO: Maybe move construction to a separate function
    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addRenderScene("graphics", info, stages.getSize(), stages.getBuffer());
    

    //This is fine for now, but the structure of this should probably change
    Bolt::RenderScene* scene = Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->getRenderScene("graphics");
    Bolt::GlobalLight globalLight("Simple Global");
    globalLight.setColor({1.0f, 0.0f, 0.0f, 1.0f});
    globalLight.setDirection({0.f, -1.0f, 0.1f});

    scene->addGlobalLight(globalLight);

    
    //def.clearColor = Quaint::QVec4(1.0f, 0.0f, 0.0f, 1.0f);
    //def.storePrevious = false;
    ////info.extents = Quaint::QVec2(~0, ~0);
    ////info.offset = Quaint::QVec2({0, 0});
    //info.extents = Quaint::QVec2(256, 560);
    //info.offset = {40, 0};
    //info.attachments.clear();
    //info.attachments.pushBack(def);
    //Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addRenderScene("TESTTEST", info, stages.getSize(), stages.getBuffer());

    //TODO: Add render scene to vulkan renderer through bolt renderer and issue construction

    uint8_t shadowStadeIdx = 0;

    uint32_t geoStageIdx = 0;
    uint8_t imguiStageIdx = 1;

    //Create a pipeline for capturing depth texture for lights

    // Pipeline creation
    Bolt::ShaderDefinition shaderDef{};
    shaderDef.shaders = Quaint::QArray<Bolt::ShaderFileInfo>(context);
    shaderDef.uniforms = Quaint::QArray<Bolt::ShaderUniform>(context);
    shaderDef.pushConstants = Quaint::QArray<Bolt::PushConstant>(context);
    shaderDef.attributeSets = Quaint::QArray<Quaint::QArray<Bolt::ShaderAttributeInfo>>(context);

    Quaint::QArray<Bolt::ShaderAttributeInfo> attributes(context);
    // Shadow casting pipeline
    attributes.pushBack({"position", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"normal", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"texcoord", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"color", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    shaderDef.attributeSets.pushBack(attributes);
    
    shaderDef.shaders.pushBack({"shadwoCast.vert", "C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\shadowCast.vert.spv"
        , "main", Bolt::EShaderStage::VERTEX});
    shaderDef.shaders.pushBack({"shadwoCast.frag", "C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\shadowCast.frag.spv"
        , "main", Bolt::EShaderStage::FRAGMENT});

    shaderDef.uniforms.pushBack({"Buffer_MVP", Bolt::EShaderResourceType::UNIFORM_BUFFER, Bolt::EShaderStage::VERTEX, 1});
    
    Bolt::Pipeline* shadowPipeline = QUAINT_NEW(context, Bolt::Pipeline, context, Quaint::QName("ShadowPipeline"), Quaint::QName("scene_lightmap"), shadowStadeIdx, shaderDef);
    shadowPipeline->cullBack();
    shadowPipeline->enableDepth();
    shadowPipeline->construct();
    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addPipeline(shadowPipeline);


    // Graphics pipeline
    shaderDef.shaders.clear();
    shaderDef.uniforms.clear();
    shaderDef.pushConstants.clear();
    shaderDef.attributeSets.clear();

    shaderDef.shaders.pushBack({"simpleTri.vert", "C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.vert.spv"
        , "main", Bolt::EShaderStage::VERTEX});
    shaderDef.shaders.pushBack({"simpleTri.frag", "C:\\Works\\Project-Quaint\\Data\\Shaders\\TestTriangle\\simpleTri.frag.spv"
        , "main", Bolt::EShaderStage::FRAGMENT});

    shaderDef.uniforms.pushBack({"Buffer_MVP", Bolt::EShaderResourceType::UNIFORM_BUFFER, Bolt::EShaderStage::VERTEX, 1});
    shaderDef.uniforms.pushBack({"Lights_MVP", Bolt::EShaderResourceType::UNIFORM_BUFFER, Bolt::EShaderStage::VERTEX, 1});
    //shaderDef.uniforms.pushBack({"CIS_TestTexture", Bolt::EShaderResourceType::COMBINED_IMAGE_SAMPLER, Bolt::EShaderStage::FRAGMENT, 1});
    
    shaderDef.uniforms.pushBack({"Lights", Bolt::EShaderResourceType::UNIFORM_BUFFER, Bolt::EShaderStage::FRAGMENT, 1});
    shaderDef.uniforms.pushBack({"Material", Bolt::EShaderResourceType::UNIFORM_BUFFER, Bolt::EShaderStage::FRAGMENT, 1});
    shaderDef.uniforms.pushBack({"shadowMap", Bolt::EShaderResourceType::COMBINED_IMAGE_SAMPLER, Bolt::EShaderStage::FRAGMENT, 1});
    
    attributes.clear();

    attributes.pushBack({"position", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"normal", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"texcoord", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});
    attributes.pushBack({"color", 16, Bolt::EFormat::R32G32B32A32_SFLOAT});

    shaderDef.attributeSets.pushBack(attributes);

    /*Creates pipeline and generate a graphic API specific object*/
    Bolt::Pipeline* pipeline = QUAINT_NEW(context, Bolt::Pipeline, context, Quaint::QName("GeoPipeline"), Quaint::QName("graphics"), geoStageIdx, shaderDef);
    pipeline->cullBack();
    pipeline->enableDepth();
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
    //TODO: Probably best to use a new renderpass with no depth support for IMGUI
    Bolt::Pipeline* imguiPipleline = QUAINT_NEW(context, Bolt::Pipeline, context, Quaint::QName("IMGUIPipeline"), Quaint::QName("graphics"), imguiStageIdx, shaderDef);
    imguiPipleline->enableBlend();
    imguiPipleline->addDynamicStage("viewport");
    imguiPipleline->addDynamicStage("scissor");
    imguiPipleline->construct();
    Bolt::RenderModule::get().getBoltRenderer()->GetRenderer()->addPipeline(imguiPipleline);

    //Shadow painter for capturing shadow maps
    Bolt::ShadowPainter* shadowPainter = QUAINT_NEW(context, Bolt::ShadowPainter, context, Quaint::QName("ShadowPipeline"));

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
    

    //std::fstream stream("C:\\Works\\Project-Quaint\\Data\\Models\\cornell_box.obj", ios_base::in | ios_base::binary);
    //std::fstream stream("C:\\Works\\Project-Quaint\\Data\\Models\\cube.obj", ios_base::in | ios_base::binary);
    std::fstream stream("C:\\Works\\Project-Quaint\\Data\\Models\\box.obj", ios_base::in | ios_base::binary);
    //std::fstream stream("C:\\Works\\Project-Quaint\\Data\\Models\\human.obj", ios_base::in | ios_base::binary);

    bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &error, &stream);
    if(!result)

    {
        std::cout << "Failed to load mesh\n";
    }

    //for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    //    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
    //        static_cast<const double>(attrib.vertices[3 * v + 0]),
    //        static_cast<const double>(attrib.vertices[3 * v + 1]),
    //        static_cast<const double>(attrib.vertices[3 * v + 2]));
    //}

    //LoadModelWithSmoothNormals(context, attrib, shapes, materials, geoPainter, modelHolder
    //    , Quaint::QVec4(-200, 0, 0, 1), 1);
        

    LoadModelWithPerFaceNormals(context, attrib, shapes, materials, geoPainter, modelHolder
        , Quaint::QVec4(2, 0, 0, 1), 1, "cube1");

    LoadModelWithPerFaceNormals(context, attrib, shapes, materials, geoPainter, modelHolder
    , Quaint::QVec4(-2, 0, 0, 1), 1, "cube2");

    LoadModelWithPerFaceNormals(context, attrib, shapes, materials, geoPainter, modelHolder
    , Quaint::QVec4(-2, 0, 0, 1), 1, "cube3");
    LoadModelWithPerFaceNormals(context, attrib, shapes, materials, geoPainter, modelHolder
    , Quaint::QVec4(-2, -3, -3, 1), 1, "cube4");
    LoadModelWithPerFaceNormals(context, attrib, shapes, materials, geoPainter, modelHolder
    , Quaint::QVec4(-2, 3, 3, 1), 1, "cube5");
    LoadModelWithPerFaceNormals(context, attrib, shapes, materials, geoPainter, modelHolder
    , Quaint::QVec4(-2, 5, -5, 1), 1, "cube6");

    Bolt::Model* floorModelPtr = QUAINT_NEW(context, Bolt::FloorModel, context, 10.0f, Quaint::QName("Floor"));
    Bolt::ModelRef floorModel(floorModelPtr, Bolt::Deleter<Bolt::FloorModel>(context));
    floorModel->setTranslation({0, -1, 0, 1});
    floorModel->construct();

    shadowPainter->AddModel(floorModel.get());
    geoPainter->AddModel(floorModel.get());

    for(auto& model : modelHolder)
    {
        shadowPainter->AddModel(model.get());
    }


    attrib.vertices.clear();
    attrib.normals.clear();
    attrib.texcoords.clear();
    shapes.clear();
    materials.clear();


    Bolt::RenderModule::get().getBoltRenderer()->addPainter(shadowPainter);
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

        ImGui::Text("DEBUG OPTIONS: 1234567890");
        ImGui::ShowDemoWindow();
        ImGui::Text("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqustuvwxyz: 1234567890");
        
        Bolt::RenderModule::get().getBoltRenderer()->update();
        Bolt::ImguiHandler::Get()->EndFrame();
    }

    //Clear all models
    for(size_t i = 0; i < modelHolder.getSize(); ++i)
    {
        modelHolder[i]->destroy();
        modelHolder[i].release();
    }

    if(floorModel.get())
    {
        floorModel->destroy();
        floorModel.release();
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