#include "renderer.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <cassert>

#ifdef API_OPENGL
#include "opengl/api_opengl.hpp"
#include "configuration/globalValues.hpp"
#endif

namespace unknown::renderer
{
    std::shared_ptr<GraphicBackend> GraphicBackend::sInstancePtr = nullptr;

    void GraphicBackend::Initialize()
    {
        if (sInstancePtr)
            return;

        sInstancePtr = std::make_shared<GraphicBackend>();
#ifdef API_OPENGL
        //sInstancePtr->mDetail = std::make_shared<opengl::API_OpenGL>();
        //sInstancePtr->mDetail->initialize();
#endif
    }

    std::shared_ptr<GraphicBackend> GraphicBackend::Instance()
    {
        if (!sInstancePtr)
            Initialize();

        return sInstancePtr;
    };

    ProgramHandle GraphicBackend::CreateProgram(const char *vsCode, const char *fsCode)
    {
        //return impl()->create_shader(vsCode, fsCode);
        return ProgramHandle();
    }

    bool GraphicBackend::UseProgram(ProgramHandle handle)
    {
        //impl()->use_program(handle);
        return true;
    }

    void GraphicBackend::SetUniform(ProgramHandle handle, std::string_view name, i32 value)
    {
        //impl()->set_uniform_int(handle, name.data(), int(value));
    }

    void GraphicBackend::SetUniform(ProgramHandle handle, std::string_view name, bool value)
    {
        //impl()->set_uniform_bool(handle, name.data(), value);
    }
    void GraphicBackend::SetUniform(ProgramHandle handle, std::string_view name, float value)
    {
        //impl()->set_uniform_float(handle, name.data(), value);
    }

    void GraphicBackend::SetUniform(ProgramHandle handle, std::string_view name, Mat4f mat)
    {
        //impl()->set_uniform_matrix_4_float(handle, name.data(), mat.data());
    }

    void GraphicBackend::SetUniform(ProgramHandle handle, std::string_view name, Vec2f vec)
    {
        //impl()->set_uniform_vector_2_float(handle, name.data(), vec.data());
    }

    void GraphicBackend::SetUniform(ProgramHandle handle, std::string_view name, Vec3f vec)
    {
        //impl()->set_uniform_vector_3_float(handle, name.data(), vec.data());
    }

    void GraphicBackend::SetUniform(ProgramHandle handle, std::string_view name, Vec4f vec)
    {
        //impl()->set_uniform_vector_4_float(handle, name.data(), vec.data());
    }

    TextureHandle GraphicBackend::CreateTexture(u32 width, u32 height, byte *data, ImageFormat format)
    {
        //return impl()->create_texture_2d(width, height, data, format);
        return TextureHandle();
    }

    bool GraphicBackend::UseTexture(TextureHandle handle, u32 slot)
    {
        //impl()->use_texture_2d(handle, slot);

        return true;
    }

    bool GraphicBackend::UseTexture(std::span<TextureHandle> textures)
    {
        // u64 size = textures.size();
        // assert(size < 16);
        // for (u32 i = 0; i < size; i++)
        // {
        //     impl()->use_texture_2d(textures[i], i);
        // }
        return true;
    }

    RenderElementHandle GraphicBackend::CreateMesh(VertexLayout layout, float *vertices, u32 vSize, u32 *indices, u32 iSize)
    {
        //return impl()->create_mesh(layout, vertices, vSize, indices, iSize);
        return RenderElementHandle();
    }

    void GraphicBackend::SetRenderStates(RenderStates states)
    {
        //return impl()->set_render_states(states);
    }

    void GraphicBackend::Draw(RenderElementHandle handle)
    {
        //return impl()->draw(handle);
    }

    void GraphicBackend::SetClearColor(Vec4f color)
    {
        //return impl()->set_clear_color(color.x(), color.y(), color.z(), color.w());
    }

    void GraphicBackend::Clear(ClearFrameBuffer clear)
    {
        //return impl()->clear_frame_buffer(clear);
    }
}