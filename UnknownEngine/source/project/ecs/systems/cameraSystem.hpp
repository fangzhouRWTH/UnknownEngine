#pragma once
#include "core/math.hpp"
#include "ecs/system.hpp"
#include "debug/log.hpp"
#include "ecs/ecs.hpp"
#include "ecs/components/transformComponent.hpp"
#include "ecs/components/cameraComponent.hpp"
#include "context/engineContext.hpp"
#include <vector>

namespace unknown::ecs
{
    class SCamera final : public System<SCamera>
    {
    public:
        SCamera()
        {
            Setup();
        }

        virtual void Init() override
        {
            INFO_PRINT("ECS:System:Camera Initialized");
        }

        virtual void Update(EngineContext & context) override
        {
            context.cameraInfo.clear();
            u64 count = _entities.size();
            mCameraInfosCache.resize(count);
            for (u64 i = 0u; i < count; i++)
            {
                auto id = _entities[i];
                auto &cache = mCameraInfosCache[i];
                const auto &t = *static_cast<CTransform *>(_ecs_manager_ptr->GetComponent<CTransform>(id));
                const auto &c = *static_cast<CCamera *>(_ecs_manager_ptr->GetComponent<CCamera>(id));

                cache.far = c.far;
                cache.near = c.near;
                cache.type = c.projection;
                cache.fov_radian = math::ToRadian(c.fov_degree);

                cache.transform = t.transform;
                cache.forward = t.transform * Vec4f(0.f,1.f,0.f,0.f);
                cache.right = t.transform * Vec4f(1.f,0.f,0.f,0.f);
                cache.up = t.transform * Vec4f(0.f,0.f,1.f,0.f);

                context.cameraInfo.push_back(cache);
            }
        }

        void GetCameraInfos(std::vector<CameraInfo> &cameraInfos)
        {
            cameraInfos = mCameraInfosCache;
        }

    private:
        virtual void Setup() override
        {
            Require(CTransform::_class_id);
            Require(CCamera::_class_id);
        }

        std::vector<CameraInfo> mCameraInfosCache;
    };
}