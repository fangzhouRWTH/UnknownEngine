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
    #define DECL_HANDLE(HANDLE_NAME) struct HANDLE_NAME : public HandleTemplate<HANDLE_NAME> {}
    // struct GPUMeshBufferHandle : public HandleTemplate<GPUMeshBufferHandle>
    // {
    // };
    DECL_HANDLE(GPUMeshBufferHandle);
    DECL_HANDLE(GPUMeshStorageHandle);

}