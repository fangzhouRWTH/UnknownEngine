#pragma once
#include "platform/type.hpp"
#include "cassert"
#include "asset/assets.hpp"
#include "world/scene.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace unknown::renderer::vulkan
{
    // TODO Temp
    class VulkanCore;
}

namespace unknown::asset
{
    enum struct ResourceType
    {
        Model,
        Texture,
        Material,

        ENUM_MAX,
    };

    struct ResourceMetaData
    {
        ResourceType type{ResourceType::ENUM_MAX};
        std::string path;
    };

    struct ResourceHandle
    {
        h64 hash;
    };

    struct MeshResourceBank
    {
        std::shared_ptr<MeshData> GetMeshResource(h64 hash);
        std::shared_ptr<MeshData> AcquireMeshResource(h64 hash);

        std::unordered_map<h64, std::shared_ptr<MeshData>> mMeshResource;
    };

    struct SceneResourceBank
    {
        // std::shared_ptr<SceneData> GetSceneResource(h64 hash);
        // std::shared_ptr<SceneData> AcquireSceneResource(h64 hash);

        // std::unordered_map<h64, std::shared_ptr<SceneData>> mSceneResource;

        std::shared_ptr<SceneTree> GetSceneResource(h64 hash);
        std::shared_ptr<SceneTree> AcquireSceneResource(h64 hash);

        std::unordered_map<h64, std::shared_ptr<SceneTree>> mSceneResource;
    };

    class ResourceManager
    {
    public:
        static void DebugPrintAssetHierarchy(std::string_view assetPath);
        static std::shared_ptr<ResourceManager> Get();

        std::shared_ptr<SceneTree> GetSceneTree(h64 hash);
        std::shared_ptr<MeshData> GetMeshData(h64 hash);

        bool AddResourceMetaData(std::string_view stringView, ResourceType type);
        bool LoadModelData(h64 hash);

        void SetRenderBackend(renderer::vulkan::VulkanCore *vkCore) { mVkCore = vkCore; }

        void Initialize();

    private:
        ResourceManager() {}
        static std::shared_ptr<ResourceManager> sInstance;

        MeshResourceBank mMeshResource;
        SceneResourceBank mSceneResource;

        std::unordered_map<h64, ResourceMetaData> mResourceMetaDataMap;
        renderer::vulkan::VulkanCore *mVkCore = nullptr;

        bool mbInitialized = false;
    };
}