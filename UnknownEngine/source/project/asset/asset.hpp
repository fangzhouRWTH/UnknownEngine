#pragma once
#include "core/base.hpp"
#include "core/handles.hpp"
#include "core/math.hpp"
#include "core/structure.hpp"
#include "memory/resource.hpp"
#include "platform/type.hpp"
#include "world/scene.hpp"


#include "renderer/rendererHandles.hpp"

#include <memory>
#include <string>
#include <vector>

// temp
#include "renderer/vulkan/vkTypes.hpp"

namespace unknown::asset {
struct IAssetObject {};

enum struct AssetType {
  Model,
  Texture,
  Material,

  ENUM_MAX,
};

struct AssetMetaData {
  AssetType type{AssetType::ENUM_MAX};
  std::string path;
};

struct AssetHandle {
  h64 hash;
};

enum class SceneContentType {
  Empty,
  Mesh,

  ENUM_MAX,
};

struct ISceneContent {
  virtual ~ISceneContent() {}
  ISceneContent(SceneContentType t) : type(t) {}
  const SceneContentType type;
  Mat4f transform = Mat4f::Identity();
};

struct SceneEmpty : public ISceneContent {
  virtual ~SceneEmpty() {}
  SceneEmpty() : ISceneContent(SceneContentType::Empty) {}
};

struct GeoSurface {
  u32 startIndex;
  u32 count;
  // Material?
};

struct SceneMesh : public ISceneContent {
  virtual ~SceneMesh() {}
  SceneMesh() : ISceneContent(SceneContentType::Mesh) {}
  h64 meshDataHash;
  h64 meshGpuInfoHash = h64(i64(-1));
};

// struct SceneNode
// {
//     std::shared_ptr<ISceneContent> content;
// };

struct SceneData {
  structure::NodeGraph<h64, std::shared_ptr<ISceneContent>> scene;
};

struct SceneObject {};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<u32> indices;
  bool uploaded = false;
  h64 gpuHash;
  // todo
  // GPUMeshBuffers buffers;
  renderer::GPUMeshBufferHandle meshBufferHandle;
};

struct MeshDataHandle : public Handle<MeshDataHandle> {
  template <typename H, typename R> friend class unknown::ResourceMap;

  template <typename H, typename R> friend class unknown::ResourceArray;
};

struct AssetMeshObject : public IAssetObject {
  std::shared_ptr<MeshData> meshDataPtr;
  renderer::GPUMeshBufferHandle meshHandle;
};

struct AssetNode {
  enum class AssetNodeType {
    EMPTY,
    MESH,

    ENUM_MAX,
  };

  Mat4f transform{Mat4f::Identity()};

  std::shared_ptr<IAssetObject> sceneObjectPtr{nullptr};
  std::string nodeName{""};
  AssetNodeType type = AssetNodeType::ENUM_MAX;
};

struct MeshAsset {
  std::string name;
  std::vector<GeoSurface> surfaces;
  GPUMeshBuffers meshBuffers;
};

struct VkVertex {
  Vec3f position;
  float uv_x;
  Vec3f normal;
  float uv_y;
  Vec4f color;
};

class IAssetManager {
public:
  virtual std::shared_ptr<SceneTree> GetSceneTree(h64 hash) = 0;
  virtual std::shared_ptr<MeshData> GetMeshData(h64 hash) = 0;
  virtual bool AddAssetMetaData(std::string_view stringView, AssetType type) = 0;
  virtual bool LoadModelData(h64 hash) = 0;

  virtual void Initialize() = 0;
};
} // namespace unknown::asset