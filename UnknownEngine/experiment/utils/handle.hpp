#pragma once
#include "platform/type.hpp"
#include <queue>

namespace unknown {

template <typename Derived> class PolicyBase {
public:
  static u32 current;
  static u32 next() {
    if (current == kInvalid)
      return current;
    return current++;
  }

  const static u32 kInvalid = 0xffffffff;
  static bool isValid(u32 value) { return value != kInvalid; }
};

template <typename Derived> u32 PolicyBase<Derived>::current = 0u;

template <typename Handle>
struct NoRecycle : public PolicyBase<NoRecycle<Handle>> {
  static void release(u32 handle_value) {}
  static u32 create() { return PolicyBase<NoRecycle<Handle>>::next(); }
  static void reset() { PolicyBase<NoRecycle<Handle>>::current = 0; }
};

template <typename Handle> struct Recycle : public PolicyBase<Recycle<Handle>> {
  static void release(u32 handle_value) {
    if (handle_value != PolicyBase<NoRecycle<Handle>>::kInvalid)
      recycleQueue.push(handle_value);
  }
  static u32 create() {
    u32 v = PolicyBase<Recycle<Handle>>::kInvalid;
    if (!recycleQueue.empty()) {
      v = recycleQueue.front();
      recycleQueue.pop();
    } else {
      v = PolicyBase<Recycle<Handle>>::next();
    }
    return v;
  }
  static void reset() {
    PolicyBase<Recycle<Handle>>::current = 0;
    std::queue<u32> newQueue{};
    recycleQueue.swap(newQueue);
  }

protected:
  static std::queue<u32> recycleQueue;
};

template <typename Handle> std::queue<u32> Recycle<Handle>::recycleQueue = {};

template <typename Handle, typename Policy> class HandleBase {
public:
  HandleBase() : value(Policy::kInvalid) {}
  bool isValid() const { return Policy::isValid(value); }
  u32 get() { return value; }
  u32 getNext() { return Policy::current; }

  static void reset_all() { Policy::reset(); }

protected:
  static Handle create() {
    Handle h;
    h.value = Policy::create();
    return h;
  }

  void release() { Policy::release(value); }

  u32 value = Policy<Handle>::kInvalid;
  static Policy policy;
};
} // namespace unknown