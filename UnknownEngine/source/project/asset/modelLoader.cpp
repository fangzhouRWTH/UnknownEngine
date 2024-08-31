#include "modelLoader.hpp"
#include "assetManager.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "platform/type.hpp"

#include <functional>

// #include "mesh.hpp"

// #include "debug/log.hpp"

namespace unknown::asset
{
    struct IntermediateData
    {
    };

    void extract_material_textures_description(aiMaterial *mat, aiTextureType type, const std::string &typeName, ModelDescription &description)
    {
        const u32 tCount = mat->GetTextureCount(type);
        for (u32 i = 0u; i < tCount; i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            switch (type)
            {
            case aiTextureType::aiTextureType_DIFFUSE:
                description.diffuseTexturePaths.insert(description.absDirectory + "/" + str.C_Str());
                break;
            case aiTextureType::aiTextureType_SPECULAR:
                description.specularTexturePaths.insert(description.absDirectory + "/" + str.C_Str());
                break;
            case aiTextureType::aiTextureType_NORMALS:
                description.normalTexturePaths.insert(description.absDirectory + "/" + str.C_Str());
                break;
            case aiTextureType::aiTextureType_METALNESS:
                description.roughnessTexurePaths.insert(description.absDirectory + "/" + str.C_Str());
                break;
            case aiTextureType::aiTextureType_AMBIENT_OCCLUSION:
                description.aoTexturePaths.insert(description.absDirectory + "/" + str.C_Str());
                break;
            default:
                break;
            }
        }
    }

    // void process_mesh(aiMesh *mesh, const aiScene *scene, SceneRawData &res, IntermediateData &interData)
    // {
    //     std::string meshName = std::string(mesh->mName.C_Str());
    //     if (auto it = res.meshNamesHash.find(meshName); it != res.meshNamesHash.end())
    //         return;

    //     std::string hString = res.absDirectory + meshName;

    //     std::hash<std::string> strhash;
    //     u64 h = strhash(std::string(hString));
    //     res.meshNamesHash.insert({meshName, SceneRawData::MeshHashIndex{h, u32(res.subMeshVerticesRanges.size())}});

    //     const u32 numVertices = mesh->mNumVertices;
    //     const u32 offset = res.vertices.size();
    //     res.subMeshVerticesRanges.push_back({offset, offset + numVertices});
    //     // res.vertices.resize(numVertices);
    //     for (u32 i = 0u; i < numVertices; i++)
    //     {
    //         const auto &inputV = mesh->mVertices[i];
    //         // auto &v = res.vertices[i];
    //         VkVertex v;
    //         v.position.x() = inputV.x;
    //         v.position.y() = inputV.y;
    //         v.position.z() = inputV.z;

    //         const auto &inputN = mesh->mNormals[i];
    //         v.normal.x() = inputN.x;
    //         v.normal.y() = inputN.y;
    //         v.normal.z() = inputN.z;

    //         // debug
    //         {
    //             v.color.segment(0, 3) = v.normal;
    //         }

    //         if (mesh->mTextureCoords[0])
    //         {
    //             const auto &inputT = mesh->mTextureCoords[0][i];
    //             // v.texCoords.x() = inputT.x;
    //             // v.texCoords.y() = inputT.y;
    //             v.uv_x = inputT.x;
    //             v.uv_y = inputT.y;
    //         }
    //         else
    //         {
    //             v.uv_x = 0.f;
    //             ;
    //             v.uv_y = 0.f;
    //             // v.texCoords = Vec2f(0, 0);
    //         }

    //         res.vertices.push_back(v);
    //     }

    //     const u32 numFaces = mesh->mNumFaces;
    //     // res.indices.resize(numIndice);
    //     u32 indicesOffset = res.indices.size();
    //     u32 count = 0u;

    //     for (u32 i = 0; i < numFaces; i++)
    //     {
    //         const aiFace &inputF = mesh->mFaces[i];
    //         const auto numIdx = inputF.mNumIndices;
    //         for (u32 j = 0u; j < numIdx; j++)
    //         {
    //             res.indices.push_back(inputF.mIndices[j] + offset);
    //             count++;
    //         }
    //     }

    //     res.subMeshIndicesRanges.push_back({indicesOffset, indicesOffset + count});

    //     // if (mesh->mMaterialIndex >= 0)
    //     // {
    //     //     aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    //     //     load_material_textures(material, aiTextureType_SPECULAR, "texture_specular", res);
    //     // }
    // };

    // void process_node(aiNode *node, const aiScene *scene, MeshRawData &res)
    // {
    //     u32 numMeshes = node->mNumMeshes;
    //     auto n = std::string(node->mName.C_Str());
    //     // INFO_PRINT(n);
    //     for (u32 i = 0; i < numMeshes; i++)
    //     {
    //         aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    //         INFO_PRINT(mesh->mName.C_Str());
    //         process_mesh(mesh, scene, res);
    //     }

    //     u32 numChildren = node->mNumChildren;
    //     for (u32 i = 0; i < numChildren; i++)
    //     {
    //         process_node(node->mChildren[i], scene, res);
    //     }
    // }

    // void process_scene_node(aiNode *node, const aiScene *scene, SceneRawData &res, std::string parent, IntermediateData &interData)
    // {
    //     // SceneRawData::SceneRawNode sNode;
    //     if (!node)
    //         return;

