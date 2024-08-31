#pragma once
#include "platform/type.hpp"
#include "core/handles.hpp"
#include "memory/resource.hpp"
#include "math.h"
#include "core/structure.hpp"
#include "world/scene.hpp"

#include "renderer/rendererHandles.hpp"

#include <memory>
#include <string>
#include <vector>

// temp
#include "renderer/vulkan/vkTypes.hpp"

namespace unknown::asset
{
    struct Vertex
    {
        Vec3f position;
        float uv_x;
        Vec3f normal;
        float uv_y;
        Vec4f color;
    };

    struct IAssetObject
    {
    };

    enum class SceneContentType
    {
        Empty,
        Mesh,

        ENUM_MAX,
    };

    struct ISceneContent
    {
        virtual ~ISceneContent() {}
        ISceneContent(SceneContentType t) : type(t) {}
        const SceneContentType type;
        Mat4f transform = Mat4f::Identity();
    };

    struct SceneEmpty : public ISceneContent
    {
        virtual ~SceneEmpty() {}
        SceneEmpty() : ISceneContent(SceneContentType::Empty) {}
    };

    struct GeoSurface
    {
        u32 startIndex;
        u32 count;
        // Material?
    };

    struct SceneMesh : public ISceneContent
    {
        virtual ~SceneMesh() {}
        SceneMesh() : ISceneContent(SceneContentType::Mesh) {}
        h64 meshDataHash;
        h64 meshGpuInfoHash = h64(i64(-1));
    };

    // struct SceneNode
    // {
    //     std::shared_ptr<ISceneContent> content;
    // };

    struct SceneData
    {
        structure::NodeGraph<h64, std::shared_ptr<ISceneContent>> scene;
    };

    struct SceneObject
    {

    };

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
    };

    struct MeshDataHandle : public Handle<MeshDataHandle>
    {
        template <typename H, typename R>
        friend class unknown::ResourceMap;

        template <typename H, typename R>
        friend class unknown::ResourceArray;
    };

    struct AssetMeshObject : public IAssetObject
    {
        std::shared_ptr<MeshData> meshDataPtr;
        renderer::GPUMeshBufferHandle meshHandle;
    };

    struct AssetNode
    {
        enum class AssetNodeType
        {
            EMPTY,
            MESH,

            ENUM_MAX,
        };

        Mat4f transform{Mat4f::Identity()};

        std::shared_ptr<IAssetObject> sceneObjectPtr{nullptr};
        std::string nodeName{""};
        AssetNodeType type = AssetNodeType::ENUM_MAX;
    };

    struct AssetHandle : public Handle<AssetHandle>
    {
        template <typename H, typename R>
        friend class unknown::ResourceMap;

        template <typename H, typename R>
        friend class unknown::ResourceArray;
    };

    enum class AssetType
    {
        RenderElement,
        Texture,
        Shader,

        Mesh,

        ENUM_MAX,
    };

    struct AssetMetaData
    {
        u64 hash;
        std::string name = "";
        AssetType type = AssetType::ENUM_MAX;
        std::string path = "";
    };

    struct AssetInfos : public ResourceInfos<AssetHandle>
    {
        u32 assetId;
        AssetMetaData metaData;
        //structure::NodeTree<std::string, AssetNode> sceneObjectTree;
    };

    struct MeshAsset
    {
        std::string name;
        std::vector<GeoSurface> surfaces;
        GPUMeshBuffers meshBuffers;
    };

    struct VkVertex
    {
        Vec3f position;
        float uv_x;
        Vec3f normal;
        float uv_y;
        Vec4f color;
    };

    struct RenderElementAssetInfo
    {
        std::string path;
        const AssetType type = AssetType::RenderElement;
    };

    struct TextureAssetInfo
    {
        std::string path;
        const AssetType type = AssetType::Texture;
    };

    struct ProgramAssetInfo
    {
        std::string vsPath;
        std::string fsPath;

        const AssetType type = AssetType::Shader;
    };

    struct LoadedRenderObject
    {
        renderer::RenderElementHandle elementHandle;

        renderer::TextureHandle diffuseTextureHandles;
        renderer::TextureHandle specularTextureHandles;
        renderer::TextureHandle normalTextureHandles;
        renderer::TextureHandle roughnessTextureHandles;
        renderer::TextureHandle aoTextureHandles;

        renderer::ProgramHandle programHandle;
    };
}