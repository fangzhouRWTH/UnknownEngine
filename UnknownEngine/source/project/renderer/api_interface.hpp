#pragma once
#include "rendererHandles.hpp"
#include "core/bit.hpp"

namespace unknown::renderer
{
    struct RenderStates
    {
        enum class State : u32
        {
            DepthTest,
            BackCulling,

            ENUM_MAX,
        };

        void reset() { states.Reset(); }
        void set(State state) { states.Set(state); }
        bool has(State state) { return states.IsSet(state); }

    private:
        BitMask<State> states;
    };

    enum class VertexElement : u8
    {
        UNDEFINED,

        Float,
        Vec2,
        Vec3,
        Vec4,
        Position3f,
        Normal3f,
        TexCoord2f,

        ENUM_MAX,
    };

    struct VertexLayout
    {
    public:
        VertexLayout &begin()
        {
            _pos = 0u;
            _count = 0u;
            _byteSize = 0u;
            for (auto &e : _elements)
            {
                e = VertexElement::UNDEFINED;
            }
            return *this;
        }
        VertexLayout &add(VertexElement ele)
        {
            assert(_pos < _max);
            _elements[_pos++] = ele;
            _count++;
            switch (ele)
            {
            case VertexElement::Vec4:
                _byteSize += 16u;
                break;
            case VertexElement::Position3f:
            case VertexElement::Normal3f:
            case VertexElement::Vec3:
                _byteSize += 12u;
                break;
            case VertexElement::TexCoord2f:
            case VertexElement::Vec2:
                _byteSize += 8u;
                break;
            case VertexElement::Float:
                _byteSize += 4u;
                break;
            default:
                assert(false);
            }
            return *this;
        }
        void end() { _pos = 0u; }

        VertexElement get()
        {
            if (_pos == _max)
                return VertexElement::UNDEFINED;
            return _elements[_pos++];
        }

        u32 size()
        {
            return _byteSize;
        }

    private:
        constexpr static u32 _max = 16u;
        VertexElement _elements[_max];
        u32 _pos = 0u;
        u32 _count = 0u;
        u32 _byteSize = 0u;
    };

    enum class ClearFrameBuffer
    {
        Color,
        Depth,
        Color_Depth,
    };

    class GraphicAPI
    {
    public:
        virtual void initialize() = 0;
        virtual void shutdown() = 0;
        virtual ProgramHandle create_shader(const char *vsCode, const char *fsCode) = 0;
        virtual TextureHandle create_texture_2d(u32 width, u32 height, byte *data, ImageFormat format) = 0;
        virtual RenderElementHandle create_mesh(VertexLayout layout, float *vertices, u32 uSize, u32 *indices, u32 iSize) = 0;

        virtual void use_program(ProgramHandle handle) = 0;
        virtual void set_uniform_bool(ProgramHandle handle, const char *name, bool value) = 0;
        virtual void set_uniform_int(ProgramHandle handle, const char *name, int value) = 0;
        virtual void set_uniform_float(ProgramHandle handle, const char *name, float value) = 0;
        virtual void set_uniform_matrix_4_float(ProgramHandle handle, const char *name, float *data) = 0;
        virtual void set_uniform_vector_2_float(ProgramHandle handle, const char *name, float *data) = 0;
        virtual void set_uniform_vector_3_float(ProgramHandle handle, const char *name, float *data) = 0;
        virtual void set_uniform_vector_4_float(ProgramHandle handle, const char *name, float *data) = 0;

        virtual void use_texture_2d(TextureHandle handle, u32 slot) = 0;

        virtual void set_render_states(RenderStates states) = 0;
        virtual void set_clear_color(float r, float g, float b, float a) = 0;
        virtual void clear_frame_buffer(ClearFrameBuffer clear) = 0;
        virtual void draw(RenderElementHandle handle) = 0;

        virtual VertexLayout get_default_vertex_layout() = 0;
    };
}