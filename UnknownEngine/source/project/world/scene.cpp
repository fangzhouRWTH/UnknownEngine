#include "scene.hpp"

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
}