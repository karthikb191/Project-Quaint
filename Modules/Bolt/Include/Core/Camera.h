#ifndef _H_CAMERA
#define _H_CAMERA

#include <QMath.h>

namespace Bolt
{
    struct CameraInitInfo
    {
        float               fov = 90.0f;
        float               aspectRatio = 1.33f;
        float               nearClipDist = 0.1f;
        float               farClipDist = 1000.0f;
        Quaint::QVec4       translation = Quaint::QVec4(1, 1, 1, 1);
        Quaint::QVec3       rotation = Quaint::QVec3(0, 0, 0);
    };
    class alignas(16) Camera
    {
    private:
        struct CameraParams
        {
            float fov               = 90.0f;
            float aspectRatio       = 1.33f;
            float nearClipDist      = 0.1f;
            float farClipDist       = 1000.0f;
        };
    public:
        void init(const CameraInitInfo& info);
        void lookAt(const Quaint::QVec4& target, const Quaint::QVec4& camLocation, const Quaint::QVec3& up);
        void setLocation(const Quaint::QVec4& location);
        void setAspectRatio(float aspectRatio);
        Quaint::QMat4x4 getViewMatrix() const;
        const Quaint::QMat4x4& getProjectionMatrix() const { return m_projection; }

    private:
        void computeProjection();

        CameraParams            m_params;
        Quaint::QMat4x4         m_transform;
        Quaint::QMat4x4         m_projection;
    };
}

#endif