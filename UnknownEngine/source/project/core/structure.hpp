#pragma once
#include <vector>
#include <unordered_map>
#include <cassert>
#include <unordered_set>
#include <platform/type.hpp>
#include <queue>
#include <functional>

namespace unknown::structure
{
    template <typename N>
    struct Node
    {
        Node(N n) : content(n) {}
        N content;

        u32 inDegree = 0u;
        u32 outDegree = 0u;
    };

    template <typename Ktype>
    struct Edge
    {
        Edge(Ktype _from, Ktype _to) : from(_from), to(_to) {}
        bool operator==(const Edge &other) const
        {
            return (this->from == other.from && this->to == other.to);
        }

        Ktype from;
        Ktype to;
    };

    template <typename Ktype>
    struct EdgeHashFunction
    {
        u64 operator()(const Edge<Ktype> &edge) const
        {
            u64 fromHash = std::hash<Ktype>()(edge.from);
            u64 toHash = std::hash<Ktype>()(edge.to) << 1;
            return fromHash ^ toHash;
        }
    };

    // template <typename K, typename N>
    // class NodeTree
    // {
    // public:
    //     NodeTree(){};

    //     bool AddRoot(K key, N content)
    //     {
    //         if (nodes.find(key) != nodes.end())
    //             return false;

    //         nodes.insert({key, Node<N>(content)});
    //         parent.insert({key, key});
    //         roots.push_back(key);

    //         return true;
    //     }

    //     bool AddChild(K pKey, K cKey, N content)
    //     {
    //         if (nodes.find(pKey) == nodes.end() || nodes.find(cKey) != nodes.end())
    //             return false;

    //         nodes.insert({cKey, Node<N>(content)});
    //         parent.insert({cKey, pKey});
    //         if (childs.find(pKey) == childs.end())
    //             childs.insert({pKey, std::vector<K>{cKey}});
    //         else
    //             childs[pKey].push_back(cKey);

    //         return true;
    //     }

    //     std::vector<K> GetRoots() { return roots; }

    //     std::vector<K> GetChilds(K pKey)
    //     {
    //         if (auto it = childs.find(pKey); it != childs.end())
    //             return it->second;

    //         return std::vector<K>{};
    //     }

    //     K GetParent(K cKey)
    //     {
    //         if (auto it = parent.find(cKey); it != parent.end())
    //             return *it;

    //         return K();
    //     }

    //     const N &GetContent(K key)
    //     {
    //         if (auto n = nodes.find(key); n != nodes.end())
    //             return n->second.content;

    //         return nullptr;
    //     }

    // private:
    //     std::unordered_map<K, Node<N>> nodes;

    //     std::unordered_map<K, std::vector<K>> childs;
    //     std::unordered_map<K, K> parent;

    //     std::vector<K> roots;
    // };

    template <typename K, typename N>
    class NodeGraph
    {
    public:
        NodeGraph(){};
        void AddNode(K key, N content)
        {
            isDirty = true;
            Node<N> node(content);
            assert(nodes.find(key) == nodes.end());
            nodes.insert(std::pair<K, Node<N>>(key, node));
        }

        void AddEdge(K from, K to)
        {
            isDirty = true;
            assert(nodes.find(from) != nodes.end());
            assert(nodes.find(to) != nodes.end());
            auto edge = Edge<K>(from, to);
            assert(edges.find(edge) == edges.end());

            edges.insert(edge);
        }

