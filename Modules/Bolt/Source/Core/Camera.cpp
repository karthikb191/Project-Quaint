#include <Core/Camera.h>

namespace Bolt
{
    void Camera::init(const CameraInitInfo& info)
    {
        m_transform = Quaint::makeTransform(info.translation, info.rotation, Quaint::QVec3(1, 1, 1));
        m_params.fov = info.fov;
        m_params.farClipDist = info.farClipDist;
        m_params.nearClipDist = info.nearClipDist;
    }
    void Camera::lookAt(const Quaint::QVec4& target, const Quaint::QVec3& up)
    {
        //TODO: This should be called from a "Transform" class 
        Quaint::QMat3x3 lookAtMatrix = Quaint::lookAt(target, m_transform.col3, up);
        m_transform.col0 = lookAtMatrix.col0;
        m_transform.col1 = lookAtMatrix.col1;
        m_transform.col2 = lookAtMatrix.col2;
    }
    Quaint::QMat4x4 Camera::getViewMatrix()
    {
        //TODO: Orthogonal check transformation matrix

        //Negate translation and tranpose rotation matrix
        Quaint::QMat3x3 invRotation = Quaint::transpose_mf(m_transform);
        Quaint::QVec4 invTranslation = m_transform.col3 * -1.0f;

        return Quaint::QMat4x4(invRotation, invTranslation);
    }
}