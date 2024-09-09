#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <cassert>
#include <unordered_set>
#include <queue>
#include <functional>

#include "platform/type.hpp"

namespace unknown::structure
{
    typedef u32 NodeIndex;
    typedef u32 NodeCap;
    typedef u32 LinkCap;

    template <NodeCap nCap /*max nodes*/, LinkCap lCap /*max links of a node (parents or childs)*/>
    class SimpleGraph
    {
    private:
        struct Node
        {
        };

        struct LinkList
        {
            u32 counter = 0u;
            std::array<NodeIndex, lCap> list;
        };

    public:
        static bool IsValid(NodeIndex idx)
        {
            return !(idx >= mIndexInvalid);
        }

        bool HasNode(NodeIndex idx) const
        {
            if (mNodes.find(idx) == mNodes.end())
                return false;
            return true;
        }

        void GetNodes(std::vector<NodeIndex> nodes)
        {
            nodes.reserve(mNodes.size());
            nodes.clear();
            for (auto i : mNodes)
            {
                nodes.push_back(i.first);
            }
        }

        NodeIndex AddNode()
        {
            if (mIndexCounter == mIndexInvalid)
                return mIndexInvalid;

            Node node;
            NodeIndex index = mIndexCounter++;
            mNodes.insert({index, node});

            addRoot(index);

            LinkList list;
            list.list.fill(mIndexInvalid);
            mChildrenMap.insert({index, list});
            mParentsMap.insert({index, list});

            setDirty();

            return index;
        }

        NodeIndex AddChild(NodeIndex parent)
        {
            if (!HasNode(parent))
                return mIndexInvalid;

            if (mChildrenMap.find(parent)->second.counter >= lCap)
                return mIndexInvalid;

            NodeIndex idx = AddNode();
            if (idx == mIndexInvalid)
                return idx;

            Link(parent, idx);

            setDirty();

            return idx;
        }

        NodeIndex AddParent(NodeIndex child)
        {
            if (!HasNode(child))
                return mIndexInvalid;

            if (mParentsMap.find(child)->second.counter >= lCap)
                return mIndexInvalid;

            NodeIndex idx = AddNode();
            if (idx == mIndexInvalid)
                return idx;

            Link(idx, child);

            removeRoot(child);

            addRoot(idx);

            setDirty();

            return idx;
        }

        bool RemoveNode(NodeIndex index)
        {
            auto nIt = mNodes.find(index);
            if (nIt == mNodes.end())
                return false;

            auto &pList = mParentsMap.find(index)->second;
            auto &cList = mChildrenMap.find(index)->second;

            for (u32 i = 0u; i < pList.counter; i++)
            {
                NodeIndex p = pList.list[i];
                auto &pcList = mChildrenMap.find(p)->second;
                for (u32 j = 0u; j < pcList.counter; j++)
                {
                    if (pcList.list[j] != index)
                        continue;

                    pcList.list[j] = pcList.list[--pcList.counter];
                    break;
                }
            }

            for (u32 i = 0u; i < cList.counter; i++)
            {
                NodeIndex c = cList.list[i];
                auto &cpList = mParentsMap.find(c)->second;
                for (u32 j = 0u; j < cpList.counter; j++)
                {
                    if (cpList.list[j] != index)
                        continue;

                    cpList.list[j] = cpList.list[--cpList.counter];
                    break;
                }

                if (cpList.counter == 0)
                    addRoot(c);
            }

            mNodes.erase(index);
            mParentsMap.erase(index);
            mChildrenMap.erase(index);
            removeRoot(index);

            setDirty();

            return true;
        }

        bool Link(NodeIndex parent, NodeIndex child)
        {
            auto pIt = mNodes.find(parent);
            auto cIt = mNodes.find(child);
            if (pIt == mNodes.end() || cIt == mNodes.end())
                return false;

            auto &pList = mParentsMap.find(child)->second;
            auto &cList = mChildrenMap.find(parent)->second;

            if (pList.counter >= lCap || cList.counter >= lCap)
                return false;

            pList.list[pList.counter++] = parent;
            cList.list[cList.counter++] = child;

            removeRoot(child);

            setDirty();

            return true;
        }

