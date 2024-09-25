#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "platform/type.hpp"
#include "world/scene.hpp"
#include "core/base.hpp"

#include <string>
#include <memory>
#include <functional>

namespace unknown::asset
{
    enum class ProcessType
    {
        Empty,
        Mesh,

        QueryMesh,
    };

    struct ProcessData
    {
        std::shared_ptr<SceneTree> sceneTree = nullptr;
        SceneNodeIndex parentIndex = SceneTree::mIndexInvalid;
        Mat4f transform = Mat4f::Identity();
        h64 meshHash;
        std::shared_ptr<std::vector<Vertex>> vertices = nullptr;
        std::shared_ptr<std::vector<u32>> indices = nullptr;
    };

    struct ProcessResult
    {
        ProcessType type;
        bool meshRegistered = false;
        SceneNodeIndex createIndex = SceneTree::mIndexInvalid;
    };

    //typedef void (*ProcessFunction)(ProcessType,ProcessData);
    typedef std::function<ProcessResult (ProcessType,ProcessData)> ProcessFunction;

    struct AssimpImportConfig
    {
        std::string path;
        //TODO wrap
        u32 postProcessOptions = aiPostProcessSteps::aiProcess_Triangulate | aiPostProcessSteps::aiProcess_FlipUVs;
        ProcessFunction nodeFunction;
        bool debugOutput = false;
    };

    struct AssimpImportData
    {
        const aiScene* scene = nullptr;
        u32 meshCount = 0u;
        aiMesh ** meshPP = nullptr;

        void reset()
        {
            scene = nullptr;
            meshCount = 0u;
            meshPP = nullptr;
        }
    };

    class AssimpImporter
    {
    public:
        AssimpImporter() {};
        bool Import(const AssimpImportConfig &config);
        bool LoadSceneTree(std::shared_ptr<SceneTree> sceneTree);

    private:
        void reset();

        AssimpImportConfig mConfig;
        Assimp::Importer mImporter;

        AssimpImportData mData;
    };
}