    //     SceneNode sNode;
    //     u32 numMeshes = node->mNumMeshes;
    //     auto aT = node->mTransformation;

    //     Mat4f trans;
    //     trans << aT.a1, aT.a2, aT.a3, aT.a4,
    //         aT.b1, aT.b2, aT.b3, aT.b4,
    //         aT.c1, aT.c2, aT.c3, aT.c4,
    //         aT.d1, aT.d2, aT.d3, aT.d4;

    //     sNode.transform = trans;
    //     sNode.nodeName = std::string(node->mName.C_Str());

    //     for (u32 i = 0; i < numMeshes; i++)
    //     {
    //         aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    //         std::string meshName = mesh->mName.C_Str();
    //         sNode.meshNames[i] = meshName;

    //         process_mesh(mesh, scene, res, interData);
    //     }

    //     res.sceneObjectTree.sceneTree.AddChild

    //         if (parent.empty())
    //             res.sceneRawTree.AddRoot(sNode.name, sNode);
    //     else res.sceneRawTree.AddChild(parent, sNode.name, sNode);

    //     u32 numChildren = node->mNumChildren;
    //     for (u32 i = 0; i < numChildren; i++)
    //     {
    //         process_scene_node(node->mChildren[i], scene, res, sNode.name);
    //     }
    // }

    void extract_mesh_description(aiMesh *mesh, const aiScene *scene, ModelDescription &description)
    {
        description.verticesCount += mesh->mNumVertices;

        const u32 numFaces = mesh->mNumFaces;
        for (u32 i = 0; i < numFaces; i++)
        {
            description.indicesCount += mesh->mFaces[i].mNumIndices;
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            // for(i32 i = 1; i <18; i++)
            // {
            //     auto c = material->GetTextureCount(aiTextureType(i));
            //     auto b = 0;
            // }
            extract_material_textures_description(material, aiTextureType_DIFFUSE, "texture_diffuse", description);
            extract_material_textures_description(material, aiTextureType_SPECULAR, "texture_specular", description);
            extract_material_textures_description(material, aiTextureType_NORMALS, "texture_normals", description);
            extract_material_textures_description(material, aiTextureType_METALNESS, "texture_roughtness", description);
            extract_material_textures_description(material, aiTextureType_AMBIENT_OCCLUSION, "texture_ao", description);
        }
    }

    void extract_node_description(aiNode *node, const aiScene *scene, ModelDescription &description)
    {
        u32 numMeshes = node->mNumMeshes;
        for (u32 i = 0; i < numMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            extract_mesh_description(mesh, scene, description);
        }

        u32 numChildren = node->mNumChildren;
        for (u32 i = 0; i < numChildren; i++)
        {
            extract_node_description(node->mChildren[i], scene, description);
        }
    }

    // void load_raw(std::string path, SceneRawData &raw)
    // {
    //     IntermediateData interData;

    //     Assimp::Importer import;
    //     const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    //     if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    //     {
    //         INFO_LOG("ERROR::ASSIMP::{}", import.GetErrorString());
    //         return;
    //     }
    //     raw.absDirectory = path.substr(0, path.find_last_of('/'));

    //     auto root = scene->mRootNode;
    //     if (root != nullptr)
    //     {
    //         process_scene_node(scene->mRootNode, scene, raw, std::string(), interData);
    //     }
    // }

    // void load_gltf(std::string path, std::shared_ptr<AssetsManager> manager)
    // {
    //     Assimp::Importer import;
    //     const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    //     if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    //     {
    //         INFO_LOG("ERROR::ASSIMP::{}", import.GetErrorString());
    //         return;
    //     }
    //     raw.absDirectory = path.substr(0, path.find_last_of('/'));

    //     auto root = scene->mRootNode;
    //     if (root != nullptr)
    //     {
    //         process_scene_node(scene->mRootNode, scene, raw, std::string(), interData);
    //     }
    // }

    void ModelLoader::GetModelDescription(std::string path, ModelDescription &description)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            // INFO_LOG("ERROR::ASSIMP::{}", import.GetErrorString());
            return;
        }
        description.absDirectory = path.substr(0, path.find_last_of('/'));
        extract_node_description(scene->mRootNode, scene, description);
    }

    // // void ModelLoader::Load(std::string path, MeshRawData &raw)
    // // {
    // //     load(path, raw);
    // // }

    // void ModelLoader::LoadGLTF(std::string path, SceneRawData &raw)
    // {
    //     raw.clear();
    //     load_raw(path, raw);
    // }

    // void ModelLoader::LoadMesh(std::string path, SceneRawData &raw)
    // {
    //     raw.clear();
    //     load_raw(path, raw);
    // }

    // bool GLTFLoader::Load(std::string path, std::shared_ptr<AssetsManager> manager)
    // {
    //     auto i = path.rfind('.');
    //     if (i == path.npos)
    //     {
    //         INFO_PRINT("failed to load unsupported asset type!");
    //         return false;
    //     }

    //     auto filetype = path.substr(i, path.length());
    //     if (filetype != ".gltf" || filetype != ".glb")
    //     {
    //         INFO_PRINT("failed to load unsupported asset type!");
    //         return false;
    //     }
    // }
}