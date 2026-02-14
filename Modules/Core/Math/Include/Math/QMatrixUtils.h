#ifndef _H_Q_PROJECTION_UTILS
#define _H_Q_PROJECTION_UTILS

#include "QMat.h"

#define PI 3.14159265f
#define TO_RADIANS(x) x * (PI/180.0f) 

namespace Quaint
{
    inline QMat4x4 buildPerspectiveProjectionMatrix(float nearClip, float farClip, float fov, float aspectRatio)
    {
        //Camera is rendered at -ve z;
        
        //Vulkan's positive Y is downwards
        QMat4x4 res = QMat4x4::Identity();
        float origR = tanf(TO_RADIANS(fov/2.0f)) * nearClip;
        // Camera renders at it's -ve z. So, the camera would've essentially rotated 180 degrees around y.
        // But we still expect right side to be +ve
        // This will change the coordinate system to right-handed
        float r = -origR;
        float l = -r;

        //Top is negative, bottom is positive
        float t = origR * (1.f/aspectRatio);
        // In vulkan y points downwards... So, we pretty much invert this too 
        t = r;
        float b = -t;

        float width = (r - l);
        float height = (t - b);
        float negDepth = (nearClip - farClip);

        res.col0.x = (2 * nearClip) / width;
        res.col2.x = (r + l) / width;   // 0 if canvas is centered. Right now, it should always be 0

        res.col1.y = (2 * nearClip) / height;
        res.col2.y = (t + b) / height; // 0 if canvas is centered. Right now it should always be 0

        //This maps Z in range [0 1]
        res.col2.z = (farClip) / (negDepth);
        res.col3.z = (nearClip * farClip) / (negDepth);
        
        //WARNING: THis is mapping Z in range [-1 1]. This would probably mess with depth mapping
        //res.col2.z = (nearClip + farClip) / (negDepth);
        //res.col3.z = 2 * (nearClip * farClip) / (negDepth);
        
        //res.col3.z = res.col3.z + 1 * 0.5f;

        res.col2.w = -1;
        res.col3.w = 0;
        return res;
    }

    inline QMat4x4 buildOrthographicProjectionMatrix(float nearClip, float farClip, float size, float aspectRatio)
    {
        QMat4x4 res = QMat4x4::Identity();
        float r = size/2;
        float l = -r;
        float height = size / aspectRatio;
        float t = height / 2;
        float b = -t;
        float f = farClip;
        float n = nearClip;

        res.col0.x = 2 / (r - l);
        res.col3.x = (l + r) / (l - r);

        res.col1.y = 2 / (t - b);
        res.col3.y = (t + b) / (b - t);

        // Z is mapped to range [0 1]
        res.col2.z = -1 / (f - n);
        res.col3.z = (n) / (n - f);

        // Z is mapped to range [-1 1]
        //res.col2.z = 2 / (f - n);
        //res.col3.z = (n + f) / (n - f);

        res.col3.w = 1;

        return res;
    }

    inline QMat4x4 buildTranslationMatrix(const QVec4& translation)
    {
        QMat4x4 res = QMat4x4::Identity();
        res.col3 = translation;
        return res;
    }
    inline QMat4x4 buildScaleMatrix(const QVec3& scale)
    {
        QMat4x4 res = QMat4x4::Identity();
        res.col0.x = scale.x;
        res.col1.y = scale.y;
        res.col2.z = scale.z;
        return res;
    }
    inline QMat4x4 buildRotationMatrixYZX(const QVec3& degrees)
    {
        //Yaw = Y, Pitch = X, Roll = Z
        float cosPitch = cos(TO_RADIANS(degrees.x));
        float sinPitch = sin(TO_RADIANS(degrees.x));

        float cosYaw = cos(TO_RADIANS(degrees.y));
        float sinYaw = sin(TO_RADIANS(degrees.y));
        
        float cosRoll = cos(TO_RADIANS(degrees.z));
        float sinRoll = sin(TO_RADIANS(degrees.z));
        

        QMat4x4 rot = QMat4x4::Identity();
        rot.col0 = QVec4(
            (cosRoll * cosYaw) - (sinRoll * sinPitch * sinYaw),
            (sinRoll * cosYaw) + (cosRoll * sinPitch * sinYaw),
            (-cosPitch * sinYaw),
            (0.0f) 
        );
        rot.col1 = QVec4(
            (-sinRoll * cosPitch),
            (cosRoll * cosPitch),
            (sinPitch),
            (0.0f)
        );
        rot.col2 = QVec4(
            (cosRoll * sinYaw) + (sinRoll * sinPitch * cosYaw),
            (sinRoll * sinYaw) - (cosRoll * sinPitch * cosYaw),
            (cosPitch * cosYaw),
            (0.0f)
        );

        return rot;
    }

    inline QMat4x4 makeTransform(const QMat4x4& translation, const QMat4x4& rotation, const QMat4x4& scale)
    {
        //Colum Major! Scale->Rotate->Translate
        return translation * rotation * scale;
    }
    inline QMat4x4 makeTransform(const QVec4& translation, const QVec3& rotation, const QVec3& scale)
    {
        return makeTransform(buildTranslationMatrix(translation),
        buildRotationMatrixYZX(rotation), 
        buildScaleMatrix(scale));
    }
    inline QMat4x4 makeTransform(const QVec4& translation, const QMat3x3& rotationAndScale)
    {
        return (buildTranslationMatrix(translation) * QMat4x4(rotationAndScale));
    }

    
    /*Gives matrix with columns aligned with target looking at*/
    inline QMat3x3 lookAt(const QVec4& target, const QVec4& source, const QVec3& normalizedUp)
    {
        //+Z points away from camera view frustum
        QVec3 forward = (source - target).normalize();
        //QVec3 forward = (target - source).normalize();
        QVec3 right = cross_vf(normalizedUp, forward).normalize();
        QVec3 up = cross_vf(forward, right).normalize();

        QMat3x3 rotMatrix = QMat3x3::Identity();
        rotMatrix.col0 = right;
        rotMatrix.col1 = up;
        rotMatrix.col2 = forward;

        return rotMatrix;
    }

    inline QMat4x4 lookAt_CorrectVersion(const QVec4& target, const QVec4& source, const QVec3& normalizedUp)
    {
        QVec3 forward = (source - target).normalize();
        QVec3 up = normalizedUp;
        QVec3 right = cross_vf(normalizedUp, forward).normalize();

        Quaint::QVec4 invTranslation;
        invTranslation.x = -Quaint::dot_vf(right, source);
        invTranslation.y = -Quaint::dot_vf(up, source);
        invTranslation.z = -Quaint::dot_vf(forward, source);
        invTranslation.w = 1.0f;


        QMat3x3 rotation = QMat3x3::Identity();
        rotation.col0 = right;
        rotation.col1 = up;
        rotation.col2 = forward;
        Quaint::QMat4x4 invRotation = Quaint::transpose_mf(rotation);


        QMat4x4 view = QMat3x3::Identity();
        view.col0.x = right.x;
        view.col1.x = right.x;
        view.col2.x = right.x;

        view.col0.y = up.x;
        view.col1.y = up.x;
        view.col2.y = up.x;

        view.col0.z = forward.x;
        view.col1.z = forward.x;
        view.col2.z = forward.x;
        view.col3 = invTranslation;

        return view;
        //return Quaint::makeTransform(invTranslation, invRotation);

    }

}

#endif //_H_Q_PROJECTION_UTILS