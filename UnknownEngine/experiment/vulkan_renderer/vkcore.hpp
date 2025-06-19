#pragma once
#include "platform/type.hpp"
#include <GLFW/glfw3.h>
#include "core/math.hpp"
#include <memory>

namespace unknown::renderer::vulkan {
struct InitDesc {
  GLFWwindow *glfwWptr;
  uint32_t width;
  uint32_t height;
};

struct TempUpdateData
{
  float deltaTime;
  float time;

  Mat4f view;
  Mat4f proj;
  Mat4f view_proj;
};

class Core {
public:
  bool init(InitDesc desc);
  void shutdown();

  //temp
  void updateData(const TempUpdateData & data);

  void preframe();
  void frame();
  void postframe();

  Core();
  ~Core();

private:
  class Impl;
  std::unique_ptr<Impl> impl = nullptr;
};
} // namespace unknown::renderer::vulkan