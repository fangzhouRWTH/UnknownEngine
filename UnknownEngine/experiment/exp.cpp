#include "exp.hpp"

#include <iostream>
#include <string>


#include "misc.hpp"
//#include "simpleVK.hpp"
#include "vulkan_renderer/vkcore.hpp"

#include "framework/framework.hpp"

#include "core/math.hpp"
#include "ecs/components/cameraComponent.hpp"
#include "ecs/components/inputControllerComponent.hpp"
#include "ecs/components/transformComponent.hpp"
#include "ecs/ecs.hpp"
#include "ecs/entities/actorEntity.hpp"
#include "ecs/systems/cameraSystem.hpp"
#include "ecs/systems/inputControllerSystem.hpp"
#include "core/clock.hpp"


using namespace unknown;

typedef std::string KSTR;
KSTR kEcsDefaultRenderSystem = "RenderSystem_Default";
KSTR kEcsDefaultCameraSystem = "CameraSystem_Default";
KSTR kEcsDefaultInputControllerSystem = "InputControllerSystem_Default";

void DefaultPlayerController(unknown::ecs::CTransform *transformComponent,
                             const unknown::input::KeyEvents &keyEvent,
                             const unknown::input::CursorPosition &cursor,
                             float dt) {
  float speed = 10.f;

  auto dx = cursor._current.x() - cursor._previous.x();
  auto dy = cursor._current.y() - cursor._previous.y();
  float yaw = dx * dt * 0.01f * 3.14159f;
  float pitch = dy * dt * 0.01f * 3.14159f;
  transformComponent->yaw += yaw;
  float npitch =
      unknown::math::Clamp(transformComponent->pitch + pitch, -M_HPI, M_HPI);
  pitch = npitch - transformComponent->pitch;
  transformComponent->pitch = npitch;
  unknown::Mat4f linear = unknown::Mat4f::Identity();

  linear.block<3, 3>(0, 0) = transformComponent->transform.block<3, 3>(0, 0);

  linear = unknown::math::Rotate(linear, unknown::Vec3f(0.f, 0.f, -1.f), yaw);
  unknown::Vec3f right =
      unknown::Vec3f(linear.col(0).x(), linear.col(0).y(), linear.col(0).z());
  linear = unknown::math::Rotate(linear, -right, pitch);

  if (keyEvent.down.IsSet(unknown::input::Key::W)) {
    unknown::Vec4f move = linear.col(1);
    transformComponent->transform.col(3) += move * speed * dt;
  }
  if (keyEvent.down.IsSet(unknown::input::Key::S)) {
    unknown::Vec4f move = linear.col(1);
    transformComponent->transform.col(3) -= move * speed * dt;
  }
  if (keyEvent.down.IsSet(unknown::input::Key::D)) {
    unknown::Vec4f move = linear.col(0);
    transformComponent->transform.col(3) += move * speed * dt;
  }
  if (keyEvent.down.IsSet(unknown::input::Key::A)) {
    unknown::Vec4f move = linear.col(0);
    transformComponent->transform.col(3) -= move * speed * dt;
  }

  transformComponent->transform.block<3, 3>(0, 0) = linear.block<3, 3>(0, 0);
  auto t = transformComponent->transform;
  //  INFO_LOG("TRANS: X: {}   Y: {}   Z: {}", t.col(3).x(), t.col(3).y(),
  //  t.col(3).z()); INFO_LOG("FORWARD: X: {}   Y: {}   Z:
  //  {}",linear.col(1).x(),linear.col(1).y(),linear.col(1).z()); INFO_LOG("dX:
  //  {}   dY: {} ", dx, dy); INFO_LOG("yaw: {}   pitch: {} ", yaw, pitch);
};

