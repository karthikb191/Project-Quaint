#ifndef _H_VULKAN_RENDERER
#define _H_VULKAN_RENDERER

#include <vulkan/vulkan.h>
#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>
#include <QuaintLogger.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <Types/QArray.h>
#include <Types/QFastArray.h>
#include <Types/QUniquePtr.h>

#include <QMath.h>
#include <Core/Camera.h>

//stb
#include <stb/stb_image.h>

#include "Internal/DeviceManager.h"
#include "Internal/PipelineManager.h"
#include "Internal/ShaderManager.h"
#include "Internal/VulkanRenderPass.h"
#include "Internal/VulkanRenderScene.h"
#include "Internal/VulkanGraphicsContext.h"

namespace Bolt
{
    using namespace vulkan;
    class BoltRenderer;
    class VulkanRenderer;
    class Bolt::RenderScene;

    namespace vulkan
    {
        class VulkanSwapchain;
    }
    
    #define MAX_FRAMES_IN_FLIGHT 2

    #define ASSERT_SUCCESS1(res, msg) assert(res == VK_SUCCESS && msg)
    #define ASSERT_SUCCESS(res, msg) ASSERT_SUCCESS1((res), msg)

    //TODO: Move these
    struct QVector2
    {
        float x = 0.f;
        float y = 0.f;
    };
    struct QVector3
    {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f; 
    };

