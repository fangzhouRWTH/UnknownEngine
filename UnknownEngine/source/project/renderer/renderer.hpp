#pragma once
#include "platform/type.hpp"
#include "define/enums.hpp"

#include "api_interface.hpp"
#include "memory/resource.hpp"
#include "rendererHandles.hpp"

#include "core/math.hpp"

#include <memory>
#include <unordered_map>
#include <string_view>
#include <span>

//#define API_OPENGL
#define API_VULKAN

namespace unknown::renderer
{
    class Texture2D;

    class GraphicBackend
    {
    public:
        struct ProgramInfos : public ResourceInfos<ProgramHandle>
        {
        };

        struct TextureInfos : public ResourceInfos<TextureHandle>
        {
            enum class Format : u8
            {
                RGB,
                RGBA,

                ENUM_MAX
            };
        };

        static void Initialize();
        static std::shared_ptr<GraphicBackend> Instance();

        static ProgramHandle CreateProgram(const char *vsCode, const char *fsCode);
        static TextureHandle CreateTexture(u32 width, u32 height, byte *data, ImageFormat format);
        static RenderElementHandle CreateMesh(VertexLayout layout, float *vertices, u32 vSize, u32 *indices = nullptr, u32 iSize = 0u);
        // static bool FindProgram(u32 hash, ProgramInfos &infos);
        // static bool FindProgram(ProgramHandle handle, ProgramInfos &infos);
        static bool UseProgram(ProgramHandle handle);

        static void SetUniform(ProgramHandle handle, std::string_view name, i32 value);
        static void SetUniform(ProgramHandle handle, std::string_view name, bool value);
        static void SetUniform(ProgramHandle handle, std::string_view name, float value);
        static void SetUniform(ProgramHandle handle, std::string_view name, Mat4f mat);
        static void SetUniform(ProgramHandle handle, std::string_view name, Vec2f vec);
        static void SetUniform(ProgramHandle handle, std::string_view name, Vec3f vec);
        static void SetUniform(ProgramHandle handle, std::string_view name, Vec4f vec);

        // static bool FindTexture(const std::string &path, TextureInfos &infos);
        // static bool FindTexture(u32 hash, TextureInfos &infos);
        // static bool FindTexture(TextureHandle handle, TextureInfos &infos);
        static bool UseTexture(TextureHandle handle, u32 slot = 0u);
        static bool UseTexture(std::span<TextureHandle> textures);

        static void SetRenderStates(RenderStates states);
        static void Draw(RenderElementHandle handle);
        static void SetClearColor(Vec4f color);
        static void Clear(ClearFrameBuffer clear);

    private:
        static std::shared_ptr<GraphicAPI> impl() { return sInstancePtr->mDetail; }
        static std::shared_ptr<GraphicBackend> sInstancePtr;

        // static std::unordered_map<u32, ShaderProgramInfos> mProgramMap;
        // static std::unordered_map<u32, TextureInfos> mTextureMap;

        // static ResourceMap<ProgramHandle, ProgramInfos> mProgramMap;
        // static ResourceMap<TextureHandle, TextureInfos> mTextureMap;

    public:
    private:
        std::shared_ptr<GraphicAPI> mDetail;
    };
}