#include "renderSystem.hpp"
#include "meshComponent.hpp"
#include "ecs/ecs.hpp"
#include "ecs/components/transformComponent.hpp"
#include "renderer/renderer.hpp"
#include "framework/framework.hpp"
#include "context/engineContext.hpp"

#include "debug/log.hpp"

namespace unknown::ecs
{

    float tempTime = 0.f;
    SRender::SRender()
    {
        Setup();
    }

    void SRender::Init()
    {
        INFO_PRINT("ECS:System:Render Initialized");
    }

    void SRender::Update(EngineContext & context)
    {
        // INFO_LOG("ECS:System:Movement Initialized");
        u32 width;
        u32 height;
        FrameworkManager::GetWindowSize(width, height);

        if(context.cameraInfo.size() == 0)
            return;

        auto cinfo = context.cameraInfo[0];

        //INFO_LOG("CAMERA: X: {}   Y: {}   Z: {}",cinfo.transform.col(3).x(),cinfo.transform.col(3).y(),cinfo.transform.col(3).z());
        //INFO_LOG("FRONT: X: {}   Y: {}   Z: {}",cinfo.forward.x(),cinfo.forward.y(),cinfo.forward.z());

        Vec3f eye = Vec3f(cinfo.transform.col(3)[0], cinfo.transform.col(3)[1], cinfo.transform.col(3)[2]);
        Vec3f center = eye + Vec3f(cinfo.forward.x(), cinfo.forward.y(), cinfo.forward.z());
        Vec3f up = Vec3f(cinfo.up.x(), cinfo.up.y(), cinfo.up.z());
        auto view = math::LookAt(eye, center, up);
        auto persM = math::Perspective(cinfo.fov_radian, width / float(height), cinfo.near, cinfo.far);

        // Vec4f color = Vec4f(0.2f, 0.3f, 0.3f, 1.f);
        // renderer::GraphicBackend::SetClearColor(color);
        // renderer::GraphicBackend::Clear(renderer::ClearFrameBuffer::Color_Depth);

        //auto manager = ecs::ECSManager::Get();
        for (auto e : _entities)
        {
            auto mesh = static_cast<CMesh *>(_ecs_manager_ptr->GetComponent<CMesh>(e));
            auto trans = static_cast<CTransform *>(_ecs_manager_ptr->GetComponent<CTransform>(e));
            auto t = trans->transform;
            // INFO_LOG("TRANS: X: {}   Y: {}   Z: {}",t.col(3).x(),t.col(3).y(),t.col(3).z());

            auto pgHandle = mesh->renderObject.programHandle;
            renderer::GraphicBackend::UseProgram(pgHandle);
            renderer::GraphicBackend::SetUniform(pgHandle, "diffuse_texture", 0);
            renderer::GraphicBackend::SetUniform(pgHandle, "specular_texture", 1);

            auto reObject = mesh->renderObject;
            std::vector<unknown::renderer::TextureHandle> textures;
            textures.push_back(reObject.diffuseTextureHandles);
            textures.push_back(reObject.specularTextureHandles);
            unknown::renderer::GraphicBackend::UseTexture(textures);

            unknown::renderer::GraphicBackend::SetUniform(pgHandle, "view", view);
            unknown::renderer::GraphicBackend::SetUniform(pgHandle, "projection", persM);
            unknown::renderer::GraphicBackend::SetUniform(pgHandle, "uCamPos", eye);

            tempTime += context.deltaTime;
            Vec3f dirLight = Vec3f(cos(tempTime),sin(tempTime),-1.f);
            unknown::renderer::GraphicBackend::SetUniform(pgHandle, "uDirectionalLight", dirLight);

            // auto p = cubePositions[i];
            // Mat4f model = math::Rotate(Mat4f::Identity(), {1.f, 0.f, 0.f}, math::ToRadian(15 * i));
            // Mat4f model = Mat4f::Identity();
            unknown::renderer::GraphicBackend::SetUniform(pgHandle, "model", trans->transform);
            renderer::GraphicBackend::Draw(reObject.elementHandle);
        }
    }

    void SRender::Setup()
    {
        Require(CMesh::_class_id);
        Require(CTransform::_class_id);
    }
}