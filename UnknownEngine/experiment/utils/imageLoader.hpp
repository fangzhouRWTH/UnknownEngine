#pragma once
#include "platform/type.hpp"
#include <string>

namespace unknown
{
    typedef byte* RawPtr;
    class ImageLoader
    {
        public:
            static RawPtr Load(const std::string & path, u32 & width,u32 & height,u32 & channels);
            static void Free(RawPtr data);
    };
}