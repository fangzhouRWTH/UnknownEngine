#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "debug/log.hpp"
#include "configuration/globalValues.hpp"
#include "world/scene.hpp"
#include "core/base.hpp"
#include "core/hash.hpp"

#include "asset/utils/assimpHelper.hpp"

#include <iostream>
#include <fstream>

namespace unknown::asset
{
    static std::string get_mesh_name(aiMesh *mesh, h64 hash)
    {
        return std::string(mesh->mName.C_Str());
    }

    static h64 get_mesh_hash(std::string_view assetPath, u32 meshIndex, std::string_view meshName)
    {
        std::stringstream ss;
        ss << assetPath << meshIndex << meshName;
        const auto str = ss.str();
        return math::HashString(str);
    }

    static void debug_scene_process_node(std::ofstream &ofs, const aiScene *scene, aiNode *node, u32 indent)
    {
        for (auto i = 0u; i < indent; i++)
        {
            ofs << "--";
        }
        ofs << "[node] [level " << indent << "]: " << node->mName.C_Str() << "\n";

        auto trans = node->mTransformation;

        for (auto i = 0u; i < indent + 1u; i++)
        {
            ofs << "--";
        }
        ofs << "==Transform== " << " [" << trans.a1 << "] " << " [" << trans.a2 << "] " << " [" << trans.a3 << "] " << " [" << trans.a4 << "] " << "\n";

        for (auto i = 0u; i < indent + 1u; i++)
        {
            ofs << "--";
        }
        ofs << "============= " << " [" << trans.b1 << "] " << " [" << trans.b2 << "] " << " [" << trans.b3 << "] " << " [" << trans.b4 << "] " << "\n";

        for (auto i = 0u; i < indent + 1u; i++)
        {
            ofs << "--";
        }
        ofs << "============= " << " [" << trans.c1 << "] " << " [" << trans.c2 << "] " << " [" << trans.c3 << "] " << " [" << trans.c4 << "] " << "\n";

        for (auto i = 0u; i < indent + 1u; i++)
        {
            ofs << "--";
        }
        ofs << "============= " << " [" << trans.d1 << "] " << " [" << trans.d2 << "] " << " [" << trans.d3 << "] " << " [" << trans.d4 << "] " << "\n";

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

    static void debug_scene_process_mesh(std::ofstream &ofs, aiNode *node, u32 indent)
    {
    }

    static void debug_scene_graph_export(std::string_view assetPath, std::string_view outputPath)
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

    static void debug_print_asset_hierarchy(std::string_view assetPath)
    {
        std::string output = config::log_folder_path + "asset_manager_log.txt";
        debug_scene_graph_export(assetPath, output);
    }

    static void debug_print_texture_info(const aiTexture * tex)
    {
        INFO_LOG(
            "[texture file path] {}",tex->mFilename.C_Str()
        )
    }

    static h64 load_mesh(const aiScene *scene, u32 meshIndex, std::shared_ptr<SceneTree> sceneTree, SceneNodeIndex parentIndex, const AssimpImportConfig &config)
    {
        const auto mesh = scene->mMeshes[meshIndex];
        const u32 numVertices = mesh->mNumVertices;
        ProcessData data;
        data.meshHash = get_mesh_hash(config.path, meshIndex, mesh->mName.C_Str());
        data.sceneTree = sceneTree;
        data.parentIndex = parentIndex;
        ProcessResult res = config.nodeFunction(ProcessType::QueryMesh, data);
        if (res.meshRegistered)
            return data.meshHash;
        std::shared_ptr<std::vector<Vertex>> vertices = std::make_shared<std::vector<Vertex>>();
        vertices->reserve(numVertices);
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
                v.uv_x = inputT.x;
                v.uv_y = inputT.y;
            }
            else
            {
                v.uv_x = 0.f;
                ;
                v.uv_y = 0.f;
            }

            vertices->push_back(v);
        }

        const u32 numFaces = mesh->mNumFaces;
        std::shared_ptr<std::vector<u32>> indices = std::make_shared<std::vector<u32>>();
        indices->reserve(numFaces);

        for (u32 i = 0; i < numFaces; i++)
        {
            const aiFace &inputF = mesh->mFaces[i];
            const auto numIdx = inputF.mNumIndices;
            for (u32 j = 0u; j < numIdx; j++)
            {
                indices->push_back(inputF.mIndices[j]);
            }
        }

        data.vertices = vertices;
        data.indices = indices;

        config.nodeFunction(ProcessType::Mesh, data);

        return data.meshHash;
    }

    static void load_node(const aiScene *scene, aiNode *node, std::shared_ptr<SceneTree> sceneTree, SceneNodeIndex parentIndex, const AssimpImportConfig &config)
    {
        u32 cNum = node->mNumChildren;
        u32 mNum = node->mNumMeshes;
        auto transform = node->mTransformation;
        Mat4f mat;
        mat << transform.a1, transform.a2, transform.a3, transform.a4,
            transform.b1, transform.b2, transform.b3, transform.b4,
            transform.c1, transform.c2, transform.c3, transform.c4,
            transform.d1, transform.d2, transform.d3, transform.d4;

        ProcessData data;
        data.transform = mat;
        data.sceneTree = sceneTree;
        data.parentIndex = parentIndex;
        auto res = config.nodeFunction(ProcessType::Empty, data);
        assert(res.createIndex!=SceneTree::mIndexInvalid);

        for (u32 i = 0u; i < mNum; i++)
        {
            auto m = node->mMeshes[i];
            h64 h = load_mesh(scene, m, sceneTree, res.createIndex, config);
        }

        for (u32 i = 0u; i < cNum; i++)
        {
            auto c = node->mChildren[i];
            load_node(scene, c, sceneTree, res.createIndex, config);
        }
    }
}