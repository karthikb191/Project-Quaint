#ifndef _H_BOLT_MATERIAL
#define _H_BOLT_MATERIAL

#include <Types/QSharedPtr.h>
#include <Types/QStaticString.h>
#include <GFX/Interface/IEntityInterfaces.h>
#include <EASTL/unordered_set.h>

//TODO: Remove this from here
#include <vulkan/vulkan.h>

namespace Bolt
{
    //TODO: Experiment with a material property block after getting shadows in
    // Main concern is the way to store/retrieve image samplers

    class Material : public IGFXEntity
    {
    public:
        Material(Quaint::IMemoryContext* context);
        virtual void construct() = 0;
        virtual void destroy() = 0;
        virtual void writeImgui() = 0;

        //TODO: Move this to a platform-specific class
        virtual void write(VkDescriptorSet set, uint16_t offset) = 0;
        virtual void update() = 0;
        
        //TODO: Implement later once there's a system to push data dynamically to descriptors
        //template<typename T>
        //const T* const getProperty(Quaint::QName name) const = 0;
        //
        //template<typename T>
        //void setProperty(Quaint::QName name, const T& value) = 0;
        
    private:
        
        //TODO: There should be some sort of a material data block.
        // This should be able to perform platform specific functionalities
        // like updating uniform buffers, images, samplers, etc.
        // Need to think on how to achieve this
    };

    using MaterialRef = Quaint::QSharedPtr<Material>;
};

#endif //_H_BOLT_MATERIAL
