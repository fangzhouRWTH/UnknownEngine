#pragma once
#include "platform/type.hpp"
#include "define/enums.hpp"
#include "context/engineContext.hpp"

#include "api_interface.hpp"
#include "memory/resource.hpp"
#include "rendererHandles.hpp"

#include "core/math.hpp"
#include "core/base.hpp"

#include <memory>
#include <unordered_map>
#include <string_view>
#include <span>

// #define API_OPENGL

#define API_VULKAN

namespace unknown::renderer
{
    class Texture2D;

    struct FrameInfo
    {
        u32 width;
        u32 height;
        EngineContext context;
    };

    class IRenderer
    {
    public:
        virtual void Initialize(const RendererInitInfo & info) = 0;
        virtual void ShutDown() = 0;
        virtual void TryResize(u32 width, u32 height) = 0;
        virtual void PushRenderObjects(std::span<RenderObject> objects) = 0;
        virtual void Frame(FrameInfo info) = 0;
        virtual void * GetCore() = 0;
        virtual GPUMeshBufferHandle UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) = 0;
        virtual void TestInitIndirectDraw(u64 indicesCount) = 0;
    };

    enum class EGraphicAPI
    {
        Vulkan
    };

    class RenderEngine final : public IRenderer
    {
    public:
        RenderEngine(EGraphicAPI api);
        ~RenderEngine();
        //static std::shared_ptr<RenderEngine> Get();
        virtual void Initialize(const RendererInitInfo & info) override;
        virtual void ShutDown() override;
        virtual void TryResize(u32 width, u32 height) override;
        virtual void PushRenderObjects(std::span<RenderObject> objects);
        virtual void Frame(FrameInfo info) override;
        virtual void * GetCore() override;

        virtual GPUMeshBufferHandle UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) override;

        virtual void TestInitIndirectDraw(u64 indicesCount) override;
    private:
        // static std::shared_ptr<GraphicAPI> impl() { return sInstancePtr->mDetail; }
        //static std::shared_ptr<RenderEngine> sInstancePtr;

        //ResourceTable<u32> resourceTest;

        // static std::unordered_map<u32, ShaderProgramInfos> mProgramMap;
        // static std::unordered_map<u32, TextureInfos> mTextureMap;

        // static ResourceMap<ProgramHandle, ProgramInfos> mProgramMap;
        // static ResourceMap<TextureHandle, TextureInfos> mTextureMap;

    private:
        const EGraphicAPI mAPI;
        bool mInitialized = false;
        std::unique_ptr<GraphicAPI> mDetail = nullptr;
    };

    // class GraphicBackend
    // {
    // public:
    //     struct ProgramInfos : public ResourceInfos<ProgramHandle>
    //     {
    //     };

    //     struct TextureInfos : public ResourceInfos<TextureHandle>
    //     {
    //         enum class Format : u8
    //         {
    //             RGB,
    //             RGBA,

    //             ENUM_MAX
    //         };
    //     };

    //     static void Initialize();
    //     static std::shared_ptr<GraphicBackend> Instance();

    //     static ProgramHandle CreateProgram(const char *vsCode, const char *fsCode);
    //     static TextureHandle CreateTexture(u32 width, u32 height, byte *data, ImageFormat format);
    //     static RenderElementHandle CreateMesh(VertexLayout layout, float *vertices, u32 vSize, u32 *indices = nullptr, u32 iSize = 0u);
    //     // static bool FindProgram(u32 hash, ProgramInfos &infos);
    //     // static bool FindProgram(ProgramHandle handle, ProgramInfos &infos);
    //     static bool UseProgram(ProgramHandle handle);

    //     static void SetUniform(ProgramHandle handle, std::string_view name, i32 value);
    //     static void SetUniform(ProgramHandle handle, std::string_view name, bool value);
    //     static void SetUniform(ProgramHandle handle, std::string_view name, float value);
    //     static void SetUniform(ProgramHandle handle, std::string_view name, Mat4f mat);
    //     static void SetUniform(ProgramHandle handle, std::string_view name, Vec2f vec);
    //     static void SetUniform(ProgramHandle handle, std::string_view name, Vec3f vec);
    //     static void SetUniform(ProgramHandle handle, std::string_view name, Vec4f vec);

    //     // static bool FindTexture(const std::string &path, TextureInfos &infos);
    //     // static bool FindTexture(u32 hash, TextureInfos &infos);
    //     // static bool FindTexture(TextureHandle handle, TextureInfos &infos);
    //     static bool UseTexture(TextureHandle handle, u32 slot = 0u);
    //     static bool UseTexture(std::span<TextureHandle> textures);

    //     static void SetRenderStates(RenderStates states);
    //     static void Draw(RenderElementHandle handle);
    //     static void SetClearColor(Vec4f color);
    //     static void Clear(ClearFrameBuffer clear);

    // private:
    //     static std::shared_ptr<GraphicAPI> impl() { return sInstancePtr->mDetail; }
    //     static std::shared_ptr<GraphicBackend> sInstancePtr;

    //     // static std::unordered_map<u32, ShaderProgramInfos> mProgramMap;
    //     // static std::unordered_map<u32, TextureInfos> mTextureMap;

    //     // static ResourceMap<ProgramHandle, ProgramInfos> mProgramMap;
    //     // static ResourceMap<TextureHandle, TextureInfos> mTextureMap;

    // public:
    // private:
    //     std::shared_ptr<GraphicAPI> mDetail;
    // };
}