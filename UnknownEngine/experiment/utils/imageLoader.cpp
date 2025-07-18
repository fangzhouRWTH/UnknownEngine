#include "imageLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace unknown {
RawPtr ImageLoader::Load(const std::string &path, u32 &width,
                                u32 &height, u32 &channels) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels,
                              STBI_rgb_alpha);
  width = u32(texWidth);
  height = u32(texHeight);
  channels = u32(texChannels);
  return pixels;
}
void ImageLoader::Free(RawPtr data) {
    stbi_image_free(data);
};
} // namespace unknown