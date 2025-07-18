#pragma once

#include "core/bit.hpp"
#include "platform/type.hpp"
#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/enum.hpp"
#include "vulkan_renderer/resource.hpp"
#include "vulkan_renderer/types.hpp"
#include "vulkan_renderer/vulkanContext.hpp"
#include <memory>
#include <variant>

namespace unknown::renderer::vulkan {
struct CompileContext {
  u32 target_width;
  u32 target_height;
  u32 target_depth;
};

struct FieldImage {
  u32 width;
  u32 height;
  u32 depth = 1u;
  ImageFormat format;
};

struct FieldSSBO {};

enum struct FieldType { Undefined, Image, ShaderStorage };

enum class FieldAccess {
  Undefined,
  Input,
  Output,

  Local,
  Global,

  ENUM_MAX
};

struct PassReflection {
  struct Field {
    std::string name;
    std::string desc;
    BitMask<FieldAccess> access;

    FieldType type;
    std::variant<FieldImage, FieldSSBO> data;

    Field &ssbo() {
      type = FieldType::ShaderStorage;
      data = FieldSSBO{};
      return *this;
    }

    Field &image() {
      type = FieldType::Image;
      data = FieldImage{};
      return *this;
    }

    Field &format(EImageFormat format) {
      assert(type==FieldType::Image);
      auto & f = std::get<FieldImage>(data).format;
      switch(format)
      {
        case EImageFormat::RGBA16Float:
          f.rgba_16_sf();
          break;
        case EImageFormat::D32Float:
          f.d_32_sf();
          break;
        default:
          assert(false);
      }
      return *this;
    };

    Field &depth(u32 width, u32 height) {
      assert(type==FieldType::Image);
      auto & f = std::get<FieldImage>(data);
      f.width = width;
      f.height = height;
      f.format.depth_aspect();
    }

    Field &texture2D(u32 width, u32 height) {
      assert(type==FieldType::Image);
      auto & f = std::get<FieldImage>(data);
      f.width = width;
      f.height = height;
      f.format.color_aspect();
    }
  };

  Field &addInput(const std::string &name, const std::string &desc) {
    Field field = Field{.name = name, .desc = desc};
    field.access.Set(FieldAccess::Input);
    return addField(field);
  }

  Field &addOutput(const std::string &name, const std::string &desc) {
    Field field = Field{.name = name, .desc = desc};
    field.access.Set(FieldAccess::Output);
    return addField(field);
  }

  Field &addInputOutput(const std::string &name, const std::string &desc) {
    Field field = Field{.name = name, .desc = desc};
    field.access.Set(FieldAccess::Input);
    field.access.Set(FieldAccess::Output);
    return addField(field);
  }

  Field &addLocal(const std::string &name, const std::string &desc) {
    Field field = Field{.name = name, .desc = desc};
    field.access.Set(FieldAccess::Local);
    return addField(field);
  }

  Field &addGlobal(const std::string &name, const std::string &desc) {
    Field field = Field{.name = name, .desc = desc};
    field.access.Set(FieldAccess::Global);
    return addField(field);
  }

private:
  Field &addField(const Field &field) {
    // TODO filter/merger/checker

    mFields.push_back(field);
    return mFields.back();
  }

  std::vector<Field> mFields;
};

class IRenderPass {
public:
  IRenderPass(const std::string &name) : mPassName(name) {}
  virtual ~IRenderPass() {}
  virtual PassReflection reflect(const CompileContext &rgCtx) = 0;

  virtual void Prepare(VulkanContext ctx){};
  virtual void Execute(VulkanContext ctx){};
  virtual void End(VulkanContext ctx){};

  std::string GetName() { return mPassName; }
  bool IsForced() {return mForceExecution; }

protected:
  bool mForceExecution = false;
  std::string mPassName;
};

class ResourcePass : public IRenderPass {
public:
  ResourcePass(const std::string &name) : IRenderPass(name) {
    mForceExecution = true;
  }
  virtual PassReflection reflect(const CompileContext &rgCtx) override {
    PassReflection r;
    r.addGlobal("Global_Uniforms_1", "external uniforms batch 1").ssbo();
    r.addGlobal("Global_Instance_1", "external ssbo batch 1").ssbo();
    r.addGlobal("Global_Frame_Target_1", "external render target 1").image();

    return r;
  }

private:
};

class ComputeTestPass : public IRenderPass {
  public:
    ComputeTestPass(const std::string & name) : IRenderPass(name) {}
    virtual PassReflection reflect(const CompileContext & rgCtx) override {
      PassReflection r;
      r.addOutput("dst", "compute texture test pass").image();
      return r;
    }
  private:
};

class ClearPass : public IRenderPass {
  public:
    ClearPass(const std::string & name) : IRenderPass(name) {}
    virtual PassReflection reflect(const CompileContext &rgCtx) override {
          PassReflection r;
    r.addOutput("dst", "clear output").image();

    return r;
    }
    private:
};

class BackgroundPass : public IRenderPass {
public:
  BackgroundPass(const std::string & name) : IRenderPass(name) {}
  virtual PassReflection reflect(const CompileContext &rgCtx) override {
    PassReflection r;
    r.addOutput("dst", "background output").image();

    return r;
  }
private:

};

class MeshTestPass : public IRenderPass {
public:
  MeshTestPass(const std::string & name) : IRenderPass(name) {}
  virtual PassReflection reflect(const CompileContext &rgCtx) override {
    PassReflection r;
    //r.addInput("dummy_texture", "compute texture test input").image();
    //r.addInput("src", "mesh test input").image();
    r.addOutput("dst", "mesh test pass render target").image();
    return r;
  }

private:
};

class PostProcessTestPass : public IRenderPass {
public:
  PostProcessTestPass(const std::string &name) : IRenderPass(name) {}
  virtual PassReflection reflect(const CompileContext &rgCtx) override {
    PassReflection r;
    r.addInput("src", "post process pass input").image();
    r.addOutput("dst", "post process pass render target").image();
    return r;
  }
};

class BlitPass : public IRenderPass {
  public:
  BlitPass(const std::string & name) : IRenderPass(name) {}
  virtual PassReflection reflect(const CompileContext &rgCtx) override {
    PassReflection r;
    r.addInput("src", "blit pass source image").image();
    r.addOutput("dst", "blit pass destination image").image();
    return r;
  }
};
} // namespace unknown::renderer::vulkan