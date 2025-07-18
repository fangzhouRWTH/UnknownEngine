#pragma once
#include <array>
#include <cassert>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "platform/type.hpp"

namespace unknown::structure {
typedef u32 NodeIndex;
typedef u32 EdgeIndex;
typedef u32 NodeCap;
typedef u32 LinkCap;

typedef u32 NodeHandle;
typedef u32 EdgeHandle;

template <NodeCap nCap /*max nodes*/,
          LinkCap lCap /*max links of a node (parents or childs)*/>
class SimpleGraph {
private:
  struct Node {};

  struct LinkList {
    u32 counter = 0u;
    std::array<NodeIndex, lCap> list;
  };

public:
  static bool IsValid(NodeIndex idx) { return !(idx >= mIndexInvalid); }

  bool HasNode(NodeIndex idx) const {
    if (mNodes.find(idx) == mNodes.end())
      return false;
    return true;
  }

  void GetNodes(std::vector<NodeIndex> nodes) {
    nodes.reserve(mNodes.size());
    nodes.clear();
    for (auto i : mNodes) {
      nodes.push_back(i.first);
    }
  }

  NodeIndex AddNode() {
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

  NodeIndex AddChild(NodeIndex parent) {
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

  NodeIndex AddParent(NodeIndex child) {
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

  bool RemoveNode(NodeIndex index) {
    auto nIt = mNodes.find(index);
    if (nIt == mNodes.end())
      return false;

    auto &pList = mParentsMap.find(index)->second;
    auto &cList = mChildrenMap.find(index)->second;

    for (u32 i = 0u; i < pList.counter; i++) {
      NodeIndex p = pList.list[i];
      auto &pcList = mChildrenMap.find(p)->second;
      for (u32 j = 0u; j < pcList.counter; j++) {
        if (pcList.list[j] != index)
          continue;

        pcList.list[j] = pcList.list[--pcList.counter];
        break;
      }
    }

    for (u32 i = 0u; i < cList.counter; i++) {
      NodeIndex c = cList.list[i];
      auto &cpList = mParentsMap.find(c)->second;
      for (u32 j = 0u; j < cpList.counter; j++) {
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

  bool Link(NodeIndex parent, NodeIndex child) {
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

  bool GetChilds(NodeIndex nodeIndex, std::vector<NodeIndex> &childs) const {
    childs.clear();
    auto nIt = mNodes.find(nodeIndex);
    if (nIt == mNodes.end())
      return false;

    auto &childList = mChildrenMap.find(nodeIndex)->second;
    childs.resize(childList.counter);
    for (u32 i = 0u; i < childList.counter; i++) {
      childs[i] = childList.list[i];
    }
    return true;
  }

  bool GetParents(NodeIndex nodeIndex, std::vector<NodeIndex> &parents) const {
    parents.clear();
    auto nIt = mNodes.find(nodeIndex);
    if (nIt == mNodes.end())
      return false;

    auto &parentList = mParentsMap.find(nodeIndex)->second;
    parents.resize(parentList.counter);
    for (u32 i = 0u; i < parentList.counter; i++) {
      parents[i] = parentList.list[i];
    }
    return true;
  }

  void GetRoots(std::vector<NodeIndex> &roots) const {
    roots.clear();
    roots.reserve(mRoots.size());
    for (auto r : mRoots) {
      roots.push_back(r);
    }
  }

  bool IsMultiGraphs() { return mRoots.size() > 1; }

  bool IsDirty() { return mIsDirty; }

  void Check() {
    if (!mIsDirty)
      return;

    mIsAcyclic = checkAcyclic();

    setClean();
  }

  bool IsAcyclic() {
    Check();

    return mIsAcyclic;
  }

private:
  inline void setDirty() { mIsDirty = true; }

  inline void setClean() { mIsDirty = false; }

  bool _rec_acyclic(std::unordered_set<NodeIndex> visit, NodeIndex next) {
    if (visit.find(next) != visit.end())
      return false;

    visit.insert(next);
    std::vector<NodeIndex> childs;
    GetChilds(next, childs);
    for (auto c : childs) {
      if (!_rec_acyclic(visit, c))
        return false;
    }
    return true;
  }

  bool checkAcyclic() {
    if (mRoots.size() == 0)
      return false;

    for (auto r : mRoots) {
      std::unordered_set<NodeIndex> visit;
      visit.insert(r);
      std::vector<NodeIndex> childs;
      GetChilds(r, childs);
      for (auto c : childs) {
        if (!_rec_acyclic(visit, c))
          return false;
      }
    }
    return true;
  }

  bool addRoot(NodeIndex idx) {
    if (!SimpleGraph<nCap, lCap>::IsValid(idx))
      return false;

    if (mRoots.find(idx) != mRoots.end())
      return false;

    mRoots.insert(idx);
    return true;
  }

  bool removeRoot(NodeIndex idx) {
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

// typedef u32 EDGE_VALUE;
// typedef u32 NODE_VALUE;

template <typename NODE_VALUE, typename EDGE_VALUE>
struct DirectedAcyclicGraph {
private:
  struct Edge {
    NodeHandle from;
    NodeHandle to;
    EDGE_VALUE value;
  };

  struct Node {
    NODE_VALUE value;

    u32 counter = 0u;
  };

  struct Direction {
    std::unordered_map<NodeHandle, EdgeHandle> out;
    std::unordered_map<NodeHandle, EdgeHandle> in;
  };

public:
  NodeHandle add_node(NODE_VALUE value) {
    NodeHandle handle = create_node_handle();
    Node n;
    n.value = value;
    nodes.insert({handle, n});
    noIncommingNodes.insert(handle);
    links.insert({handle, Direction()});
    return handle;
  }

  EdgeHandle add_edge(NodeHandle from, NodeHandle to, EDGE_VALUE value) {
    noIncommingNodes.erase(to);
    if (!has_node(from) || !has_node(to)) {
      // throw
      assert(false);
      return invalidHandle;
    }
    EdgeHandle handle = create_edge_handle();
    Edge e;
    e.from = from;
    e.to = to;
    e.value = value;
    edges.insert({handle, e});

    add_link(from, to, handle);

    return handle;
  }

  void remove_node(NodeHandle handle) {
    if (has_node(handle)) {
      noIncommingNodes.erase(handle);
      nodes.erase(handle);
      freeNodeHandle.push(handle);

      if (auto ll = links.find(handle); ll != links.end()) {
        auto &out = ll->second.out;
        auto &in = ll->second.in;

        for (auto o : out) {
          auto e = o.second;
          edges.erase(e);
          freeEdgeHandle.push(e);
          links.find(o.first)->second.in.erase(handle);
          if (links.find(o.first)->second.in.size() == 0)
            noIncommingNodes.insert(o.first);
        }
        for (auto i : in) {
          auto e = i.second;
          edges.erase(e);
          freeEdgeHandle.push(e);
          links.find(i.first)->second.out.erase(handle);
        }
      }

      links.erase(handle);
    }
  }

  void remove_edge(NodeIndex from, NodeIndex to) {
    if (!has_edge(from, to)) {
      // throw
      assert(false);
      return;
    }

    auto edge = links.find(from)->second.out.find(to)->second;
    freeEdgeHandle.push(edge);
    links.find(from)->second.out.erase(to);
    links.find(to)->second.in.erase(from);
    edges.erase(edge);

    if (links.find(to)->second.in.size() == 0)
      noIncommingNodes.insert(to);
  }

  void remove_edge(EdgeHandle handle) {
    if (!has_edge(handle)) {
      // throw
      assert(false);
      return;
    }
    auto ei = edges.find(handle);
    auto from = ei->second.from;
    auto to = ei->second.to;
    remove_edge(from, to);
  }

  NODE_VALUE get_node_value(NodeHandle handle) const {
    if (!has_node(handle)) {
      // throw
      assert(false);
      return NODE_VALUE();
    }

    return nodes.find(handle)->second.value;
  }

  void set_node_value(NodeHandle handle, NODE_VALUE value) {
    if (!has_node(handle)) {
      // throw
      assert(false);
    }

    nodes.find(handle)->second.value = value;
  }

  EdgeHandle get_edge_handle(NodeIndex from, NodeIndex to) {
    if (!has_edge(from, to)) {
      // throw
      return invalidHandle;
    }

    return links.find(from)->second.out.find(to)->second;
  }

  EDGE_VALUE get_edge_value(EdgeHandle handle) {
    if (!has_edge(handle)) {
      // throw
      assert(false);
      return EDGE_VALUE();
    }

    return edges.find(handle)->second.value;
  }

  EDGE_VALUE get_edge(NodeIndex from, NodeIndex to) {
    auto handle = get_edge_handle(from, to);
    return edges.find(handle)->second.value;
  }

  void set_edge_value(EdgeHandle handle, EDGE_VALUE value) {
    has_edge(handle);
    edges.find(handle)->second.value = value;
  }

  void set_edge_value(NodeIndex from, NodeIndex to, EDGE_VALUE value) {
    auto handle = get_edge_handle(from, to);
    set_edge_value(handle, value);
  }

  // bool is_acyclic() {
  //   clear_state();
  //   std::queue<NodeHandle> q;
  //   for (auto nh : noIncommingNodes) {
  //     q.push(nh);
  //   }

  //   u32 visited = 0u;
  //   u32 size = q.size();
  //   while (size != 0u) {
  //     auto nh = q.front();
  //     q.pop();
  //     size--;
  //     u32 counter = nodes.find(nh)->second.counter;
  //     if (counter > links.find(nh)->second.in.size())
  //       return false;
  //     else if (counter == 0u) {
  //       for (auto o : links.find(nh)->second.out) {
  //         visit(nh);
  //         visited++;
  //         q.push(o.first);
  //         size++;
  //       }
  //     }
  //   }

  //   if (visited + noIncommingNodes.size() != nodes.size())
  //     return false;
  //   return true;
  // }

  bool is_acyclic(const std::vector<NodeHandle> &roots, bool reverse) {
    clear_state();

    auto getLinks = [](NodeHandle handle,
                       bool rv, std::unordered_map<NodeHandle, Direction> & lks) -> std::unordered_map<NodeHandle, EdgeHandle> {
      auto ls = lks.find(handle);
      if (ls != lks.end())
        return rv ? ls->second.in : ls->second.out;

      return std::unordered_map<NodeHandle, EdgeHandle>();
    };

    if (reverse) {
      for (auto h : roots) {
        bool is_end = links.find(h)->second.out.size() == 0;
        if (!is_end)
          return false;
      }
    }
    else {
      for (auto h : roots) {
        bool is_start = links.find(h)->second.in.size() == 0;
        if (!is_start)
          return false;
      }
    }

    std::queue<NodeHandle> q;

    for (auto nh : roots) {
      q.push(nh);
    }

    u32 visited = 0u;
    u32 size = q.size();
    while (size != 0u) {
      auto nh = q.front();
      q.pop();
      size--;
      u32 counter = nodes.find(nh)->second.counter;
      if (counter > getLinks(nh, !reverse, links).size())
        return false;
      else if (counter == 0u) {
        for (auto o : getLinks(nh, reverse, links)) {
          visit(nh,reverse);
          visited++;
          q.push(o.first);
          size++;
        }
      }
    }

    if (visited + roots.size() != nodes.size())
      return false;
    return true;
  }

  bool topo_sort(const std::vector<NodeHandle> &roots, bool reverse,
                 std::vector<std::pair<NodeHandle,u32>> &sort) {
    if (!is_acyclic(roots, reverse))
      return false;

    clear_state();
    std::queue<NodeHandle> q;

    auto getLinks = [](NodeHandle handle,
                       bool rv,std::unordered_map<NodeHandle, Direction> lks) -> std::unordered_map<NodeHandle, EdgeHandle> {
      auto ls = lks.find(handle);
      if (ls != lks.end())
        return rv ? ls->second.in : ls->second.out;

      return std::unordered_map<NodeHandle, EdgeHandle>();
    };

    u32 rank = 0;
    for (auto n : roots) {
      q.push(n);
    }
    u32 size = q.size();
    u32 counter = size;
    while (size != 0u) {
      NodeHandle nh = q.front();
      q.pop();
      size--;
      counter--;
      sort.push_back(std::pair{nh,rank});

      auto ls = getLinks(nh,reverse,links);
      for (auto o : ls) {
        if (visit(o.first,reverse)) {
          q.push(o.first);
          size++;
        }
      }

      if(counter==0u)
      {
        counter=size;
        rank+=1;
      }
    }

    return true;
  }

  // std::vector<NodeHandle> topo_sort() {
  //   clear_state();
  //   std::vector<NodeHandle> sortNodes;
  //   std::queue<NodeHandle> q;
  //   for (auto n : noIncommingNodes) {
  //     q.push(n);
  //   }

  //   u32 size = q.size();
  //   while (size != 0u) {
  //     NodeHandle nh = q.front();
  //     q.pop();
  //     size--;
  //     sortNodes.push_back(nh);

  //     for (auto o : links.find(nh)->second.out) {
  //       if (visit(o.first)) {
  //         q.push(o.first);
  //         size++;
  //       }
  //     }
  //   }

  //   return sortNodes;
  // }

  bool has_node(NodeHandle handle) const {
    return nodes.find(handle) != nodes.end();
  }

  bool has_edge(EdgeHandle handle) const {
    return edges.find(handle) != edges.end();
  }

  bool has_edge(NodeHandle from, NodeHandle to) const {
    if (!has_node(from) || !has_node(to))
      return false;

    auto &out = links.find(from)->second.out;
    if (out.find(to) != out.end())
      return true;

    return false;
  }

  const static u32 invalidHandle = 0xFFFFFFFF;

private:
  void clear_state() {
    for (auto &n : nodes) {
      n.second.counter = 0u;
    }
  }

  bool visit(NodeHandle handle, bool reverse) {
    auto &n = nodes.find(handle)->second;
    n.counter++;
    if (n.counter == (reverse?links.find(handle)->second.out.size():links.find(handle)->second.in.size()))
      return true;
    else
      return false;
  }

  void add_link(NodeHandle from, NodeHandle to, EdgeHandle edge) {
    if (links.find(from) == links.end())
      links.insert({from, Direction()});
    if (links.find(to) == links.end())
      links.insert({to, Direction()});

    links.find(from)->second.out.insert({to, edge});
    links.find(to)->second.in.insert({from, edge});
  }

  NodeHandle create_node_handle() {
    NodeHandle handle = invalidHandle;
    if (freeNodeHandle.size() > 0u) {
      handle = freeNodeHandle.front();
      freeNodeHandle.pop();
    } else if (currentNodeHandle != invalidHandle) {
      handle = currentNodeHandle++;
    } else {
      // should throw
      assert(false);
    }
    return handle;
  }

  EdgeHandle create_edge_handle() {
    EdgeHandle handle = invalidHandle;
    if (freeEdgeHandle.size() > 0u) {
      handle = freeEdgeHandle.front();
      freeEdgeHandle.pop();
    } else if (currentEdgeHandle != invalidHandle) {
      handle = currentEdgeHandle++;
    } else {
      // should throw
      assert(false);
    }
    return handle;
  }

  std::unordered_set<NodeHandle> noIncommingNodes;
  std::unordered_map<NodeHandle, Node> nodes;
  std::unordered_map<EdgeHandle, Edge> edges;

  std::unordered_map<NodeHandle, Direction> links;

  std::queue<NodeHandle> freeNodeHandle;
  std::queue<EdgeHandle> freeEdgeHandle;
  NodeHandle currentNodeHandle = 0u;
  EdgeHandle currentEdgeHandle = 0u;
};
} // namespace unknown::structure
  // namespace unknown::structure