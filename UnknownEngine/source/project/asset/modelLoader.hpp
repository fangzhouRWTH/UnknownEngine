#pragma once
#include <string>
#include <vector>
#include "core/math.hpp"
#include "platform/type.hpp"
// temp vulkan
#include "renderer/vulkan/vkTypes.hpp"
// temp vulkan
#include <unordered_set>
#include "core/structure.hpp"
#include "world/scene.hpp"

#include "assets.hpp"

namespace unknown::asset
{
    class AssetsManager;
    enum class TexType
    {
        Diffuse,
        Specular,
        Normal,
        Roughtness,
        AO,

        ENUM_MAX,
    };

    struct Tex
    {
        TexType type;
        std::string path;
    };

    struct SceneRawData
    {
        struct MeshHashIndex
        {
            u64 hash;
            u32 rangeIndex;
        };

        SceneRawData() { clear(); }
        std::string absDirectory;
        std::string name;

        // todo use only one type
        // std::vector<Vertex> vertices;
        std::vector<VkVertex> vertices;
        std::vector<u32> indices;
        std::vector<Tex> textures;
        std::vector<u32> parent;
        std::vector<std::pair<u32, u32>> subMeshVerticesRanges;
        std::vector<std::pair<u32, u32>> subMeshIndicesRanges;

        std::unordered_map<std::string, MeshHashIndex> meshNamesHash; // mesh name / hash
        // structure::NodeTree<std::string,SceneRawNode> sceneRawTree;
        //structure::NodeTree<std::string, SceneNode> sceneObjectTree;

        void clear()
        {
            absDirectory.clear();
            name.clear();
            vertices.clear();
            indices.clear();
            textures.clear();
            parent.clear();
            subMeshVerticesRanges.clear();
            subMeshIndicesRanges.clear();
            meshNamesHash.clear();
        }
    };

    struct ModelDescription
    {
        std::string absDirectory;
        std::string name;

        u32 verticesCount;
        u32 indicesCount;

        // std::vector<std::string> diffuseTexturePaths;
        // std::vector<std::string> specularTexturePaths;
        // std::vector<std::string> normalTexturePaths;
        // std::vector<std::string> roughnessTexurePaths;
        // std::vector<std::string> aoTexturePaths;

        std::unordered_set<std::string> diffuseTexturePaths;
        std::unordered_set<std::string> specularTexturePaths;
        std::unordered_set<std::string> normalTexturePaths;
        std::unordered_set<std::string> roughnessTexurePaths;
        std::unordered_set<std::string> aoTexturePaths;
    };

    class GLTFLoader
    {
    public:
        static bool Load(std::string path, std::shared_ptr<AssetsManager> manager);
    };

    class ModelLoader
    {
    public:
        static void GetModelDescription(std::string path, ModelDescription &description);
        // static void Load(std::string path, MeshRawData &raw);
        //static void LoadGLTF(std::string path, SceneRawData &raw);

        //static void LoadMesh(std::string path, SceneRawData &raw);
        // static SceneObjectsTree Load
    };
}