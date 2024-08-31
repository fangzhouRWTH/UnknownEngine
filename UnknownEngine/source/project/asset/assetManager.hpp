#pragma once
#include "core/singleton.hpp"
#include "core/handles.hpp"
#include "renderer/rendererHandles.hpp"
#include "memory/resource.hpp"
#include "platform/type.hpp"
#include <string>
#include <unordered_map>
#include <vector>

#include "assets.hpp"

// temp
#include "modelLoader.hpp"

namespace unknown::asset
{
    // class AssetManager : public Singleton<AssetManager>
    // {
    //     friend class Singleton<AssetManager>;

    // public:
    //     virtual void initialize() override;
    //     ~AssetManager(){};
    //     static renderer::RenderElementHandle RequestRenderElement(RenderElementAssetInfo info) { return sInstance->requestRenderElement(info); }
    //     static renderer::TextureHandle RequestTexture(TextureAssetInfo info) { return sInstance->requestTexture(info); }
    //     static renderer::ProgramHandle RequestProgram(ProgramAssetInfo info) { return sInstance->requestProgram(info); }
    //     static LoadedRenderObject RequestRenderObject(RenderElementAssetInfo info) { return sInstance->requestRenderObject(info); }
    //     // static bool LoadMeshRawData(MeshRawData & raw, RenderElementAssetInfo info) { return sInstance->loadMeshRawData(raw, info); }
    //     static bool LoadSceneRawData(SceneRawData &raw, RenderElementAssetInfo info) { return sInstance->loadSceneRawData(raw, info); }

    // protected:
    //     AssetManager(){};

    //     std::unordered_map<u64, renderer::RenderElementHandle> mLoadedRenderElements;
    //     std::unordered_map<u64, renderer::TextureHandle> mLoadedTexture;
    //     std::unordered_map<u64, renderer::ProgramHandle> mLoadedProgram;

    // private:
    //     renderer::RenderElementHandle requestRenderElement(RenderElementAssetInfo info);
    //     renderer::TextureHandle requestTexture(TextureAssetInfo info);
    //     renderer::ProgramHandle requestProgram(ProgramAssetInfo info);
    //     LoadedRenderObject requestRenderObject(RenderElementAssetInfo info);

    //     // bool loadMeshRawData(MeshRawData &raw, RenderElementAssetInfo info);
    //     bool loadSceneRawData(SceneRawData &raw, RenderElementAssetInfo info);
    // };

    // //TODO MEMORY ALLOCATOR
    // //TODO MULTI THREAD
    // class AssetsManager : public Singleton<AssetsManager>
    // {
    //     friend class Singleton<AssetsManager>;
    //     friend class GLTFLoader;

    // public:
    //     std::shared_ptr<MeshAsset> GetMeshAsset(std::string name);
    //     std::shared_ptr<MeshAsset> GetMeshAsset(AssetHandle handle);

    //     AssetHandle GetAssetHandle(std::string name);
    //     AssetInfos GetAssetInfos(AssetHandle handle);

    // private:
    //     AssetsManager();

    //     void load_engine_config_default_assets_info();

    //     virtual void initialize() override {};

    //     static std::string kAssetDefaultRootFolder;
    //     static std::string kAssetDefaultModelFolder;
    //     static std::string kAssetDefaultConfigFolder;

    //     std::unordered_map<std::string,AssetHandle> mNameToHandle;

    //     std::unordered_map<h64,MeshData> mMeshDataMap;

    //     ResourceArray<AssetHandle, AssetInfos> mAssetsInfoMap;       
    //     ResourceArray<AssetHandle, MeshAsset> mMeshAssetMap;
    // };
}