#ifndef _H_CAMERA
#define _H_CAMERA

#include <QMath.h>

namespace Bolt
{
    struct CameraInitInfo
    {
        float               fov = 90.0f;
        float               nearClipDist = 0.1f;
        float               farClipDist = 1000.0f;
        Quaint::QVec4       translation;
        Quaint::QVec3       rotation;
    };
    class Camera
    {
    private:
        struct CameraParams
        {
            float fov               = 90.0f;
            float nearClipDist      = 0.1f;
            float farClipDist       = 1000.0f;
        };
    public:
        void init(const CameraInitInfo& info);
        void lookAt(const Quaint::QVec4& target, const Quaint::QVec3& up);
        Quaint::QMat4x4 getViewMatrix();

    private:
        Quaint::QMat4x4         m_transform;
        CameraParams            m_params;
    };
}

#endif