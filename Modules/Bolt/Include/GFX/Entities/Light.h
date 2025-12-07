#ifndef _H_BOLT_LIGHT
#define _H_BOLT_LIGHT

#include <GFX/Interface/IEntityInterfaces.h>
#include <GFX/Data/LightData.h>
#include <Types/QStaticString.h>

namespace Bolt
{
    enum ELightType
    {
        Global,
        Point
    };

    //TODO: Doesn't really need to be IGFXEntity as it's just an abstraction of struct that gets filled into a buffer
    class LightBase
    {
    public:
        LightBase(ELightType type);
        
        void setName(const Quaint::QName& name) { m_name = name; }

        const Quaint::QName& getName() const{ return m_name; }

        virtual const void* getData() const = 0;
        virtual uint16_t getDataSize() const = 0;
        virtual void writeImgui() = 0;

    protected:
        Quaint::QName m_name;
        ELightType m_lightType;
    };

    class GlobalLight : public LightBase
    {
    public:
        GlobalLight(Quaint::QName name = "");
        void setColor(const QVec4& color);
        void setDirection(const QVec3& direction);

        
        virtual const void* getData() const { return (const void*)(&m_data);};
        virtual uint16_t getDataSize() const { return sizeof(GlobalLightData); };
        const Quaint::QVec4 getColor() const { return Quaint::QVec4(m_data.color[0], m_data.color[1], m_data.color[2], m_data.color[3]); }
        const Quaint::QVec3 getDirection() const { return Quaint::QVec3(m_data.direction[0], m_data.direction[1], m_data.direction[2]); }
        
        virtual void writeImgui();

    private:
        GlobalLightData m_data;
    };

    class PointLight : public LightBase
    {
    public:
        PointLight(Quaint::QName name = "");

        virtual const void* getData() const { return &m_data;};
        virtual uint16_t getDataSize() const { return sizeof(PointLightData); };
        virtual void writeImgui();

    private:
        PointLightData m_data;
    };
}

#endif //_H_BOLT_LIGHT