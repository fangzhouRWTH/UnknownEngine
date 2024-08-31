#pragma once
#include "core/handles.hpp"
#include "memory/resource.hpp"

namespace unknown::renderer
{
    // _HANDLEDECL(ProgramHandle);
    // _HANDLEDECL(TextureHandle);
    // _HANDLEDECL(MeshHandle);

    // _HANDLEDECL(RenderElementHandle);

    struct ProgramHandle : public Handle<ProgramHandle>
    {
        template <typename H, typename R>
        friend class ResourceMap;

        template <typename H, typename R>
        friend class ResourceArray;
    };

    struct TextureHandle : public Handle<TextureHandle>
    {
        template <typename H, typename R>
        friend class ResourceMap;

        template <typename H, typename R>
        friend class ResourceArray;
    };

    struct MeshHandle : public Handle<MeshHandle>
    {
        template <typename H, typename R>
        friend class ResourceMap;

        template <typename H, typename R>
        friend class ResourceArray;
    };

    struct RenderElementHandle : public Handle<RenderElementHandle>
    {
        template <typename H, typename R>
        friend class ResourceMap;

        template <typename H, typename R>
        friend class ResourceArray;
    };

    // VULKAN

    struct GPUMeshBufferHandle : public Handle<GPUMeshBufferHandle>
    {
        template <typename H, typename R>
        friend class ResourceMap;

        template <typename H, typename R>
        friend class ResourceArray;
    };
}