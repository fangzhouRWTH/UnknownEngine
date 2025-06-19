#pragma once

#include "handle.hpp"
#include <cassert>
#include <vector>

namespace unknown {

#define DECL_CONTAINER_HANDLE(HANDLE_NAME, RECYCLE_POLICY, CONTAINER,          \
                              RESOURCE)                                        \
  class HANDLE_NAME                                                            \
      : public HandleBase<HANDLE_NAME, RECYCLE_POLICY<HANDLE_NAME>> {          \
    friend class CONTAINER<HANDLE_NAME, RESOURCE>;                             \
  };

template <typename HandleType, typename ResourceType> class VectorContainer {
public:
  void resize(u64 size) { container.resize(size, {false, ResourceType()}); }

  HandleType push(ResourceType resource) {
    HandleType handle = HandleType::create();

    if (handle.isValid()) {
      if (handle.get() >= container.size()) {
        u32 newSize = (container.size() + 1u) * 2u; // MARK
        resize(newSize);
      }

      container[handle.get()].first = true;
      container[handle.get()].second = resource;
    }

    return handle;
  }

  const ResourceType & get(HandleType handle) const {
    return container[handle.get()].second;
  }

  ResourceType & get(HandleType handle) {
    assert(handle.isValid());
    return container[handle.get()].second;
  }

  std::vector<std::pair<bool, ResourceType>> &getContainer() {
    return container;
  }

  void release(HandleType handle) {
    container[handle.get()].first = false;
    handle.release();
  }

  void reset() {
    u64 size = container.size();
    std::vector<std::pair<bool, ResourceType>> newContainer(
        size, {false, ResourceType()});
    container.swap(newContainer);
    HandleType::reset_all();
  }

private:
  std::vector<std::pair<bool, ResourceType>> container;
};
} // namespace unknown