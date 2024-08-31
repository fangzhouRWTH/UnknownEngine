#include "math.hpp"

#include <debug/log.hpp>
#include <Eigen/Geometry>

#include <cassert>

namespace unknown::math
{
    u32 pack_unml_4f_u32(Vec4f vec)
    {
        // assert(vec.x() <= 255.f);
        // assert(vec.x() >= 0.f);
        // assert(vec.y() <= 255.f);
        // assert(vec.y() >= 0.f);
        // assert(vec.z() <= 255.f);
        // assert(vec.z() >= 0.f);
        // assert(vec.w() <= 255.f);
        // assert(vec.w() >= 0.f);

        u32 _X = u32(clamp(vec.x(), 0.f, 255.f)) & 255u;
        u32 _Y = u32(clamp(vec.y(), 0.f, 255.f)) & 255u;
        u32 _Z = u32(clamp(vec.z(), 0.f, 255.f)) & 255u;
        u32 _W = u32(clamp(vec.w(), 0.f, 255.f)) & 255u;

        u32 _R = (_X << 24u) &
                 (_Y << 16u) &
                 (_Z << 8u) &
                 _W;
        return _R;
    }

    u32 pack_nml_4f_u32(Vec4f vec)
    {
        u32 _X = u32(clamp(vec.x(), 0.f, 1.f) * 255u) & 255u;
        u32 _Y = u32(clamp(vec.y(), 0.f, 1.f) * 255u) & 255u;
        u32 _Z = u32(clamp(vec.z(), 0.f, 1.f) * 255u) & 255u;
        u32 _W = u32(clamp(vec.w(), 0.f, 1.f) * 255u) & 255u;

        u32 _R = (_X << 24u) &
                 (_Y << 16u) &
                 (_Z << 8u) &
                 _W;
        return _R;
    }

    Mat4f Translate(const Mat4f &mat, const Vec3f &vec)
    {
        Eigen::Affine3f _translation;
        _translation = Eigen::Translation<float, 3>(vec);
        return _translation.matrix() * mat;
    }

    Mat4f Rotate(const Mat4f &mat, const Qua4f &q)
    {
        Mat4f rm = Mat4f::Identity();
        rm.block(0, 0, 3, 3) = q.toRotationMatrix();
        return rm * mat;
    }

    Mat4f Rotate(const Mat4f &mat, const Vec3f &axis, const float &radian)
    {
        // Qua4f q;
        // q = Eigen::AngleAxis<float>(radian,axis);
        // return Rotate(mat, q);
        Eigen::Affine3f t;
        t = Eigen::AngleAxis(radian, axis);
        return t.matrix() * mat;
    }

    Mat4f LookAt(const Vec3f &eye, const Vec3f &center, const Vec3f &up)
    {
        Vec3f f = (center - eye);
        f.normalize();
        Vec3f u = up;
        u.normalize();
        Vec3f s = f.cross(u);
        u = s.cross(f);

        Mat4f Result = Mat4f::Identity();
        Result.col(0)(0) = s.x();
        Result.col(1)(0) = s.y();
        Result.col(2)(0) = s.z();
        Result.col(0)(1) = u.x();
        Result.col(1)(1) = u.y();
        Result.col(2)(1) = u.z();
        Result.col(0)(2) = -f.x();
        Result.col(1)(2) = -f.y();
        Result.col(2)(2) = -f.z();
        Result.col(3)(0) = -s.dot(eye);
        Result.col(3)(1) = -u.dot(eye);
        Result.col(3)(2) = f.dot(eye);

        return Result;
    }

    Mat4f Perspective(const float &radian, const float &aspect, const float &near, const float &far)
    {
        //assert(aspect != 0.f);
        assert(far != near);

        float tanHalfFovy = tan(radian / 2.f);

        Mat4f Result = Mat4f::Zero();
        Result.col(0)(0) = 1.f / (aspect * tanHalfFovy);
        Result.col(1)(1) = 1.f / (tanHalfFovy);
        Result.col(2)(2) = -(far + near) / (far - near);
        Result.col(2)(3) = -1.f;
        Result.col(3)(2) = -(2.f * far * near) / (far - near);
        return Result;
    }

    Mat4f PerspectiveVK(const float &radian, const float &aspect, const float &near, const float &far)
    {
        //assert(aspect != 0.f);
        assert(far != near);

        float tanHalfFovy = tan(radian / 2.f);

        Mat4f X = Mat4f::Identity();
        X.col(1)(1) = -1.f;
        X.col(2)(2) = -1.f;

        Mat4f P = Mat4f::Zero();
        P.col(0)(0) = 1.f / (aspect * tanHalfFovy);
        P.col(1)(1) = -1.f / (tanHalfFovy);
        P.col(2)(2) = -(far + near) / (far - near);
        P.col(2)(3) = -1.f;
        P.col(3)(2) = -(2.f * far * near) / (far - near);
        return P;
    }
}