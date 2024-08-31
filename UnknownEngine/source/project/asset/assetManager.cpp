#include "assetManager.hpp"
#include <cassert>
#include <functional>
#include "renderer/renderer.hpp"
#include "textureLoader.hpp"
#include "core/hash.hpp"

#include "modelLoader.hpp"

// #include "stb/stb_image.h"
#include "configuration/globalValues.hpp"
#include "define/enums.hpp"
#include "debug/log.hpp"

#include <cassert>
#include <fstream>
#include <sstream>

#include "serialization/jsonSerializer.hpp"

namespace unknown::asset
{
    // void AssetManager::initialize()
    // {
    //     if (sInitialized)
    //     {
    //         return;
    //     }
    //     TextureLoader::Initialize();
    //     sInitialized = true;
    // }

    // renderer::RenderElementHandle AssetManager::requestRenderElement(RenderElementAssetInfo info)
    // {
    //     // assert(!info.path.empty());
    //     // std::hash<std::string> strhash;
    //     // u64 h = strhash(info.path);

    //     // auto item = mLoadedRenderElements.find(h);
    //     // if (item != mLoadedRenderElements.end())
    //     //     return item->second;

    //     // MeshRawData raw;
    //     // ModelLoader::Load(info.path, raw);
    //     // // u32 vSize = raw.vertices.size() * sizeof(Vertex);
    //     // u32 vSize = raw.vertices.size() * sizeof(VkVertex);
    //     // u32 iSize = raw.indices.size() * sizeof(u32);
    //     // renderer::VertexLayout layout;
    //     // layout.begin()
    //     //     .add(renderer::VertexElement::Position3f)
    //     //     .add(renderer::VertexElement::Normal3f)
    //     //     .add(renderer::VertexElement::TexCoord2f)
    //     //     .end();
    //     // auto rehandle = renderer::GraphicBackend::CreateMesh(layout, (float *)raw.vertices.data(), vSize, (u32 *)raw.indices.data(), iSize);
    //     // mLoadedRenderElements[h] = rehandle;

    //     // for (auto t : raw.textures)
    //     // {
    //     //     TextureAssetInfo tInfo;
    //     //     tInfo.path = t.path;
    //     //     auto txhandle = requestTexture(tInfo);
    //     // }
    //     return renderer::RenderElementHandle();
    // }
    // renderer::TextureHandle AssetManager::requestTexture(TextureAssetInfo info)
    // {
    //     assert(!info.path.empty());
    //     std::hash<std::string> strhash;
    //     u64 h = strhash(std::string(info.path));

    //     auto it = mLoadedTexture.find(h);
    //     if (it != mLoadedTexture.end())
    //         return it->second;

    //     u32 width, height, nrChannels;
    //     auto data = asset::TextureLoader::LoadImage(info.path.c_str(), width, height, nrChannels);

    //     ImageFormat format;
    //     if (nrChannels == 3u)
    //         format = ImageFormat::RGB;
    //     else if (nrChannels == 4u)
    //     {
    //         format = ImageFormat::RGBA;
    //     }
    //     else
    //         assert(false);

    //     renderer::TextureHandle handle;

    //     if (data)
    //     {
    //         handle = renderer::GraphicBackend::CreateTexture(width, height, data, format);
    //         if (handle.IsValid())
    //         {
    //             TextureLoader::FreeImage(data);
    //             mLoadedTexture[h] = handle;
    //             return handle;
    //         }
    //     }
    //     TextureLoader::FreeImage(data);
    //     assert(false);
    //     return handle;
    // }
    // renderer::ProgramHandle AssetManager::requestProgram(ProgramAssetInfo info)
    // {
    //     assert(!info.vsPath.empty() && !info.fsPath.empty());
    //     std::hash<std::string> strhash;
    //     u64 h = strhash(info.vsPath + info.fsPath);

    //     auto item = mLoadedProgram.find(h);
    //     if (item != mLoadedProgram.end())
    //         return item->second;

    //     std::string vertexCode;
    //     std::string fragmentCode;
    //     std::ifstream vShaderFile;
    //     std::ifstream fShaderFile;

    //     // std::string vspath = config::shader_folder_path + vs;
    //     // std::string fspath = config::shader_folder_path + fs;
    //     std::string vspath = info.vsPath;
    //     std::string fspath = info.fsPath;

    //     vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    //     fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    //     try
    //     {
    //         vShaderFile.open(vspath);
    //         fShaderFile.open(fspath);
    //         std::stringstream vShaderStream, fShaderStream;
    //         vShaderStream << vShaderFile.rdbuf();
    //         fShaderStream << fShaderFile.rdbuf();
    //         vShaderFile.close();
    //         fShaderFile.close();
    //         vertexCode = vShaderStream.str();
    //         fragmentCode = fShaderStream.str();
    //     }
    //     catch (const std::ifstream::failure &e)
    //     {
    //         assert(false);
    //     }
    //     const char *vShaderCode = vertexCode.c_str();
    //     const char *fShaderCode = fragmentCode.c_str();

