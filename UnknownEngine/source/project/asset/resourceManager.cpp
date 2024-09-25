#include "resourceManager.hpp"
#include "core/hash.hpp"
#include "debug/log.hpp"
#include "configuration/globalValues.hpp"

#include "renderer/vulkan/vkCore.hpp"

#include "utils/assimpHelper.hpp"

#include <iostream>
#include <fstream>

namespace unknown::asset
{
    //std::shared_ptr<ResourceManager> ResourceManager::sInstance = nullptr;

    struct SceneProcessData
    {
        const aiScene *scene;
        const std::string_view path;
        const h64 pathHash;
        std::shared_ptr<SceneData> const sceneData;

        aiNode *node;
        std::string parentName;
        h64 parentH = 0u;
        MeshResourceBank &meshResource;

        SceneProcessData(const aiScene *s,
                         std::string_view p,
                         const h64 &h,
                         MeshResourceBank &mResource,
                         std::shared_ptr<SceneData> sData) : scene(s),
                                                             path(p),
                                                             pathHash(h),
                                                             meshResource(mResource),
                                                             sceneData(sData) {}
    };

    struct MeshProcessData
    {
        h64 hash;
        aiMesh *mesh;
        MeshResourceBank &meshResource;

        MeshProcessData(MeshResourceBank &mResource) : meshResource(mResource) {}
    };

    std::shared_ptr<MeshData> process_scene_mesh(MeshProcessData mData)
    {
        auto res = mData.meshResource.AcquireMeshResource(mData.hash);
        if (!res)
            assert(false);
        auto mesh = mData.mesh;

        const u32 numVertices = mesh->mNumVertices;

        for (u32 i = 0u; i < numVertices; i++)
        {
            const auto &inputV = mesh->mVertices[i];
            // auto &v = res.vertices[i];
            Vertex v;
            v.position.x() = inputV.x;
            v.position.y() = inputV.y;
            v.position.z() = inputV.z;

            const auto &inputN = mesh->mNormals[i];
            v.normal.x() = inputN.x;
            v.normal.y() = inputN.y;
            v.normal.z() = inputN.z;

            // debug
            {
                v.color.segment(0, 3) = v.normal;
            }

            if (mesh->mTextureCoords[0])
            {
                const auto &inputT = mesh->mTextureCoords[0][i];
                // v.texCoords.x() = inputT.x;
                // v.texCoords.y() = inputT.y;
                v.uv_x = inputT.x;
                v.uv_y = inputT.y;
            }
            else
            {
                v.uv_x = 0.f;
                ;
                v.uv_y = 0.f;
                // v.texCoords = Vec2f(0, 0);
            }

            res->vertices.push_back(v);
        }

        const u32 numFaces = mesh->mNumFaces;
        for (u32 i = 0; i < numFaces; i++)
        {
            const aiFace &inputF = mesh->mFaces[i];
            const auto numIdx = inputF.mNumIndices;
            for (u32 j = 0u; j < numIdx; j++)
            {
                res->indices.push_back(inputF.mIndices[j]);
            }
        }

        if (res->indices.empty() || res->vertices.empty())
            // TODO REMOVE RESOURCE
            return nullptr;

        return res;
    }

    void process_scene_node(SceneProcessData spData, h64 &nodeCounter)
    {
        if (!spData.node)
            return;

        auto node = spData.node;
        u32 numMeshes = node->mNumMeshes;
        u32 numChilds = node->mNumChildren;

        if (numMeshes == 0u && numChilds == 0u)
            return;

        auto aTransform = node->mTransformation;
        Mat4f nTransform;
        nTransform << aTransform.a1, aTransform.a2, aTransform.a3, aTransform.a4,
            aTransform.b1, aTransform.b2, aTransform.b3, aTransform.b4,
            aTransform.c1, aTransform.c2, aTransform.c3, aTransform.c4,
            aTransform.d1, aTransform.d2, aTransform.d3, aTransform.d4;

        auto nodeName = std::string(node->mName.C_Str());

        std::shared_ptr<SceneEmpty> scPtr = std::make_shared<SceneEmpty>();

        scPtr->transform = nTransform;
        // scene
        h64 pNodeH = spData.parentH;
        h64 nodeH = ++nodeCounter;
        spData.sceneData->scene.AddNode(nodeH, scPtr);
        if (spData.parentH != 0)
        {
            spData.sceneData->scene.AddEdge(pNodeH, nodeH);
        }
        // mesh
        for (u32 i = 0u; i < numMeshes; i++)
        {
            std::shared_ptr<SceneMesh> smPtr = std::make_shared<SceneMesh>();
            smPtr->transform = nTransform;
            MeshProcessData mpData(spData.meshResource);
            mpData.mesh = spData.scene->mMeshes[node->mMeshes[i]];
            std::stringstream meshName;
            meshName << '#' << mpData.mesh->mName.C_Str();
            // std::string meshName = std::string("#") + mpData.mesh->mName.C_Str();
            std::stringstream meshHashName;
            meshHashName << spData.path.data() << meshName.str();
            mpData.hash = math::HashString(meshHashName.str());
            auto mPtr = process_scene_mesh(mpData);
            assert(mPtr);
            smPtr->meshDataHash = mpData.hash;
            meshName << '#' << i;
            auto key = meshName.str();

            h64 mNodeH = ++nodeCounter;

            spData.sceneData->scene.AddNode(mNodeH, smPtr);
            spData.sceneData->scene.AddEdge(pNodeH, mNodeH);
        }

        for (u32 i = 0u; i < numChilds; i++)
        {
            SceneProcessData cspData = spData;
            cspData.node = node->mChildren[i];
            cspData.parentName = nodeName;
            cspData.parentH = nodeH;
            process_scene_node(cspData, nodeCounter);
        }
    }

