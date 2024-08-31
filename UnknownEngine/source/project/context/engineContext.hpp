#pragma once
#include <memory>
#include <vector>
#include "core/math.hpp"

namespace unknown
{
    namespace ecs
    {
        class ECSManager;
    }

    enum class ProjectionType
    {
        Isometric,
        Perspective,
    };

    struct CameraInfo
    {
        ProjectionType type;
        float far;
        float near;
        float fov_radian;

        Mat4f transform;
        Vec4f forward;
        Vec4f right;
        Vec4f up;
    };

    struct EngineContext
    {
        float deltaTime;
        std::vector<CameraInfo> cameraInfo;
        std::shared_ptr<ecs::ECSManager> ecsManager;
    };
}