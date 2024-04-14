#ifndef _H_DEVICE_MANAGER
#define _H_DEVICE_MANAGER

#include <Interface/IMemoryContext.h>
#include <Singleton.h>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <Types/QArray.h>
#include <Types/QStaticString.h>

namespace Bolt
{
    enum EQueueType : uint32_t
    {
        Graphics = VK_QUEUE_GRAPHICS_BIT,
        Compute = VK_QUEUE_COMPUTE_BIT,
        Transfer = VK_QUEUE_TRANSFER_BIT,
        Invalid = 0xffffffff
    };
    struct QueueRequirements
    {
        EQueueType          queueType;
        uint8_t             numRequired;
    };
    struct PhysicalDeviceRequirements
    {
        QueueRequirements                   graphics;
        QueueRequirements                   compute;
        QueueRequirements                   transfer;

        //TODO: Remove this ugly piece of code below after adding an allocator
        Quaint::QArray<Quaint::QString256>  extensions = Quaint::QArray<Quaint::QString256>::GetInvalidPlaceholder();

        bool            requireSwapchainSupport = false;
        //Device should have a queue family that supports all operations above
        bool            strictlyIdealQueueFamilyRequired = false;
    };

    struct LogicalDeviceRequirements
    {
        
    };

    struct QueueDefinition
    {
        bool isvalid() const;
        bool supportsGraphicsOps() const;
        bool supportsComputeOps() const;
        bool supportsTransferOps() const;
        uint32_t getQueueFamily() const;
        
        uint32_t getTypeFlags() const { return m_queueFlags; }
    private:
        friend class DeviceManager;

        uint32_t                    m_queueFamily = -1;
        uint32_t                    m_queueFlags = EQueueType::Invalid;
    };

    struct DeviceDefinition
    {
    public:
        DeviceDefinition(Quaint::IMemoryContext* context);
        DeviceDefinition() = default;
        DeviceDefinition(const DeviceDefinition&) = default;
        DeviceDefinition& operator=(const DeviceDefinition&) = default;

        /*Simple Getters and Setters*/
        const QueueDefinition& getQueueOfType(const EQueueType flags);

        VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
        VkDevice getDevice() { return m_device; }

    private:
        friend class DeviceManager;

        VkPhysicalDevice                    m_physicalDevice = VK_NULL_HANDLE;
        VkDevice                            m_device = VK_NULL_HANDLE;

        //TODO: Remove this ugly piece of code below after adding an allocator
        Quaint::QArray<QueueDefinition>     m_queueDefinitions = Quaint::QArray<QueueDefinition>::GetInvalidPlaceholder();
    };

    class DeviceManager : public Singleton<DeviceManager>
    {
        DECLARE_SINGLETON(Bolt::DeviceManager);
    public:

        struct ExternalDependencies
        {
            VkInstance      instance = VK_NULL_HANDLE;
            VkSurfaceKHR    surface = VK_NULL_HANDLE;   
        };

        void injectReferences(ExternalDependencies dependencies) { m_dependencies = dependencies; }

        VkResult createDevicesAndQueues(const PhysicalDeviceRequirements& phyDevReq, const LogicalDeviceRequirements& logDevReq);


        /*Getters and Setters*/

        const DeviceDefinition& getDeviceDefinition() { return m_deviceDefinition; }

    private:
        virtual ~DeviceManager() = default;
        DeviceManager();

        VkPhysicalDevice getBestPhysicalDeviceFor(const PhysicalDeviceRequirements& phyDevReq);
        uint32_t getDeviceSuitabilityScore(VkPhysicalDevice device, const PhysicalDeviceRequirements& phyDevReq);
        void populateQueueDefinition(const PhysicalDeviceRequirements& phyDevReq);
        VkDevice createLogicalDevice(const LogicalDeviceRequirements& logDevReq);

        //Currently only supporting a single device definition, but this can be extended
        DeviceDefinition            m_deviceDefinition;
        ExternalDependencies        m_dependencies;
    };
}

#endif //_H_DEVICE_MANAGER