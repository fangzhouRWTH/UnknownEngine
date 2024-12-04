#pragma once
#include "platform/type.hpp"
#include "cassert"
#include "asset/asset.hpp"
#include "world/scene.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace unknown::renderer
{
    // TODO Temp
    class IRenderer;
}

namespace unknown::renderer::vulkan
{
    // TODO Temp
    class VulkanCore;
}

namespace unknown::asset
{
    struct MeshAssetBank
    {
        std::shared_ptr<MeshData> GetMeshAsset(h64 hash);
        std::shared_ptr<MeshData> AcquireMeshAsset(h64 hash);

        std::unordered_map<h64, std::shared_ptr<MeshData>> mMeshAsset;
    };

    struct SceneAssetBank
    {
        std::shared_ptr<SceneTree> GetSceneAsset(h64 hash);
        std::shared_ptr<SceneTree> AcquireSceneAsset(h64 hash);

        std::unordered_map<h64, std::shared_ptr<SceneTree>> mSceneAsset;
    };

    class AssetManager final : public IAssetManager
    {
    public:
        static void DebugPrintAssetHierarchy(std::string_view assetPath);

        AssetManager(std::shared_ptr<renderer::IRenderer> renderer) : mpRenderer(renderer) {}

        virtual std::shared_ptr<SceneTree> GetSceneTree(h64 hash) override;
        virtual std::shared_ptr<MeshData> GetMeshData(h64 hash) override;

        virtual bool AddAssetMetaData(std::string_view stringView, AssetType type) override;
        virtual bool LoadModelData(h64 hash) override;

        virtual void Initialize() override;

    private:
        std::shared_ptr<renderer::IRenderer> mpRenderer;

        MeshAssetBank mMeshAsset;
        SceneAssetBank mSceneAsset;

        std::unordered_map<h64, AssetMetaData> mAssetMetaDataMap;

        bool mbInitialized = false;
    };
}