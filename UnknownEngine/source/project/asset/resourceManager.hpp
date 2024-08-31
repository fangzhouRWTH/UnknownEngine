#pragma once
#include "platform/type.hpp"
#include "cassert"
#include "asset/assets.hpp"

#include <memory>
#include <string>
#include <unordered_map>

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
        std::shared_ptr<SceneData> GetSceneResource(h64 hash);
        std::shared_ptr<SceneData> AcquireSceneResource(h64 hash);

        std::unordered_map<h64, std::shared_ptr<SceneData>> mSceneResource;
    };

    class ResourceManager
    {
    public:
        static std::shared_ptr<ResourceManager> Get();

        std::shared_ptr<SceneData> GetSceneData(h64 hash);
        std::shared_ptr<MeshData> GetMeshData(h64 hash);

        bool AddResourceMetaData(std::string_view stringView, ResourceType type);
        bool LoadModelData(h64 hash);

    private:
        void Initialize();
        static std::shared_ptr<ResourceManager> sInstance;

        MeshResourceBank mMeshResource;
        SceneResourceBank mSceneResource;

        std::unordered_map<h64, ResourceMetaData> mResourceMetaDataMap;
    };
}