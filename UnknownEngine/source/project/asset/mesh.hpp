#pragma once
#include <vector>
#include "core/math.hpp"
#include "platform/type.hpp"
#include "renderer/texture.hpp"
#include <string>

namespace unknown::asset
{
    // TEMPERAL
    struct Vertex
    {
        Vec3f position = Vec3f(0.f);
        Vec3f normal = Vec3f(0.f);
        Vec2f texCoords = Vec2f(0.f);
    };

    struct Tex
    {
        renderer::Texture2D id;
        std::string type;
        std::string path;
    };

    // class Mesh
    // {
    // public:
    //     std::vector<Vertex> mVertices;
    //     std::vector<u32> mIndices;
    //     std::vector<Tex> mTextures;

    //     Mesh(std::vector<Vertex> vertices,
    //          std::vector<u32> indices,
    //          std::vector<Tex> textures);
    //     private:
    // };
}