    //     renderer::ProgramHandle handle = renderer::GraphicBackend::CreateProgram(vShaderCode, fShaderCode);
    //     mLoadedProgram[h] = handle;
    //     return handle;
    // }

    // LoadedRenderObject AssetManager::requestRenderObject(RenderElementAssetInfo info)
    // {
    //     assert(!info.path.empty());
    //     std::hash<std::string> strhash;
    //     u64 h = strhash(info.path);

    //     ModelDescription description;
    //     ModelLoader::GetModelDescription(info.path, description);

    //     LoadedRenderObject reObject;

    //     reObject.elementHandle = requestRenderElement(info);

    //     if (u32 count = description.diffuseTexturePaths.size(); count > 0u)
    //     {
    //         if (count > 1u)
    //             INFO_PRINT("ASSET::Trying to load multiple textures of same type, not supported!!!");
    //         TextureAssetInfo tInfo;
    //         tInfo.path = std::string(description.diffuseTexturePaths.begin()->data());
    //         reObject.diffuseTextureHandles = requestTexture(tInfo);
    //     }

    //     if (u32 count = description.specularTexturePaths.size(); count > 0u)
    //     {
    //         if (count > 1u)
    //             INFO_PRINT("ASSET::Trying to load multiple textures of same type, not supported!!!");
    //         TextureAssetInfo tInfo;
    //         tInfo.path = std::string(description.specularTexturePaths.begin()->data());
    //         reObject.specularTextureHandles = requestTexture(tInfo);
    //     }

    //     if (u32 count = description.normalTexturePaths.size(); count > 0u)
    //     {
    //         if (count > 1u)
    //             INFO_PRINT("ASSET::Trying to load multiple textures of same type, not supported!!!");
    //         TextureAssetInfo tInfo;
    //         tInfo.path = std::string(description.normalTexturePaths.begin()->data());
    //         reObject.normalTextureHandles = requestTexture(tInfo);
    //     }

    //     if (u32 count = description.roughnessTexurePaths.size(); count > 0u)
    //     {
    //         if (count > 1u)
    //             INFO_PRINT("ASSET::Trying to load multiple textures of same type, not supported!!!");
    //         TextureAssetInfo tInfo;
    //         tInfo.path = std::string(description.roughnessTexurePaths.begin()->data());
    //         reObject.roughnessTextureHandles = requestTexture(tInfo);
    //     }

    //     if (u32 count = description.aoTexturePaths.size(); count > 0u)
    //     {
    //         if (count > 1u)
    //             INFO_PRINT("ASSET::Trying to load multiple textures of same type, not supported!!!");
    //         TextureAssetInfo tInfo;
    //         tInfo.path = std::string(description.aoTexturePaths.begin()->data());
    //         reObject.aoTextureHandles = requestTexture(tInfo);
    //     }

    //     return reObject;
    // }

    // // bool AssetManager::loadMeshRawData(MeshRawData &raw, RenderElementAssetInfo info)
    // // {
    // //     assert(!info.path.empty());
    // //     // std::hash<std::string> strhash;
    // //     // u64 h = strhash(info.path);

    // //     // auto item = mLoadedRenderElements.find(h);
    // //     // if (item != mLoadedRenderElements.end())
    // //     //     return item->second;
    // //     ModelLoader::Load(info.path, raw);
    // //     assert(raw.subMeshIndicesRanges.size() == raw.subMeshVerticesRanges.size());
    // //     return true;
    // // }

    // bool AssetManager::loadSceneRawData(SceneRawData &raw, RenderElementAssetInfo info)
    // {
    //     assert(!info.path.empty());
    //     // std::hash<std::string> strhash;
    //     // u64 h = strhash(info.path);

    //     // auto item = mLoadedRenderElements.find(h);
    //     // if (item != mLoadedRenderElements.end())
    //     //     return item->second;
    //     ModelLoader::LoadGLTF(info.path, raw);
    //     assert(raw.subMeshIndicesRanges.size() == raw.subMeshVerticesRanges.size());
    //     return true;
    // }

    // // AssetsManager
    // std::string AssetsManager::kAssetDefaultRootFolder = "";
    // std::string AssetsManager::kAssetDefaultModelFolder = config::model_folder_path;
    // std::string AssetsManager::kAssetDefaultConfigFolder = config::config_folder_path;

    // void parse_file_name(std::string_view path, std::string &folderPath, std::string &fileName)
    // {
    //     auto pos = path.rfind('/');
    //     folderPath = path.substr(0, pos == std::string::npos ? path.length() : pos + 1);
    //     fileName = path.substr(pos == std::string::npos ? path.length() : pos + 1, path.length());
    // }

