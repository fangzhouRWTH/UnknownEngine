#include "scene.hpp"
//#include <nlohmann/json.hpp>
#include "serialization/jsonSerializer.hpp"
#include "asset/asset.hpp"
#include "core/hash.hpp"

namespace unknown
{
    const SceneNodeIndex SceneTree::mIndexInvalid = structure::SimpleGraph<512u, 512u>::mIndexInvalid;

    SceneTree::SceneTree()
    {
        std::shared_ptr<ISceneNode> node = std::make_shared<SceneEmptyNode>();
        mNodes.insert({mRootIndex, node});
        mGraph.AddNode();
    }

    std::pair<SceneNodeIndex, std::shared_ptr<ISceneNode>> SceneTree::CreateNode(SceneNodeType type, SceneNodeIndex parent)
    {
        SceneNodeIndex idx = mGraph.AddChild(parent);
        if (idx == mGraph.mIndexInvalid)
            return std::pair<SceneNodeIndex, std::shared_ptr<ISceneNode>>(mIndexInvalid,nullptr);

        std::shared_ptr<ISceneNode> node;
        switch (type)
        {
        case SceneNodeType::Empty:
            node = std::make_shared<SceneEmptyNode>();
            break;
        case SceneNodeType::Mesh:
            node = std::make_shared<SceneMeshNode>();
            break;
        default:
            assert(false);
        }

        mNodes.insert({idx, node});
        return {idx, node};
    }

    std::pair<SceneNodeIndex, std::shared_ptr<ISceneNode>> SceneTree::CreateAttachRoot(SceneNodeType type)
    {
        return CreateNode(type,mRootIndex);
    }

    std::shared_ptr<ISceneNode> SceneTree::GetNode(SceneNodeIndex index)
    {
        auto nIt = mNodes.find(index);
        return nIt->second;
    }

    bool SceneTree::RemoveNode(SceneNodeIndex index)
    {
        if(index == mRootIndex||!mGraph.HasNode(index))
            return false;

        std::vector<structure::NodeIndex> parents;
        std::vector<structure::NodeIndex> childs;
        mGraph.GetParents(index,parents);
        assert(parents.size()==1);
        mGraph.GetChilds(index,childs);

        mGraph.RemoveNode(index);

        structure::NodeIndex p = parents[0];

        for(auto c : childs)
        {
            mGraph.Link(p,c);
        }

        mNodes.erase(index);
        return true;
    }

    bool SceneTree::HasNode(SceneNodeIndex index)
    {
        return mGraph.HasNode(index);
    }

    bool SceneTree::GetChilds(SceneNodeIndex index, std::vector<SceneNodeIndex> &childs)
    {
        return mGraph.GetChilds(index, childs);
    }

    SceneNodeIndex SceneTree::GetParent(SceneNodeIndex index)
    {
        std::vector<structure::NodeIndex> parents;
        index = mIndexInvalid;

        if(!mGraph.GetParents(index, parents))
            return false;

        return parents[0];
    }

    void SceneLoader::LoadScene(const SceneLoaderContext & context)
    {
        Serializer json;

        if(!context.managerPtr||!json.Load(context.sceneFilePath))
            return;

        auto scene = json.Access("scene");

        if(!scene.isEmpty())
        {
            auto assets = scene.Access("assets");
            auto asize = assets.Size();
            for(u64 i = 0; i < asize; i++)
            {
                auto item = assets.Access(i);
                auto optPath = item.Access("path").Value<std::string>();
                if(optPath.has_value())
                {
                    context.managerPtr->AddAssetMetaData(optPath.value(), asset::AssetType::Model);
                    h64 h = math::HashString(optPath.value());
                    
                    bool load = context.managerPtr->LoadModelData(h);
                }
            }
        }
    }
}