#ifndef _H_SHADER_GROUP
#define _H_SHADER_GROUP
#include "Resources.h"
#include <Types/QStaticString.h>
#include <Types/QMap.h>

namespace Bolt
{
    struct ShaderAttribute
    {
        uint32_t location = 0;
        uint32_t size = 0;
        uint32_t stride = 0;
        EFormat format = EFormat::R32G32B32A32_SFLOAT;
    };

    /* Virtual class not intended to be instantiated directly*/
    class ShaderGroup : public ShaderGroupBase
    {
    public:
        ShaderGroup(Quaint::IMemoryContext* context, const Quaint::QName& name
        , const Quaint::QPath& vertShaderPath, const Quaint::QPath& fragShaderPath
        , const Quaint::QMap<Quaint::QName, ShaderAttribute>&& vertexAttributes)
        : ShaderGroupBase(context)
        , m_name(name)
        , m_vertexAttributes(vertexAttributes)
        {}
        
        const Quaint::QName& getName() const { return m_name; }
        const Quaint::QMap<Quaint::QName, ShaderAttribute>& getVertexAttributes() const { return m_vertexAttributes; }

    private:
        Quaint::QName m_name = "";
        Quaint::QMap<Quaint::QName, ShaderAttribute> m_vertexAttributes;
    };
}

#endif //_H_SHADER_GROUP