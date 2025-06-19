#pragma once

#include "platform/type.hpp"
#include "utils/hash.hpp"

#include <cassert>
#include <string>

namespace unknown {
struct SString {
  bool operator==(const SString &rhs) const {
    bool res = rhs.size == size;
    if (!res)
      return false;

    for (auto i = 0; i < size; i++)
      res &= str[i] == rhs.str[i];

    return res;
  }

  u64 hash() const {
    std::hash<std::string> hasher;
    return hasher(getString());
  }

  SString() {}

  SString(const std::string &_str) { assign(_str); }

  SString(const SString &_str) { assign(_str); }

  SString(const char *c) { assign(c); }

  SString &operator=(const char *c) {
    assign(c);
    return *this;
  }

  SString &operator=(const SString &rhs) {
    assign(rhs);
    return *this;
  }

  SString &operator=(const std::string &rhs) {
    assign(rhs);
    return *this;
  }

  std::string getString() const {
    std::string _str(str, size);
    return _str;
  }

  std::string data() const { return str; }

  u32 getSize() const { return size; }

private:
  void assign(const char *c) {
    if (c) {
      size = std::strlen(c);
      std::strcpy(str, c);
    }
  }

  void assign(const std::string &_str) {
    assert(_str.size() <= capacity);
    size = _str.size();
    memcpy(&str, _str.data(), size);
  }

  void assign(const SString &_str) {
    assert(_str.size <= capacity);
    size = _str.size;
    memcpy(&str, _str.str, size);
  }

  constexpr static u8 capacity = 64;
  char str[capacity];
  u8 size = 0;
};
} // namespace unknown

namespace std {
template <> struct hash<unknown::SString> {
  std::size_t operator()(const unknown::SString &key) const {
    return std::size_t(key.hash());
  }
};
} // namespace std
