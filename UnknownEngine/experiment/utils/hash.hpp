#pragma once

#include "platform/type.hpp"
#include <string>

namespace unknown
{
    template <typename T>
    void hash_combine(u64 & seed, const T& val)
    {
        seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template<typename... Args>
    u64 make_combined_hash(const Args&... args){
        u64 seed = 0;
        (hash_combine(seed,args),...);
        return seed;
    }
}