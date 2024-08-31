#pragma once
#ifdef API_OPENGL
#include "renderer/api_interface.hpp"
#include "renderer/resource.hpp"
#include <glad/glad.h>
#include <cassert>

namespace unknown::renderer::opengl
{
    class API_OpenGL : public GraphicAPI
    {
    public:
        virtual void initialize() override;
        virtual void shutdown() override;

        virtual ProgramHandle create_shader(const char *vsCode, const char *fsCode) override;
        virtual TextureHandle create_texture_2d(u32 width, u32 height, byte *data, ImageFormat format) override;
        virtual RenderElementHandle create_mesh(VertexLayout layout, float *vertices, u32 vSize, u32 *indices, u32 iSize) override;

        virtual void use_program(ProgramHandle handle) override;

        virtual void set_uniform_bool(ProgramHandle handle, const char *name, bool value) override;
        virtual void set_uniform_int(ProgramHandle handle, const char *name, int value) override;
        virtual void set_uniform_float(ProgramHandle handle, const char *name, float value) override;
        virtual void set_uniform_matrix_4_float(ProgramHandle handle, const char *name, float *data) override;
        virtual void set_uniform_vector_2_float(ProgramHandle handle, const char *name, float *data) override;
        virtual void set_uniform_vector_3_float(ProgramHandle handle, const char *name, float *data) override;
        virtual void set_uniform_vector_4_float(ProgramHandle handle, const char *name, float *data) override;

        virtual void use_texture_2d(TextureHandle handle, u32 slot) override;

        virtual void set_render_states(RenderStates states) override;
        virtual void draw(RenderElementHandle handle) override;
        virtual void set_clear_color(float r, float g, float b, float a) override;
        virtual void clear_frame_buffer(ClearFrameBuffer clear) override;

        virtual VertexLayout get_default_vertex_layout() override { return sDefaultVertexLayout; }

    private:
        struct RenderElementInfo : public ResourceInfos<RenderElementHandle>
        {
            u32 VBO;
            u32 VAO;
            u32 EBO;
            u32 verticesCount;
            u32 indicesCount;
        };

        struct ProgramInfos : public ResourceInfos<ProgramHandle>
        {
            u32 programId;
        };

        struct TextureInfos : public ResourceInfos<TextureHandle>
        {
            u32 textureId;

            enum class Format : u8
            {
                RGB,
                RGBA,

                ENUM_MAX
            };
        };

        ResourceArray<RenderElementHandle, RenderElementInfo>;
        ResourceArray<ProgramHandle, ProgramInfos> mProgramMap;
        ResourceArray<TextureHandle, TextureInfos> mTextureMap;

        VertexLayout sDefaultVertexLayout;
    };

    void API_OpenGL::initialize()
    {
        sDefaultVertexLayout.begin()
            .add(VertexElement::Position3f)
            .add(VertexElement::Normal3f)
            .add(VertexElement::TexCoord2f)
            .end();
    }

    void API_OpenGL::shutdown()
    {
    }

