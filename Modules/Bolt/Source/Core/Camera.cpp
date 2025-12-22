#include <Core/Camera.h>

namespace Bolt
{
    void Camera::init(const CameraInitInfo& info)
    {
        m_transform = Quaint::makeTransform(info.translation, info.rotation, Quaint::QVec3(1, 1, 1));
        Quaint::QMat3x3 invRotation = Quaint::transpose_mf(m_transform);
        m_params.fov = info.fov;
        m_params.farClipDist = info.farClipDist;
        m_params.nearClipDist = info.nearClipDist;
        computeProjection();
    }
    void Camera::lookAt(const Quaint::QVec4& target,const Quaint::QVec4& camLocation, const Quaint::QVec3& up)
    {
        //TODO: This should be called from a "Transform" class
        Quaint::QMat4x4 lookAtMatrix = Quaint::lookAt(target, camLocation, up);

        //m_transform = lookAtMatrix;
        m_transform.col0 = lookAtMatrix.col0;
        m_transform.col1 = lookAtMatrix.col1;
        m_transform.col2 = lookAtMatrix.col2;
        m_transform.col3 = camLocation;
    }
    void Camera::setLocation(const Quaint::QVec4& location)
    {
        //TODO: This is wrong!
        m_transform.col3 = location;
    }
    Quaint::QMat4x4 Camera::getViewMatrix() const
    {
        //TODO: Orthogonal check transformation matrix

        //Negate translation and tranpose rotation matrix
        Quaint::QMat3x3 invRotation = Quaint::transpose_mf(m_transform);
        //Quaint::QVec4 invTranslation = m_transform.col3 * -1.0f;
        Quaint::QVec4 invTranslation;
        invTranslation.x = -Quaint::dot_vf(m_transform.col0, m_transform.col3);
        invTranslation.y = -Quaint::dot_vf(m_transform.col1, m_transform.col3);
        invTranslation.z = -Quaint::dot_vf(m_transform.col2, m_transform.col3);
        invTranslation.w = 1.0f;

        return Quaint::makeTransform(invTranslation, invRotation);
        //return Quaint::QMat4x4(invRotation, invTranslation);
    }

    void Camera::setAspectRatio(float aspectRatio)
    {
        m_params.aspectRatio = aspectRatio;
        computeProjection();
    }
    void Camera::computeProjection()
    {
        m_projection = Quaint::buildPerspectiveProjectionMatrix(m_params.nearClipDist, m_params.farClipDist, m_params.fov, m_params.aspectRatio);
    }
}