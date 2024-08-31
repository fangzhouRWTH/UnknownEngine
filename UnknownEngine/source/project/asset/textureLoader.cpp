#include "textureLoader.hpp"

#define USING_STB

#if defined(USING_STB)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "renderer/renderer.hpp"

namespace unknown::asset
{
    void TextureLoader::Initialize()
    {
        stbi_set_flip_vertically_on_load(true);
    }

    byte* TextureLoader::LoadImage(const char* path, u32& width, u32& height, u32& nrChannels, u32 r)
    {
        int iwidth,iheight,inrChannels;
        byte* data = stbi_load(path, &iwidth, &iheight,&inrChannels, r);
        width = u32(iwidth);
        height = u32(iheight);
        nrChannels = u32(inrChannels);
        return data;
    }

    void TextureLoader::FreeImage(byte* data)
    {
        stbi_image_free(data);
    }
}
#endif