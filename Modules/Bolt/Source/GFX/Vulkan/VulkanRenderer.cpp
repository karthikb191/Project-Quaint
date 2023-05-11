#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QArray.h>

namespace Bolt
{

    VulkanRenderer::VulkanRenderer()
    {
        int test = 111;
        test -= 100;
        int a = test;

        a += 10;

        m_running = false;
    }
    VulkanRenderer::VulkanRenderer(int a)
    {
        int test = 111;
        test -= 100;
        a = test;

        a += 10;

        m_running = false;
    }
    VulkanRenderer::~VulkanRenderer()
    {
        int test = 111;
        test -= 100;
        int a = test;

        a += 10;

        m_running = false;
    }
    void VulkanRenderer::init(Quaint::IMemoryContext* context)
    {
        m_context = context;
        if(m_context != nullptr)
        {
            createAllocationCallbacks();
        }
        //Vulkan Instance creation

        m_running = true;
    }

    void VulkanRenderer::shutdown()
    {

    }
    
    void VulkanRenderer::createAllocationCallbacks()
    {

    }
    void VulkanRenderer::createInstance()
    {

    }
}