    void debug_scene_process_node(std::ofstream &ofs, const aiScene *scene, aiNode *node, u32 indent)
    {
        for (auto i = 0u; i < indent; i++)
        {
            ofs << "--";
        }
        ofs << "[node] [level " << indent << "]: " << node->mName.C_Str() << "\n";

        u32 cNum = node->mNumChildren;
        u32 mNum = node->mNumMeshes;

        for (u32 i = 0u; i < mNum; i++)
        {
            for (auto j = 0u; j < indent + 1u; j++)
            {
                ofs << "--";
            }
            auto m = node->mMeshes[i];

            ofs << "[mesh] [index " << node->mMeshes[i] << "]: " << scene->mMeshes[node->mMeshes[i]]->mName.C_Str() << "\n";
        }

        for (u32 i = 0u; i < cNum; i++)
        {
            auto c = node->mChildren[i];
            debug_scene_process_node(ofs, scene, c, indent + 1u);
        }
    }

    void debug_scene_process_mesh(std::ofstream &ofs, aiNode *node, u32 indent)
    {
    }

    void debug_scene_graph_export(std::string_view assetPath, std::string_view outputPath)
    {
        std::ofstream file(outputPath.data());
        if (file.bad())
            return;

        u32 indent = 0u;
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(assetPath.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            INFO_LOG("ERROR::ASSIMP::{}", import.GetErrorString());
            return;
        }

        file << "Asset: [" << scene->mName.C_Str() << "]:[" << assetPath << "]" << "\n";

        debug_scene_process_node(file, scene, scene->mRootNode, indent + 1u);

        file.close();
    }

    void load_gltf(renderer::vulkan::VulkanCore *vkCore, std::string_view path, h64 hash, SceneResourceBank &sceneResource, MeshResourceBank &meshResource)
    {
        ProcessFunction nodeProcessCallback = [&](ProcessType pType, ProcessData pData) -> ProcessResult
        {
            ProcessResult res;
            switch (pType)
            {
            case ProcessType::Empty:
                // INFO_PRINT("Processing Empty Node");
                {
                    assert(pData.sceneTree);
                    assert(SceneTree::mIndexInvalid != pData.parentIndex);
                    auto nodePair = pData.sceneTree->CreateNode(SceneNodeType::Empty, pData.parentIndex);
                    auto nodePtr = std::dynamic_pointer_cast<SceneEmptyNode>(nodePair.second);
                    assert(nodePtr);
                    nodePtr->transform = pData.transform;
                    res.createIndex = nodePair.first;
                }
                break;
            case ProcessType::Mesh:
                // INFO_PRINT("Processing Mesh Node");
                {
                    assert(pData.vertices);
                    assert(pData.indices);
                    auto mr = meshResource.GetMeshResource(pData.meshHash);
                    assert(mr);
                    // todo optimize
                    mr->vertices = *pData.vertices;
                    mr->indices = *pData.indices;
                    // todo
                    mr->buffers = vkCore->uploadMesh(mr->indices, mr->vertices);
                    mr->uploaded = true;
                }
                break;
            case ProcessType::QueryMesh:
            {
                assert(pData.sceneTree);
                assert(SceneTree::mIndexInvalid != pData.parentIndex);
                auto mr = meshResource.GetMeshResource(pData.meshHash);
                if (mr != nullptr)
                    res.meshRegistered = true;
                else
                {
                    res.meshRegistered = false;
                    mr = meshResource.AcquireMeshResource(pData.meshHash);
                    assert(mr);
                }
                auto nodePair = pData.sceneTree->CreateNode(SceneNodeType::Mesh, pData.parentIndex);
                auto nodePtr = std::dynamic_pointer_cast<SceneMeshNode>(nodePair.second);
                assert(nodePtr);
                nodePtr->data.ResourceHash = pData.meshHash;
                res.createIndex = nodePair.first;
            }
            break;
            default:
                break;
            }

            return res;
        };

        AssimpImportConfig config;
        config.path = path;
        config.debugOutput = false;
        config.nodeFunction = nodeProcessCallback;
        AssimpImporter importer;
        importer.Import(config);

        std::shared_ptr<SceneTree> sceneTree = sceneResource.AcquireSceneResource(hash);
        importer.LoadSceneTree(sceneTree);
    }