int main() {
  uint32_t height = 1080u;
  uint32_t width = 1920u;

  // Framework initialization
  {
    FrameworkInfo fInfo;
    fInfo.cursorMode = input::CursorMode::Default;
    fInfo.name = "EXP V 0 0 0";
    fInfo.width = 1920u;
    fInfo.height = 1080u;
    FrameworkManager::Initialize(fInfo);
  }

  renderer::vulkan::Core vkCore;

  renderer::vulkan::InitDesc desc;

  desc.glfwWptr =
      static_cast<GLFWwindow *>(FrameworkManager::GetWindowRawPointer());
  desc.height = height;
  desc.width = width;

  vkCore.init(desc);

  auto clock = std::shared_ptr<core::Clock>(new core::Clock);
  clock->Activate();

  auto ecsManager = std::shared_ptr<ecs::ECSManager>(new ecs::ECSManager);
  ecs::Initializer initializer(ecsManager);

  auto controllersystem_h = initializer.AddSystem<ecs::SInputController>(
      kEcsDefaultInputControllerSystem);
  auto camerasystem_h =
      initializer.AddSystem<ecs::SCamera>(kEcsDefaultCameraSystem);

  // Build Systems Depandency
  bool buildRes = initializer.BuildGraph();
  assert(buildRes);

  ecsManager->Initialize();

  auto player_default_h =
      ecsManager->CreateEntity<ecs::EPlayer>("player_default");
  ecsManager->CreateComponent<ecs::EPlayer, ecs::CTransform>(
      player_default_h._entity_id);

  // temp
  auto pTrans =
      ecsManager->GetComponent<ecs::CTransform>(player_default_h._entity_id);
  static_cast<ecs::CTransform *>(pTrans)->transform.col(3) =
      Vec4f(0.f, -2.8f, 0.f, 1.f);
  // temp

  ecsManager->CreateComponent<ecs::EPlayer, ecs::CCamera>(
      player_default_h._entity_id);
  ecsManager->CreateComponent<ecs::EPlayer, ecs::CInputController>(
      player_default_h._entity_id);

  auto controller = static_cast<ecs::CInputController *>(
      ecsManager->GetComponent<ecs::CInputController>(
          player_default_h._entity_id));
  controller->controllTransformComponent = DefaultPlayerController;
  controller->target =
      ecs::CInputController::ControllTarget::TransformComponent;

  ecsManager->RegisterEntity<ecs::SInputController, ecs::EPlayer>(
      controllersystem_h._system_id, player_default_h._entity_id);
  ecsManager->RegisterEntity<ecs::SCamera, ecs::EPlayer>(
      camerasystem_h._system_id, player_default_h._entity_id);

  while (FrameworkManager::RunningMain()) {
    clock->CheckDeltaTime();
    FrameworkManager::Update();
    u32 windowHeight = 0u;
    u32 windowWidth = 0u;
    FrameworkManager::GetWindowSize(windowWidth, windowHeight);
    float dt = clock->GetDeltaTime().count();
    EngineContext context;
    context.deltaTime = dt;
    context.ecsManager = ecsManager;
    ecsManager->Update(context);

    const auto &cInfo = context.cameraInfo[0];

    Vec3f eye = Vec3f(cInfo.transform.col(3)[0], cInfo.transform.col(3)[1],
                      cInfo.transform.col(3)[2]);
    Vec3f center =
        eye + Vec3f(cInfo.forward.x(), cInfo.forward.y(), cInfo.forward.z());
    Vec3f up = Vec3f(cInfo.up.x(), cInfo.up.y(), cInfo.up.z());
    auto view = math::LookAt(eye, center, up);
    auto projection = math::PerspectiveVK(
        cInfo.fov_radian, (float)1920.f / (float)1080.f, cInfo.near, cInfo.far);
    // window.update();
    
    renderer::vulkan::TempUpdateData tData;
    tData.view = view;
    tData.proj = projection;
    tData.view_proj = projection * view;
    tData.deltaTime = dt;
    tData.time = clock->CheckTimePast().count();
    vkCore.updateData(tData);
    vkCore.preframe();
    vkCore.frame();
    vkCore.postframe();

    // window.postUpdate();
    FrameworkManager::PostUpdate();
  }
  vkCore.shutdown();
  // window.terminate();
  FrameworkManager::TerminateMain();
}