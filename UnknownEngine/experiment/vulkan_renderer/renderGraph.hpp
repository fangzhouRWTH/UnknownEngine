#pragma once

#include "core/structure/graph.hpp"
#include "platform/type.hpp"
#include "vulkan_renderer/common.hpp"
#include "vulkan_renderer/renderPass.hpp"
#include "vulkan_renderer/resource.hpp"
#include "vulkan_renderer/types.hpp"
#include "vulkan_renderer/vulkanContext.hpp"
#include <memory>
#include <variant>

namespace unknown::renderer::vulkan {
enum class Access { ReadOnly, WriteOnly, ReadWrite };

struct Attachment {
  ImageHandle handle;
  // test
  Access access = Access::ReadWrite;
};

struct RenderGraphNode {};

class PassDependency {};

class RenderGraph {
public:
};

class RenderGraphBuilder {
private:
  struct DependencyInfo {
    //PassReflection::Field src;
    //PassReflection::Field dst;
    std::string src;
    std::string dst;
  };

  struct Dependency {
    std::vector<DependencyInfo> data;
  };

  struct OutputMark
  {
    std::string passName;
    std::string fieldName;
    u32 graphIndex;
  };

public:
  void AddPrePass(std::shared_ptr<IRenderPass> pass)
  {
    if(!pass->IsForced())
      return;
    auto passName = pass->GetName();
    assert(!hasPass(passName));
    mForcedPrepass.push_back(pass);
  }

  void AddPass(std::shared_ptr<IRenderPass> pass) {
    if(pass->IsForced())
      return;
    auto passName = pass->GetName();
    assert(!hasPass(passName));
    addPassNode(passName, pass);
  }

  void AddDependency(const std::string &src, const std::string &dst) {
    auto srcPassName = getPassName(src);
    auto dstPassName = getPassName(dst);

    assert(hasPass(srcPassName));
    assert(hasPass(dstPassName));

    auto srcFieldName = getFieldName(src);
    auto dstFieldName = getFieldName(dst);

    addDependency(srcPassName,dstPassName,DependencyInfo{.src = srcFieldName, .dst = dstFieldName});
  }

  void MarkOutput(const std::string & output)
  {
    auto passName = getPassName(output);
    auto fieldName = getPassName(output);

    //TODO
    assert(hasPass(passName));

    OutputMark mark;
    mark.graphIndex = mPassesNameToIndex[passName];
    mark.passName = passName;
    mark.fieldName = fieldName;
    mOutputs.push_back(mark);
  }

  void Import() {}
  void Export() {}

  void Build() {
    std::vector<u32> ends;
    for(auto o : mOutputs)
    {
      ends.push_back(o.graphIndex);
    }
    mSortedPassIndices.clear();
    assert(mGraph.topo_sort(ends,true,mSortedPassIndices));
  }

private:
  std::shared_ptr<IRenderPass> getPassByName(const std::string &name)
  {
    u32 nodeIndex = mPassesNameToIndex[name];
    mGraph.get_node_value(nodeIndex);
    return mPasses[mGraph.get_node_value(nodeIndex)];
  }

  std::string getPassName(const std::string &name) {
    auto pos = name.find('.');
    if (pos != std::string::npos)
      return name.substr(0, pos);
    return std::string();
  }

  std::string getFieldName(const std::string &name) {
    auto pos = name.find('.');
    if (pos != std::string::npos)
      return name.substr(pos + 1, name.size() - pos - 1);
    return std::string();
  }

  bool hasPass(const std::string &passName) {
    return (mPassesNameToIndex.find(passName) != mPassesNameToIndex.end());
  }

  bool hasEdge(const std::string &srcPass, const std::string &dstPass)
  {
    auto srcPassNodeHandle = mPassesNameToIndex[srcPass];
    auto dstPassNodeHandle = mPassesNameToIndex[dstPass];
    auto h = mGraph.get_edge_handle(srcPassNodeHandle,dstPassNodeHandle);
    return h!=mGraph.invalidHandle;
  }

  u32 addEdge(const std::string &srcPass, const std::string &dstPass, u32 value)
  {
    auto srcPassNodeHandle = mPassesNameToIndex[srcPass];
    auto dstPassNodeHandle = mPassesNameToIndex[dstPass];
    return mGraph.add_edge(srcPassNodeHandle,dstPassNodeHandle, value);
  }

  u32 getEdgeHandle(const std::string &srcPass, const std::string &dstPass)
  {
    auto srcPassNodeHandle = mPassesNameToIndex[srcPass];
    auto dstPassNodeHandle = mPassesNameToIndex[dstPass];
    return mGraph.get_edge_handle(srcPassNodeHandle,dstPassNodeHandle);
  }

  u32 getDependencyIndex(const std::string &srcPass, const std::string &dstPass)
  {
    auto h = getEdgeHandle(srcPass,dstPass);
    return mGraph.get_edge_value(h);
  }

  void addPassNode(const std::string &passName,
                   std::shared_ptr<IRenderPass> pass) {
    mPasses.push_back(pass);
    auto nodeIndex = mGraph.add_node(mPasses.size() - 1);
    mPassesNameToIndex.insert({passName, nodeIndex});
  }

  void addDependency(const std::string & srcPassName, const std::string & dstPassName, DependencyInfo info) {
    u32 depId;
    if(hasEdge(srcPassName,dstPassName))
      depId = getDependencyIndex(srcPassName,dstPassName);
    else
    {
      mDependencies.emplace_back();
      depId = mDependencies.size()-1;
      addEdge(srcPassName,dstPassName,depId);
    }

    mDependencies[depId].data.push_back(info);
  }

private:
  structure::DirectedAcyclicGraph<u32, u32> mGraph;

  std::unordered_map<std::string, u32> mPassesNameToIndex;
  std::vector<std::shared_ptr<IRenderPass>> mPasses;
  std::vector<std::shared_ptr<IRenderPass>> mForcedPrepass;

  std::vector<std::pair<structure::NodeHandle,u32>> mSortedPassIndices;

  std::vector<OutputMark> mOutputs;

  std::vector<Dependency> mDependencies;
};

} // namespace unknown::renderer::vulkan