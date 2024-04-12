#ifndef _H_DEVICE_MANAGER
#define _H_DEVICE_MANAGER

#include <Interface/IMemoryContext.h>
#include <Singleton.h>
#include <cstdint>
//#include <vulkan/vulkan.h>

namespace Bolt
{
    enum class EQueueType : int
    {
        Graphics,
        Compute,
        Transfer
    };
    struct QueueRequirements
    {
        EQueueType          queueType;
        uint8_t             numRequired;
    };
    struct PhysicalDeviceRequirements
    {
        QueueRequirements           graphics;
        QueueRequirements           compute;
        QueueRequirements           transfer;

        //Device should have a queue family that supports all operations above
        bool            strictlyIdealQueueFamilyRequired = false;
    };

    struct LogicalDeviceRequirements
    {
        
    };

    class DeviceManager : public Singleton<DeviceManager>
    {
    public:
        DECLARE_SINGLETON(Bolt::DeviceManager);

        //VkResult Init();

    private:
        DeviceManager() = default;
    };
}

#endif //_H_DEVICE_MANAGER