    // AssetInfos create_asset_info(u64 hash, std::string_view path, std::string_view name, std::string_view type)
    // {
    //     AssetInfos infos;
    //     infos.metaData.hash = hash;
    //     infos.metaData.name = name;
    //     infos.metaData.path = path;
    //     if (type == "mesh")
    //         infos.metaData.type = AssetType::Mesh;
    //     return infos;
    // }

    // std::shared_ptr<MeshAsset> load_mesh_asset(AssetInfos info)
    // {
    //     assert(info.metaData.type == AssetType::Mesh);
    //     SceneRawData raw;

    //     ModelLoader::LoadMesh(info.metaData.path+info.metaData.name,raw);
        
    //     return nullptr;
    // }

    // std::shared_ptr<MeshAsset> AssetsManager::GetMeshAsset(std::string name)
    // {
    //     auto it = mNameToHandle.find(name);
    //     if (it == mNameToHandle.end())
    //         return nullptr;

    //     return GetMeshAsset(it->second);
    // }

    // std::shared_ptr<MeshAsset> AssetsManager::GetMeshAsset(AssetHandle handle)
    // {
    //     assert(handle.IsValid());

    //     AssetInfos info;
    //     assert(mAssetsInfoMap.FindResource(handle, info));

    //     if (info.metaData.type != AssetType::Mesh)
    //     {
    //         INFO_LOG("ASSET MANAGER: REQUIRING WRONG TYPE : {}", info.metaData.name);
    //         return nullptr;
    //     }

    //     std::shared_ptr<MeshAsset> mPtr;
    //     if (mMeshAssetMap.FindResource(handle, mPtr))
    //     {
    //         return mPtr;
    //     }

    //     mPtr = load_mesh_asset(info);
    //     if (!mPtr)
    //     {
    //         INFO_LOG("ASSET MANAGER: FAILED LOADING : {}", info.metaData.name);
    //         return nullptr;
    //     }

    //     //mMeshAssetMap.insert({handle,mPtr});
    //     return mPtr;
    // }

    // AssetHandle AssetsManager::GetAssetHandle(std::string name)
    // {
    //     auto it = mNameToHandle.find(name);
    //     if (it == mNameToHandle.end())
    //         return AssetHandle();

    //     return it->second;
    // }

    // AssetInfos AssetsManager::GetAssetInfos(AssetHandle handle)
    // {
    //     assert(handle.IsValid());
    //     AssetInfos info;
    //     assert(mAssetsInfoMap.FindResource(handle, info));
    //     return info;
    // }

    // AssetsManager::AssetsManager()
    // {
    //     load_engine_config_default_assets_info();
    // }

    // void AssetsManager::load_engine_config_default_assets_info()
    // {
    //     std::string config = kAssetDefaultConfigFolder + "EngineConfig.json";
    //     std::ifstream file(config);
    //     JsonS data = JsonS::parse(file);

    //     auto assetsInfo = data["assets"];
    //     auto assetsEntries = assetsInfo.find("entries");
    //     if (assetsEntries != assetsInfo.end())
    //     {
    //         for (auto it = assetsEntries->begin(); it != assetsEntries->end(); it++)
    //         {
    //             auto folderpath = it->find("folder");
    //             if (folderpath == it->end())
    //                 continue;

    //             INFO_PRINT(*folderpath);

    //             auto items = it->find("items");
    //             if (items == it->end())
    //                 continue;

    //             for (auto itm = items->begin(); itm != items->end(); itm++)
    //             {
    //                 auto type = itm->find("type");
    //                 auto filename = itm->find("filename");

    //                 std::string _file_path = *folderpath;
    //                 std::string _file_name = *filename;
    //                 auto fullpath = _file_path + _file_name;

    //                 parse_file_name(fullpath, _file_path, _file_name);

    //                 if (mNameToHandle.find(_file_name) != mNameToHandle.end())
    //                 {
    //                     INFO_LOG("ASSET MANAGER WARN: ASSET {} , NAME REGISTERED", fullpath);
    //                     continue;
    //                 }

    //                 u64 h = math::HashString(fullpath);

    //                 auto info = create_asset_info(h, _file_path, _file_name, std::string(*type));

    //                 auto aHandle = mAssetsInfoMap.AddResource(info);

    //                 if (!aHandle.IsValid())
    //                 {
    //                     INFO_LOG("ASSET MANAGER WARN: ASSET {} , FAIL REGISTERATION", fullpath);
    //                     continue;
    //                 }

    //                 mNameToHandle.insert({_file_name, aHandle});

    //                 if (type != itm->end() && filename != itm->end())
    //                     INFO_LOG("ASSET MANAGER REGISTRATION: FOLDER {}, ASSET {}, TYPE: {} ", _file_path, _file_name, *type);
    //             }
    //         }
    //     }
    // }
}