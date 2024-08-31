#pragma once

#include "../platform/type.hpp"
#include <string>

namespace unknown::asset
{
    class TextureLoader
    {
        public:
            static void Initialize();
            static byte* LoadImage(const char* path, u32& width, u32& height, u32& nrChannels, u32 r = 0);
            static void FreeImage(byte* data);
    };
}