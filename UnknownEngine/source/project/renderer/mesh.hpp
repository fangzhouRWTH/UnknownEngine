#pragma once

#include <string>
#include "renderer.hpp"
#include <memory>

namespace unknown::renderer
{
    struct MeshDescription
    {

    };

    class Mesh
    {
    public:
        Mesh();
        Mesh(MeshHandle handle);
        void SetHandle(MeshHandle handle);
        MeshHandle GetHandle() const;

    private:
        MeshHandle mHandle;
    };

    class MeshManager
    {
    public:
        static void Initialize();
        static void Shutdown();
        static MeshHandle LoadMesh(const MeshDescription& desc);
    private:
        struct MeshContainer;
        static std::unique_ptr<MeshContainer> sMeshContainer;
    };
}