    struct QVertex
    {
        QVector2    position;
        QVector3    color;
        QVector2    texCoord;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription desc{};
            desc.binding = 0; //only 1 binding for now. This parameter specifies the index of binding in array of bindings
            desc.stride = sizeof(QVertex);
            desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //Must be read per vertex
            return desc;
        }
        static void getAttributeDescription(Quaint::QFastArray<VkVertexInputAttributeDescription, 3>& res)
        {
            VkVertexInputAttributeDescription desc{};
            //From which VertexInputBindingDescription we are trying to access the attribute from
            //Accessing position attribute
            desc.binding = 0;
            desc.location = 0;
            desc.offset = offsetof(QVertex, position);
            desc.format = VK_FORMAT_R32G32_SFLOAT;
            res[0] = desc;

            //Accessing the color attribute
            desc.binding = 0;
            desc.location = 1;
            desc.offset = offsetof(QVertex, color);
            desc.format = VK_FORMAT_R32G32B32_SFLOAT;
            res[1] = desc;

            //Accessing texcoord attribute
            desc.binding = 0;
            desc.location = 2;
            desc.offset = offsetof(QVertex, texCoord);
            desc.format = VK_FORMAT_R32G32_SFLOAT;
            res[2] = desc;
        }
    };

    struct UniformBufferObject
    {
        Quaint::QMat4x4     model;
        Quaint::QMat4x4     view;
        Quaint::QMat4x4     proj;
    };
    
    struct FrameInfo
    {
        UniformBufferObject         ubo;
        VkFramebuffer               frameBuffer;
        VkFence                     renderFence = VK_NULL_HANDLE;
    };

    // TODO: This might not be a right name
    class GraphicsContextInternal
    {
    public:
        GraphicsContextInternal();
        void destroy();

        void setup(VulkanRenderer* renderer);
        void initialize(uint32_t width, uint32_t height, VulkanTexture* texture);
        void initializeCommandRecordCapability(const EQueueType queueType);

        void begin();
        void end(VkImage swapChainImage);

        VkSemaphore& getSyncSemaphore() { return m_syncSemaphore; }
        VulkanTexture* getRenderTexture() { return m_texture; }
    private:
        void construct();

        VulkanRenderer*         m_renderer = nullptr;
        VulkanTexture*          m_texture = nullptr;     
        VkFramebuffer           m_frameBuffer = VK_NULL_HANDLE;
        VkRenderPass            m_renderPass = VK_NULL_HANDLE;
        VkCommandPool           m_commandPool = VK_NULL_HANDLE; //TODO: Pool should ideally come from a different place
        VkCommandBuffer         m_commandBuffer = VK_NULL_HANDLE;
        VkQueue                 m_queue = VK_NULL_HANDLE;
        uint32_t                m_queueIndex = -1;

        VkSemaphore             m_syncSemaphore = VK_NULL_HANDLE;
        VkFence                 m_internalFence = VK_NULL_HANDLE;
    };

    class alignas(16) VulkanRenderer : public IRenderer, public IRenderObjectBuilder
    {
        friend class BoltRenderer;
        
        template<typename T, typename ...ARGS>
        friend T* ::allocFromContext(Quaint::IMemoryContext* context, ARGS...);

    public:
        struct QueueFamily
        {
        public:
            void set(uint32_t idx) {index = idx; valid = true;}
            void unset() {index = 0; valid = false;}
            uint32_t get() { return index; }
            bool isSet(){ return valid; }

        private:
            uint32_t        index  = 0;
            bool            valid       = false;
        };
        struct QueueFamilies
        {
            QueueFamily     graphics{};
            QueueFamily     presentation{};
            QueueFamily     transfer{};

            bool allSet() { return graphics.isSet() && presentation.isSet() &&transfer.isSet(); }
        };

        struct SwapchainSupportInfo
        {
            SwapchainSupportInfo(Quaint::IMemoryContext* context)
            : surfaceCapabilities{0}
            , surfaceFormat(context)
            , presentMode(context)
            {}
            VkSurfaceCapabilitiesKHR            surfaceCapabilities;
            Quaint::QArray<VkSurfaceFormatKHR>  surfaceFormat;
            Quaint::QArray<VkPresentModeKHR>    presentMode;
        };

        struct FixedStageInfo
        {
            VkPipelineDynamicStateCreateInfo        dynamicStateInfo{};
            VkPipelineVertexInputStateCreateInfo    vertexInputStateInfo{};
            VkPipelineInputAssemblyStateCreateInfo  pipelineInputAssemblyStateInfo{};
            VkPipelineViewportStateCreateInfo       viewportStateInfo{};
            VkPipelineRasterizationStateCreateInfo  rasterizationStateInfo{};
            VkPipelineMultisampleStateCreateInfo    multisampleSateInfo{};
            VkPipelineColorBlendAttachmentState     blendAttachmentState{};
            VkPipelineColorBlendStateCreateInfo     colorBlendInfo{};
        };

    public:
        void init() override;
        void shutdown() override;
        void render() override;

        DeviceManager* getDeviceManager() { return m_deviceManager; }
        VkAllocationCallbacks* getAllocationCallbacks() { return m_allocationPtr; }
        VkSurfaceKHR getSurface() { return m_surface; }
        VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
        VkDevice getDevice() { return m_device; }
        Quaint::IMemoryContext* getMemoryContext() { return m_context; }
        
        static VulkanRenderer* get() { return s_Instance; }

        IShaderGroupConstructor* getShaderGroupConstructor() { return m_shaderManager; }

        IRenderObjectImpl*  buildRenderObjectImplFor(RenderObject* obj) override;
        vulkan::RenderFrameScene* getRenderFrameScene() { return &m_renderScene; }

        VulkanSwapchain* getSwapchain() { return m_vulkanSwapchain.get(); }

        //TODO: Fix this
        const UniformBufferObject& getMVPMatrix() { return m_ubo; }
    
        void createBuffer(size_t bufferSize, VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags propertyFlags,
        VkDeviceMemory& deviceMemory,
        VkBuffer& buffer);

        void createBuffer(size_t bufferSize, void* data, VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags propertyFlags,
        VkDeviceMemory& deviceMemory,
        VkBuffer& buffer);

        void createTextureFromFile(const char* path, VulkanTexture& outTexuture, const VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT);

        //TODO:
        void crateShaderInputTextureFromFile(const char* path, VulkanTexture& outTexuture, const VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT);

        void mapBufferToMemory();

        void addRenderScene(RenderScene* scene);
        void constructPendingRenderScenes();
    private:
        void createBuffer2(size_t bufferSize, VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags propertyFlags,
        VkDeviceMemory& deviceMemory,
        VkBuffer& buffer);

    //------ Static Allocation Functions
        static void* VKAPI_PTR allocationFunction(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR freeFunction(void* pUserData, void* pMemory);
        static void* VKAPI_PTR reallocFunction(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR internalAllocationFunction
        (void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR internalFreeFunction
        (void* pUserData, void* pMemory, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
    //----------------------------------

    //------ Static Message Handlers

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunction(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

    //------------------------------
        VkCommandPool buildCommandPool(const VkCommandPoolCreateFlags flags, const EQueueTypeFlags supportedQueues, const bool requiresPresentationSupport = false);

        void updateUniformBuffer(uint32_t index);
        void updateUniformBufferProxy();
        void drawFrame();

        void createAllocationCallbacks();
        void createInstance();
        void createSurface();
        void selectPhysicalDevice();
        void createLogicalDevice();

        void createScene();

        void createSwapchain();
        void createImageViews();
        void setupFixedFunctions(FixedStageInfo& fixedStageInfo);
        void createRenderPass();
        void createDescriptorSetLayout();
        void createRenderPipeline();

        void createFrameBuffers();
        
        void createCommandPool();
        void createGraphicsCommandPool();
        void createTransferCommandPool();

        void createSampleImage();
        void createSampleImageView();
        void createSampleImageSampler();

        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffers();

        void createDescriptorPool();
        void createDescriptorSets();

        void createCommandBuffer();
        void createSyncObjects();

        void cleanupOutofDateSwapchain();
        void recreateSwapchain();


        void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
        
        //TODO: Surround this with platform spec macro
        void createWindowsSurface(); //Surface creation might affect physical device selection
    #ifdef DEBUG_BUILD
        void setupDebugMessenger();
        void destroyDebugMessenger();        
    #endif

        //void updateTriangleColor();

        VulkanRenderer(Quaint::IMemoryContext* context);
        virtual ~VulkanRenderer();
        
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer(VulkanRenderer&&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(VulkanRenderer&&) = delete;

        bool                                m_running = false;
        Quaint::IMemoryContext*             m_context;
        Camera                              m_camera;

        VkAllocationCallbacks               m_defGraphicsAllocator;
        VkAllocationCallbacks*              m_allocationPtr;
        VkInstance                          m_instance = VK_NULL_HANDLE;
        VkPhysicalDevice                    m_physicalDevice = VK_NULL_HANDLE;
        VkSurfaceKHR                        m_surface = VK_NULL_HANDLE;
        VkDevice                            m_device = VK_NULL_HANDLE;

        VkSwapchainKHR                      m_swapchain = VK_NULL_HANDLE;
        VkSwapchainKHR                      m_outOfDateSwapchain = VK_NULL_HANDLE;
        VkFormat                            m_swapchainFormat;
        VkExtent2D                          m_swapchainExtent;
        Quaint::QArray<VkImage>             m_swapchainImages;
        Quaint::QArray<VkImageView>         m_swapchainImageViews;

        VkRenderPass                        m_renderPass = VK_NULL_HANDLE;
        VkPipelineLayout                    m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline                          m_graphicsPipeline = VK_NULL_HANDLE;

        VkDescriptorSetLayout               m_descriptorSetLayout = VK_NULL_HANDLE;

        VkQueue                             m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue                             m_presentQueue = VK_NULL_HANDLE;
        VkQueue                             m_transferQueue = VK_NULL_HANDLE;

        Quaint::QArray<VkFramebuffer>       m_frameBuffers;
        VkCommandPool                       m_graphicsCommandPool = VK_NULL_HANDLE;
        VkCommandPool                       m_transferCommandPool = VK_NULL_HANDLE;
        
        VkBuffer                            m_vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory                      m_vertexBufferGpuMemory = VK_NULL_HANDLE; //Memory in GPU

        VkBuffer                            m_indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory                      m_indexBufferGpuMemory = VK_NULL_HANDLE;

        Quaint::QArray<VkBuffer>            m_uniformBuffers;
        Quaint::QArray<VkDeviceMemory>      m_uniformBufferGpuMemory;
        Quaint::QArray<void*>               m_mappedUniformBuffers;

        VkDescriptorPool                    m_descriptorPool;
        Quaint::QArray<VkDescriptorSet>     m_descriptorSets;

        Quaint::QArray<VkCommandBuffer>     m_commandBuffers;

        Quaint::QArray<VkSemaphore>         m_imageAvailableSemaphores;
        Quaint::QArray<VkSemaphore>         m_renderFinishedSemaphores;
        Quaint::QArray<VkFence>             m_inFlightFences;

        uint8_t                             m_currentFrame = 0;

        UniformBufferObject                 m_ubo;

        VkImage                             m_texture;
        VkImageView                         m_textureView;
        VkSampler                           m_textureSampler;
        VkDeviceMemory                      m_textureGpuMemory;

        DeviceManager*                      m_deviceManager = nullptr;
        ShaderManager*                      m_shaderManager = nullptr;

        vulkan::VulkanRenderPass            m_customRenderPass;
        vulkan::RenderFrameScene            m_renderScene;                     
        static VulkanRenderer*              s_Instance;

        vulkan::GraphicsContext             m_immediateContext;
        Quaint::QUniquePtr<VulkanSwapchain> m_vulkanSwapchain = nullptr;
        Quaint::QArray<RenderScene*>        m_renderScenes;

    #ifdef DEBUG_BUILD      
        VkDebugUtilsMessengerEXT            m_debugMessenger = VK_NULL_HANDLE;
    #endif
    };

    VulkanRenderer::SwapchainSupportInfo querySwapchainSupport(Quaint::IMemoryContext* context, const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    VkSurfaceFormatKHR chooseSurfaceFormat(VulkanRenderer::SwapchainSupportInfo& supportInfo);
    VkExtent2D chooseSwapExtent(Quaint::IMemoryContext* context, VulkanRenderer::SwapchainSupportInfo& supportInfo);
    VkPresentModeKHR choosePresentationMode(Quaint::IMemoryContext* context, VulkanRenderer::SwapchainSupportInfo& supportInfo);
}

#endif