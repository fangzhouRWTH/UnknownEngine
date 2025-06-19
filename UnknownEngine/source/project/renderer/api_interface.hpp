#pragma once
#include "rendererHandles.hpp"
#include "context/engineContext.hpp"
#include "core/bit.hpp"
#include "core/base.hpp"
#include "core/math.hpp"
#include "core/hash.hpp"
#include "debug/log.hpp"

#include <span>
#include <unordered_map>
#include <cassert>

namespace unknown::renderer
{
    typedef u64 MaterialKey;

    typedef u32 MaterialClassID;

    enum class PipelineStage
    {
        Vertex,
        Fragment,
        VertexFragment,

        Compute,

        EnumMax
    };

    enum class PipelineResourceType
    {
        UniformBuffer,
        Sampler2D,

        EnumMax
    };

    enum class SharedResource
    {
        Local,
        Global,

        EnumMax
    };

    struct MaterialPipelineBinding
    {
        PipelineResourceType type;
        SharedResource shared;
        PipelineStage stage;
        u32 size;
    };

    struct MaterialClassInfo
    {
        std::string_view vertexShader;
        std::string_view fragmentShader;
        std::unordered_map<std::string,MaterialPipelineBinding> bindings;
        MaterialKey materialKey = 0;
        
        void GenerateKey()
        {
            if(materialKey==0)
            {
                materialKey = hash(*this);
            }
        }

        MaterialKey GetKey() const
        {
            return materialKey;
        }

        static u64 hash(const MaterialClassInfo & info)
        {
            u32 stringSize = info.vertexShader.size() + info.fragmentShader.size();
            for(const auto & b : info.bindings)
            {
                stringSize += b.first.size();
            }
            std::string hashStr;
            hashStr.resize(stringSize);

            auto cptr = hashStr.begin();
            std::copy(info.vertexShader.begin(),info.vertexShader.end(),cptr);
            //memcpy(cptr,info.vertexShader.data(),info.vertexShader.size());
            cptr += info.vertexShader.size();
            std::copy(info.fragmentShader.begin(),info.fragmentShader.end(),cptr);
            //memcpy(cptr,info.fragmentShader.data(),info.fragmentShader.size());
            cptr += info.fragmentShader.size();

            for(const auto & [n,b] : info.bindings)
            {
                auto sSize = n.size();
                assert(cptr+sSize<=hashStr.end());
                std::copy(n.begin(),n.end(),cptr);
                //memcpy(cptr, n.data(), sSize);
                cptr += sSize;
            }

            auto h = math::HashString(hashStr);
            
            INFO_LOG("Material Hash [{}], Value [{}]", hashStr,h);

            return h;
        }
    };

    struct SceneUniform
    {
        Mat4f view;
        Mat4f proj;
        Mat4f viewproj;
        Vec4f ambientColor;
        Vec4f sunlightDirection; // w for sun power
        Vec4f sunlightColor;
    };

    struct RendererInitInfo
    {
        void* windowPtr = nullptr;
    };

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

    // enum class VertexElement : u8
    // {
    //     UNDEFINED,

    //     Float,
    //     Vec2,
    //     Vec3,
    //     Vec4,
    //     Position3f,
    //     Normal3f,
    //     TexCoord2f,

    //     ENUM_MAX,
    // };

    // struct VertexLayout
    // {
    // public:
    //     VertexLayout &begin()
    //     {
    //         _pos = 0u;
    //         _count = 0u;
    //         _byteSize = 0u;
    //         for (auto &e : _elements)
    //         {
    //             e = VertexElement::UNDEFINED;
    //         }
    //         return *this;
    //     }
    //     VertexLayout &add(VertexElement ele)
    //     {
    //         assert(_pos < _max);
    //         _elements[_pos++] = ele;
    //         _count++;
    //         switch (ele)
    //         {
    //         case VertexElement::Vec4:
    //             _byteSize += 16u;
    //             break;
    //         case VertexElement::Position3f:
    //         case VertexElement::Normal3f:
    //         case VertexElement::Vec3:
    //             _byteSize += 12u;
    //             break;
    //         case VertexElement::TexCoord2f:
    //         case VertexElement::Vec2:
    //             _byteSize += 8u;
    //             break;
    //         case VertexElement::Float:
    //             _byteSize += 4u;
    //             break;
    //         default:
    //             assert(false);
    //         }
    //         return *this;
    //     }
    //     void end() { _pos = 0u; }

    //     VertexElement get()
    //     {
    //         if (_pos == _max)
    //             return VertexElement::UNDEFINED;
    //         return _elements[_pos++];
    //     }

    //     u32 size()
    //     {
    //         return _byteSize;
    //     }

    // private:
    //     constexpr static u32 _max = 16u;
    //     VertexElement _elements[_max];
    //     u32 _pos = 0u;
    //     u32 _count = 0u;
    //     u32 _byteSize = 0u;
    // };

    enum class ClearFrameBuffer
    {
        Color,
        Depth,
        Color_Depth,
    };

    struct InstanceData
    {
        Vec3f pos;
        Vec3f rot;
        float scale;
        u32 texIndex;
    };

    struct RenderObject
    {
        GPUMeshBufferHandle meshBufferHandle;
        Mat4f transform;
        u32 indicesCount;
        
        //material
        MaterialKey materialKey = 0;
    };

    class GraphicAPI
    {
    public:
        virtual void initialize(const RendererInitInfo & info) = 0;
        virtual void shutdown() = 0;

        virtual void try_resize(u32 width, u32 height) = 0;
        virtual void * get_core() = 0;


        virtual void push_render_objects(std::span<RenderObject> objects) = 0;
        virtual void frame(u32 width, u32 height, EngineContext context) = 0;
        // virtual ProgramHandle create_shader(const char *vsCode, const char *fsCode) = 0;
        // virtual TextureHandle create_texture_2d(u32 width, u32 height, byte *data, ImageFormat format) = 0;
        //virtual RenderElementHandle create_mesh(VertexLayout layout, float *vertices, u32 uSize, u32 *indices, u32 iSize) = 0;
        virtual GPUMeshBufferHandle upload_mesh(std::span<uint32_t> indices, std::span<Vertex> vertices) = 0;
        
        virtual void test_init_indirect_draw(u64 indicesCount) = 0;
        
        // virtual void use_program(ProgramHandle handle) = 0;
        // virtual void set_uniform_bool(ProgramHandle handle, const char *name, bool value) = 0;
        // virtual void set_uniform_int(ProgramHandle handle, const char *name, int value) = 0;
        // virtual void set_uniform_float(ProgramHandle handle, const char *name, float value) = 0;
        // virtual void set_uniform_matrix_4_float(ProgramHandle handle, const char *name, float *data) = 0;
        // virtual void set_uniform_vector_2_float(ProgramHandle handle, const char *name, float *data) = 0;
        // virtual void set_uniform_vector_3_float(ProgramHandle handle, const char *name, float *data) = 0;
        // virtual void set_uniform_vector_4_float(ProgramHandle handle, const char *name, float *data) = 0;

        // virtual void use_texture_2d(TextureHandle handle, u32 slot) = 0;

        // virtual void set_render_states(RenderStates states) = 0;
        // virtual void set_clear_color(float r, float g, float b, float a) = 0;
        // virtual void clear_frame_buffer(ClearFrameBuffer clear) = 0;
        // virtual void draw(RenderElementHandle handle) = 0;

        // virtual VertexLayout get_default_vertex_layout() = 0;
    };
}