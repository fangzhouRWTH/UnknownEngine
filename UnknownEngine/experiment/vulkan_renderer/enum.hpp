#pragma once

namespace unknown::renderer::vulkan {
enum struct EImageFormat {
  Unknown,
  R8Unorm,
  R8Snorm,
  R16Unorm,
  R16Snorm,
  RG8Unorm,
  RG8Snorm,
  RG16Unorm,
  RG16Snorm,
  RGB5A1Unorm,
  RGBA8Unorm,
  RGBA8Snorm,
  RGB10A2Unorm,
  RGB10A2Uint,
  RGBA16Unorm,
  RGBA16Snorm,
  RGBA8UnormSrgb,
  R16Float,
  RG16Float,
  RGBA16Float,
  R32Float,
  RG32Float,
  RGB32Float,
  RGBA32Float,
  R11G11B10Float,
  RGB9E5Float,
  R8Int,
  R8Uint,
  R16Int,
  R16Uint,
  R32Int,
  R32Uint,
  RG8Int,
  RG8Uint,
  RG16Int,
  RG16Uint,
  RG32Int,
  RG32Uint,
  RGB32Int,
  RGB32Uint,
  RGBA8Int,
  RGBA8Uint,
  RGBA16Int,
  RGBA16Uint,
  RGBA32Int,
  RGBA32Uint,

  BGRA4Unorm,
  BGRA8Unorm,
  BGRA8UnormSrgb,

  BGRX8Unorm,
  BGRX8UnormSrgb,
  R5G6B5Unorm,

  // Depth-stencil
  D32Float,
  D32FloatS8Uint,
  D16Unorm,

  // Compressed formats
  BC1Unorm, // DXT1
  BC1UnormSrgb,
  BC2Unorm, // DXT3
  BC2UnormSrgb,
  BC3Unorm, // DXT5
  BC3UnormSrgb,
  BC4Unorm, // RGTC Unsigned Red
  BC4Snorm, // RGTC Signed Red
  BC5Unorm, // RGTC Unsigned RG
  BC5Snorm, // RGTC Signed RG
  BC6HS16,
  BC6HU16,
  BC7Unorm,
  BC7UnormSrgb,

  ENUM_MAX
};
}