    void ResourceManager::DebugPrintAssetHierarchy(std::string_view assetPath)
    {
        std::string output = config::log_folder_path + "asset_manager_log.txt";
        debug_scene_graph_export(assetPath, output);
    }

    std::shared_ptr<ResourceManager> ResourceManager::Get()
    {
        static std::shared_ptr<ResourceManager> sInstance(new ResourceManager);
        return sInstance;
    }

    // std::shared_ptr<SceneData> ResourceManager::GetSceneData(h64 hash)
    // {
    //     return mSceneResource.GetSceneResource(hash);
    // }

    std::shared_ptr<SceneTree> ResourceManager::GetSceneTree(h64 hash)
    {
        return mSceneResource.GetSceneResource(hash);
    }

    std::shared_ptr<MeshData> ResourceManager::GetMeshData(h64 hash)
    {
        return mMeshResource.GetMeshResource(hash);
    }

    bool ResourceManager::AddResourceMetaData(std::string_view stringView, ResourceType type)
    {
        assert(!stringView.empty());
        assert(type != ResourceType::ENUM_MAX);
        if (auto p = stringView.rfind('.'); p != stringView.npos)
        {
            auto fileformat = stringView.substr(p, stringView.length());
            h64 h = math::HashString(stringView);
            if (auto it = mResourceMetaDataMap.find(h); it != mResourceMetaDataMap.end())
            {
                assert(false);
                return false;
            }

            ResourceMetaData meta;

            switch (type)
            {
            case ResourceType::Model:
            {
                if (fileformat == ".gltf" || fileformat == ".glb")
                {
                    meta.path = stringView;
                    meta.type = ResourceType::Model;
                    break;
                }
                assert(false);
                break;
            }
            default:
                assert(false);
                break;
            }

            mResourceMetaDataMap.insert({h, meta});
            return true;
        }

        assert(false);
        return false;
    }

    bool ResourceManager::LoadModelData(h64 hash)
    {
        if (auto it = mResourceMetaDataMap.find(hash); it != mResourceMetaDataMap.end())
        {
            auto meta = it->second;
            if (meta.type != ResourceType::Model)
                return false;

            load_gltf(mVkCore, meta.path, hash, mSceneResource, mMeshResource);
        }
        return false;
    }

    void ResourceManager::Initialize()
    {
        if(mbInitialized)
            return;

        mbInitialized = true;
    }

    std::shared_ptr<MeshData> MeshResourceBank::GetMeshResource(h64 hash)
    {
        if (mMeshResource.find(hash) != mMeshResource.end())
        {
            return mMeshResource[hash];
        }
        return nullptr;
    }

    std::shared_ptr<MeshData> MeshResourceBank::AcquireMeshResource(h64 hash)
    {
        if (auto it = mMeshResource.find(hash); it == mMeshResource.end())
        {
            auto m = std::make_shared<MeshData>();
            mMeshResource[hash] = m;
            return m;
        }
        else
        {
            return it->second;
        }
        // return nullptr;
    }

    std::shared_ptr<SceneTree> SceneResourceBank::GetSceneResource(h64 hash)
    {
        if (mSceneResource.find(hash) != mSceneResource.end())
        {
            return mSceneResource[hash];
        }
        return nullptr;
    }

    std::shared_ptr<SceneTree> SceneResourceBank::AcquireSceneResource(h64 hash)
    {
        if (mSceneResource.find(hash) == mSceneResource.end())
        {
            auto s = std::make_shared<SceneTree>();
            mSceneResource[hash] = s;
            return s;
        }
        return nullptr;
    }
}