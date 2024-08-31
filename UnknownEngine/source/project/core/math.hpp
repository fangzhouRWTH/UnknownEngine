#pragma once

#include <Eigen/Dense>
#include "platform/type.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846f /* pi */
#endif

#ifndef M_HPI
#define M_HPI 1.570796326f /* half pi */
#endif

namespace unknown
{
    typedef Eigen::Vector2<float> Vec2f;
    typedef Eigen::Vector3<float> Vec3f;
    typedef Eigen::Vector4<float> Vec4f;

    typedef Eigen::Matrix3<float> Mat3f;
    typedef Eigen::Matrix4<float> Mat4f;

    typedef Eigen::Rotation2Df Rot2f;
    typedef Eigen::Quaternionf Qua4f;

    namespace math
    {
        u32 pack_nml_4f_u32(Vec4f vec);
        u32 pack_unml_4f_u32(Vec4f vec);

        Mat4f Translate(const Mat4f &mat, const Vec3f &vec);
        Mat4f Rotate(const Mat4f &mat, const Qua4f &q);
        Mat4f Rotate(const Mat4f &mat, const Vec3f &axis, const float &radian);
        Mat4f Perspective(const float &radian, const float &aspect, const float &near, const float &far);
        Mat4f PerspectiveVK(const float &radian, const float &aspect, const float &near, const float &far);
        Mat4f LookAt(const Vec3f &eye, const Vec3f &center, const Vec3f &up);

        template <typename T>
        T ToRadian(T degree)
        {
            return degree / 180.f * M_PI;
        }

        template <typename T>
        T ToDegree(T radian)
        {
            return radian * 180.f / M_PI;
        }

        template <typename T>
        T max(T left, T right)
        {
            return left > right ? left : right;
        }

        template <typename T>
        T min(T left, T right)
        {
            return left < right ? left : right;
        }

        // template <typename T>
        // T clamp(T v, T l, T r)
        // {
        //     v = v > l ? v : l;
        //     v = v < r ? v : r;
        //     return v;
        // }

        template <typename T>
        T clamp(T value, T min, T max)
        {
            return value < min ? min : value < max ? value
                                                   : max;
        }
    }
}