        bool Build()
        {
            if (!isDirty)
                return true;

            childsRange.clear();
            parentsRange.clear();
            childsKey.clear();
            parentsKey.clear();
            roots.clear();
            ends.clear();
            singles.clear();
            topoSortResult.clear();
            topoSortResult.reserve(nodes.size());

            for (auto &n : nodes)
            {
                n.second.inDegree = 0u;
                n.second.outDegree = 0u;
            }

            for (auto &n : nodes)
            {
                K k = n.first;
                u32 childsRangeLeft = childsKey.size();
                u32 parentsRangeLeft = parentsKey.size();
                u32 cRange = 0u;
                u32 pRange = 0u;

                for (const auto &e : edges)
                {
                    if (k == e.from)
                    {
                        cRange++;
                        if (auto it = nodes.find(k); it != nodes.end())
                        {
                            it->second.outDegree += 1u;
                            childsKey.push_back(e.to);
                        }
                    }

                    if (k == e.to)
                    {
                        pRange++;
                        if (auto it = nodes.find(k); it != nodes.end())
                        {
                            it->second.inDegree += 1u;
                            parentsKey.push_back(e.from);
                        }
                    }
                }

                if (cRange == 0 && pRange == 0)
                    singles.push_back(k);
                else if (cRange == 0)
                    ends.push_back(k);
                else if (pRange == 0)
                    roots.push_back(k);

                childsRange[k] = std::pair<u32, u32>(childsRangeLeft, childsRangeLeft + cRange);
                parentsRange[k] = std::pair<u32, u32>(parentsRangeLeft, parentsRangeLeft + pRange);
            }

            return topologicSort();
        }

        std::vector<K> GetTopologicKeyOrder()
        {
            if (isDirty || !isAcyclic)
                return std::vector<K>();

            return topoSortResult;
        }

        Node<N> *GetNode(K key)
        {
            auto n = nodes.find(key);
            if (n == nodes.end())
                return nullptr;
            else
                return &(n->second);
        }

        std::vector<N> GetTopologicContentOrder()
        {
            std::vector<N> content;

            if (!isDirty && isAcyclic)
            {
                content.reserve(topoSortResult.size());
                for (auto k : topoSortResult)
                {
                    content.push_back(nodes.find(k)->second.content);
                }
            }
            return content;
        }

        void RecursiveOperationDownward(const std::function<void(N, N)> &op)
        {
            for (auto r : roots)
            {
                recursiveOperationDownward(op, r);
            }
        }

    private:
        void recursiveOperationDownward(const std::function<void(N, N)> &op, K upper)
        {
            auto [left, right] = childsRange[upper];

            for (u32 i = left; i < right; i++)
            {
                K ck = childsKey[i];
                assert(nodes.find(upper)!=nodes.end());
                assert(nodes.find(ck)!=nodes.end());
                op(nodes.find(upper)->second.content, nodes.find(ck)->second.content);
                recursiveOperationDownward(op, ck);
            }
        }

        bool topologicSort()
        {
            u32 count = 0u;
            std::queue<K> q;

            for (K key : roots)
            {
                q.push(key);
            }

            for (K key : singles)
            {
                count++;
                topoSortResult.push_back(key);
            }

            while (!q.empty())
            {
                const K &k = q.front();
                count++;
                std::pair<u32, u32> range = childsRange[k];
                for (u32 i = range.first; i < range.second; i++)
                {
                    K ck = childsKey[i];
                    if (auto it = nodes.find(ck); it != nodes.end())
                    {
                        Node<N> &n = it->second;
                        n.inDegree--;
                        if (n.inDegree == 0u)
                            q.push(ck);
                    }
                }
                topoSortResult.push_back(k);
                q.pop();
            }

            isDirty = false;
            if (count == nodes.size())
            {
                isAcyclic = true;
                return true;
            }
            else
            {
                isAcyclic = false;
                return false;
            }
        }

        bool isAcyclic = false;
        bool isDirty = true;

        std::unordered_set<Edge<K>, EdgeHashFunction<K>> edges;
        std::unordered_map<K, Node<N>> nodes;

        std::unordered_map<K, std::pair<u32, u32>> childsRange;
        std::unordered_map<K, std::pair<u32, u32>> parentsRange;

        std::vector<K> childsKey;
        std::vector<K> parentsKey;

        std::vector<K> roots;
        std::vector<K> ends;
        std::vector<K> singles;

        std::vector<K> topoSortResult;
    };
}