#ifndef _H_Q_PROJECTION_UTILS
#define _H_Q_PROJECTION_UTILS

#include "QMat.h"

#define PI 3.14159265
#define TO_RADIANS(x) x * (PI/180.0f) 

namespace Quaint
{
    inline QMat4x4 buildProjectionMatrix(float nearClip, float farClip, float fov, float aspectRatio)
    {
        QMat4x4 res = QMat4x4::Identity();
        float r = tanf(TO_RADIANS(fov)) * nearClip;
        float l = -r;
        float t = r * (1.f/aspectRatio);
        float b = -t;

        float width = (r - l);
        float height = (t - b);
        float negDepth = (nearClip - farClip);

        res.col0.x = (2 * nearClip) / width; 
        res.col2.x = (r + l) / width;

        res.col1.y = (2 * nearClip) / height;
        res.col2.y = (t + b) / height;

        res.col2.z = (nearClip + farClip) / (negDepth);
        res.col3.z = 2 * (nearClip * farClip) / (negDepth);

        res.col2.w = -1;
        res.col3.w = 0;
    }

    /*Gives matrix with columns aligned with target looking at*/
    inline QMat3x3 lookAt(const QVec4& target, const QVec4& source, const QVec3& normalizedUp)
    {
        //+Z points away from camera
        QVec3 forward = source - target;
        QVec3 right = cross_vf(normalizedUp, forward);
        QVec3 up = cross_vf(forward, right);

        QMat3x3 res = QMat3x3::Identity;
        res.col0 = right;
        res.col1 = up;
        res.col2 = forward;
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
    inline QMat4x4 transform(const QVec4& translation, const QMat4x4& rotation, const QMat4x4& scale)
    {
        return(buildTranslationMatrix(translation), rotation, scale);
    }
    inline QMat4x4 transform(const QMat4x4& translation, const QMat4x4& rotation, const QMat4x4& scale)
    {
        return translation * rotation * scale;
    }
}

#endif //_H_Q_PROJECTION_UTILS