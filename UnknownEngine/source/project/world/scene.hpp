#pragma once
//#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "core/math.hpp"
#include "core/structure.hpp"
#include "core/structure/graph.hpp"
//#include "renderer/rendererHandles.hpp"

namespace unknown
{
    // struct SceneObjectsTree
    // {
    // public:
    //     structure::NodeTree<std::string, SceneNode> sceneTree;

    // private:
    // };
    enum class SceneNodeType
    {
        Empty,
        Mesh,

        ENUM_MAX,
    };

    struct ISceneNode
    {
        virtual ~ISceneNode() {}
        ISceneNode(SceneNodeType t) : type(t) {}
        const SceneNodeType type;
        Mat4f transform = Mat4f::Identity();
    };

    struct SceneEmptyNode final : public ISceneNode
    {
        virtual ~SceneEmptyNode() override {};
        SceneEmptyNode() : ISceneNode(SceneNodeType::Empty) {}
    };

    struct MeshNodeData
    {
        h64 ResourceHash;
        h64 GPUResourceHash;
    };

    struct SceneMeshNode final : public ISceneNode
    {
        virtual ~SceneMeshNode() override {};
        SceneMeshNode() : ISceneNode(SceneNodeType::Mesh) {}
        MeshNodeData data;
    };

    typedef structure::NodeIndex SceneNodeIndex;
    class SceneTree
    {
        // TODO CONSTRUCTORS
    public:
        SceneTree();
        std::pair<SceneNodeIndex, std::shared_ptr<ISceneNode>> CreateNode(SceneNodeType type, SceneNodeIndex parent);
        std::pair<SceneNodeIndex, std::shared_ptr<ISceneNode>> CreateAttachRoot(SceneNodeType type);
        std::shared_ptr<ISceneNode> GetNode(SceneNodeIndex index);
        bool RemoveNode(SceneNodeIndex index);
        bool HasNode(SceneNodeIndex index);
        bool GetChilds(SceneNodeIndex index, std::vector<SceneNodeIndex> & childs);
        SceneNodeIndex RootIndex() {return mRootIndex;}
        SceneNodeIndex GetParent(SceneNodeIndex index);

        const static SceneNodeIndex mIndexInvalid;
    private:
        std::unordered_map<SceneNodeIndex, std::shared_ptr<ISceneNode>> mNodes;
        structure::SimpleGraph<512u, 4096u> mGraph;
        const SceneNodeIndex mRootIndex = 0u;
    };

    class SceneLoader
    {
    public:
        static void LoadScene(std::string sceneFilePath)
        {
            // std::ifstream f(sceneFilePath);
            // json data = json::parse(f);

            // auto scene = data["scene"];
            // auto entities = scene.find("entities");
            // if (entities != scene.end())
            // {
            //     for (auto it = entities->begin(); it != entities->end(); it++)
            //     {
            //         auto name = it->find("name");
            //         if (name != it->end())
            //         {
            //             std::cout << name.value() << std::endl;
            //         }
            //     }
            // }
        }
    };
}