    ProgramHandle API_OpenGL::create_shader(const char *vsCode, const char *fsCode)
    {
        u32 vertex, fragment;
        int success;
        char infoLog[512];
        // vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vsCode, NULL);
        glCompileShader(vertex);
        // print compile errors if any
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        };

        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fsCode, NULL);
        glCompileShader(fragment);
        // print compile errors if any
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        };

        u32 ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        // print linking errors if any
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << infoLog << std::endl;
            ID = 0xFFFFFFF;
            assert(false);
        }
        ProgramInfos info;
        info.programId = ID;
        // delete shaders; they’re linked into our program and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        return mProgramMap.AddResource(info);
    }

    TextureHandle API_OpenGL::create_texture_2d(u32 width, u32 height, byte *data, ImageFormat format)
    {
        u32 texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // set the texture wrapping/filtering options (on currently bound texture)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int _format;
        switch (format)
        {
        case ImageFormat::RGB:
            _format = GL_RGB;
            break;
        case ImageFormat::RGBA:
            _format = GL_RGBA;
            break;
        default:
            assert(false);
        }

        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, _format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            assert(false);
        }

        TextureInfos info;
        info.textureId = texture;
        return mTextureMap.AddResource(info);
    }

    // inline u32 API_OpenGL::create_mesh()
    RenderElementHandle API_OpenGL::create_mesh(VertexLayout layout, float *vertices, u32 vSize, u32 *indices, u32 iSize)
    {
        API_OpenGL::RenderElementInfo infos;
        infos.verticesCount = vSize / layout.size();
        infos.indicesCount = iSize / sizeof(u32);

        u32 VBO = 0u;
        u32 VAO = 0u;
        u32 EBO = 0u;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        bool indexDraw = iSize > 0u && indices != nullptr ? true : false;
        if (indexDraw)
            glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vSize, vertices, GL_STATIC_DRAW);

        if (indexDraw)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize, indices, GL_STATIC_DRAW);
        }

        infos.EBO = EBO;
        infos.VAO = VAO;
        infos.VBO = VBO;
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        {
            i32 attrPos = 0;
            i32 byteOffset = 0;
            i32 byteStrideSize = layout.size();
            VertexElement ele = layout.get();
            while (ele != VertexElement::UNDEFINED)
            {
                u32 type;
                i32 typeCount;
                u32 typeSize;
                switch (ele)
                {
                case VertexElement::Vec4:
                    type = GL_FLOAT;
                    typeCount = 4;
                    typeSize = 4u;
                    break;
                case VertexElement::Position3f:
                case VertexElement::Normal3f:
                case VertexElement::Vec3:
                    type = GL_FLOAT;
                    typeCount = 3;
                    typeSize = 4u;
                    break;
                case VertexElement::TexCoord2f:
                case VertexElement::Vec2:
                    type = GL_FLOAT;
                    typeCount = 2;
                    typeSize = 4u;
                    break;
                case VertexElement::Float:
                    type = GL_FLOAT;
                    typeCount = 1;
                    typeSize = 4u;
                    break;
                default:
                    assert(false);
                }
                glVertexAttribPointer(attrPos, typeCount, type, GL_FALSE, byteStrideSize, (void *)byteOffset);
                glEnableVertexAttribArray(attrPos);
                attrPos++;
                byteOffset += typeCount * typeSize;
                ele = layout.get();
            }
        }

        glBindVertexArray(0);

        // // position attribute
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        // glEnableVertexAttribArray(0);
        // // // color attribute
        // // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        // // glEnableVertexAttribArray(1);
        // // texture coord attribute
        // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        // glEnableVertexAttribArray(1);

        return mRenderElementMap.AddResource(infos);
    }

    void API_OpenGL::use_program(ProgramHandle handle)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUseProgram(info.programId);
    }

    void API_OpenGL::set_uniform_bool(ProgramHandle handle, const char *name, bool value)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUniform1i(glGetUniformLocation(info.programId, name), int(value));
    }

    void API_OpenGL::set_uniform_int(ProgramHandle handle, const char *name, int value)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUniform1i(glGetUniformLocation(info.programId, name), value);
    }

    void API_OpenGL::set_uniform_float(ProgramHandle handle, const char *name, float value)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUniform1f(glGetUniformLocation(info.programId, name), value);
    }

    void API_OpenGL::set_uniform_matrix_4_float(ProgramHandle handle, const char *name, float *data)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUniformMatrix4fv(glGetUniformLocation(info.programId, name), 1, GL_FALSE, data);
    }

    void API_OpenGL::set_uniform_vector_2_float(ProgramHandle handle, const char *name, float *data)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUniform2fv(glGetUniformLocation(info.programId, name), 1, data);
    }

    void API_OpenGL::set_uniform_vector_3_float(ProgramHandle handle, const char *name, float *data)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUniform3fv(glGetUniformLocation(info.programId, name), 1, data);
    }

    void API_OpenGL::set_uniform_vector_4_float(ProgramHandle handle, const char *name, float *data)
    {
        ProgramInfos info;

        if (!mProgramMap.FindResource(handle, info))
            assert(false);

        glUniform4fv(glGetUniformLocation(info.programId, name), 1, data);
    }

    void API_OpenGL::use_texture_2d(TextureHandle handle, u32 slot)
    {
        TextureInfos info;

        if (!mTextureMap.FindResource(handle, info))
            assert(false);

        int _slot = int(slot) + GL_TEXTURE0;
        glActiveTexture(_slot);
        glBindTexture(GL_TEXTURE_2D, info.textureId);
    }

    void API_OpenGL::set_render_states(RenderStates states)
    {
        if (states.has(RenderStates::State::DepthTest))
            glEnable(GL_DEPTH_TEST);
        if (states.has(RenderStates::State::BackCulling))
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
        }
    }

    void API_OpenGL::draw(RenderElementHandle handle)
    {
        RenderElementInfo info;

        if (!mRenderElementMap.FindResource(handle, info))
            assert(false);

        bool indexDraw = info.EBO == 0u ? false : true;

        glBindVertexArray(info.VAO);

        if (indexDraw)
            glDrawElements(GL_TRIANGLES, info.indicesCount, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(GL_TRIANGLES, 0, info.verticesCount);

        glBindVertexArray(0);
    }

    void API_OpenGL::set_clear_color(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
    }

    void API_OpenGL::clear_frame_buffer(ClearFrameBuffer clear)
    {
        switch (clear)
        {
        case ClearFrameBuffer::Color:
            glClear(GL_COLOR_BUFFER_BIT);
            break;
        case ClearFrameBuffer::Depth:
            glClear(GL_DEPTH_BUFFER_BIT);
            break;
        case ClearFrameBuffer::Color_Depth:
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            break;
        default:
            assert(false);
        }
    }
}
#endif