        bool GetChilds(NodeIndex nodeIndex, std::vector<NodeIndex> &childs) const
        {
            childs.clear();
            auto nIt = mNodes.find(nodeIndex);
            if (nIt == mNodes.end())
                return false;

            auto &childList = mChildrenMap.find(nodeIndex)->second;
            childs.resize(childList.counter);
            for (u32 i = 0u; i < childList.counter; i++)
            {
                childs[i] = childList.list[i];
            }
            return true;
        }

        bool GetParents(NodeIndex nodeIndex, std::vector<NodeIndex> &parents) const
        {
            parents.clear();
            auto nIt = mNodes.find(nodeIndex);
            if (nIt == mNodes.end())
                return false;

            auto &parentList = mParentsMap.find(nodeIndex)->second;
            parents.resize(parentList.counter);
            for (u32 i = 0u; i < parentList.counter; i++)
            {
                parents[i] = parentList.list[i];
            }
            return true;
        }

        void GetRoots(std::vector<NodeIndex> &roots) const
        {
            roots.clear();
            roots.reserve(mRoots.size());
            for (auto r : mRoots)
            {
                roots.push_back(r);
            }
        }

        bool IsMultiGraphs()
        {
            return mRoots.size() > 1;
        }

        bool IsDirty()
        {
            return mIsDirty;
        }

        void Check()
        {
            if (!mIsDirty)
                return;

            mIsAcyclic = checkAcyclic();

            setClean();
        }

        bool IsAcyclic()
        {
            Check();

            return mIsAcyclic;
        }

    private:
        inline bool setDirty()
        {
            mIsDirty = true;
        }

        inline void setClean()
        {
            mIsDirty = false;
        }

        bool _rec_acyclic(std::unordered_set<NodeIndex> visit, NodeIndex next)
        {
            if (visit.find(next) != visit.end())
                return false;

            visit.insert(next);
            std::vector<NodeIndex> childs;
            GetChilds(next, childs);
            for (auto c : childs)
            {
                if (!_rec_acyclic(visit, c))
                    return false;
            }
            return true;
        }

        bool checkAcyclic()
        {
            if (mRoots.size() == 0)
                return false;

            for (auto r : mRoots)
            {
                std::unordered_set<NodeIndex> visit;
                visit.insert(r);
                std::vector<NodeIndex> childs;
                GetChilds(r, childs);
                for (auto c : childs)
                {
                    if (!_rec_acyclic(visit, c))
                        return false;
                }
            }
            return true;
        }

        bool addRoot(NodeIndex idx)
        {
            if (!SimpleGraph<nCap, lCap>::IsValid(idx))
                return false;

            if (mRoots.find(idx) != mRoots.end())
                return false;

            mRoots.insert(idx);
            return true;
        }

        bool removeRoot(NodeIndex idx)
        {
            if (!SimpleGraph<nCap, lCap>::IsValid(idx))
                return false;

            if (mRoots.find(idx) == mRoots.end())
                return false;

            mRoots.erase(idx);
            return true;
        }

    public:
        const static NodeIndex mIndexInvalid;

    private:
        std::unordered_set<NodeIndex> mRoots;
        std::unordered_map<NodeIndex, Node> mNodes;
        std::unordered_map<NodeIndex, LinkList> mChildrenMap;
        std::unordered_map<NodeIndex, LinkList> mParentsMap;

        NodeIndex mIndexCounter = 0u;
        bool mIsDirty = true;
        bool mIsAcyclic = true;
    };

    template <NodeCap nCap, LinkCap lCap>
    const NodeIndex SimpleGraph<nCap, lCap>::mIndexInvalid = 0xFFFFFFFF;

}