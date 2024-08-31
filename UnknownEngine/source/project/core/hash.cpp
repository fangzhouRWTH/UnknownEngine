#include "hash.hpp"

#include <functional>

namespace unknown::math
{
    u64 HashString(std::string_view strView)
    {
        // assert(!info.path.empty());
        std::hash<std::string_view> strhash;
        return strhash(strView);
    }
}