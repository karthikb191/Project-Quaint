#ifndef _H_BOLT_MATERIAL
#define _H_BOLT_MATERIAL

#include <Types/QSharedPtr.h>
#include <GFX/Interface/IEntityInterfaces.h>
#include <EASTL/unordered_set.h>

namespace Bolt
{
    class Material : public IGFXEntity
    {
    public:
        Material(Quaint::IMemoryContext* context);
        virtual void construct() = 0;
        virtual void destroy() = 0;
        
        
        
    private:
        
    };

    using MaterialRef = Quaint::QSharedPtr<Material>;
};

#endif //_H_BOLT_MATERIAL
