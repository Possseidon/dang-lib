#pragma once

#include "Object.h"
#include "Program.h"
#include "VBO.h"

namespace dang::gl
{

enum class BeginMode : GLenum {
    Points = GL_POINTS,
    Lines = GL_LINES,
    LineLoop = GL_LINE_LOOP,
    LineStrip = GL_LINE_STRIP,
    Triangles = GL_TRIANGLES,
    TriangleStrip = GL_TRIANGLE_STRIP,
    TriangleFan = GL_TRIANGLE_FAN,
    LinesAdjacency = GL_LINES_ADJACENCY,
    LineStripAdjacency = GL_LINE_STRIP_ADJACENCY,
    TrianglesAdjacency = GL_TRIANGLES_ADJACENCY,
    TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
    Patches = GL_PATCHES
};

struct VAOInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr BindingPoint BindingPoint = BindingPoint::VertexArray;
};

class VAOBase : public Object<VAOInfo> {
public:
    VAOBase(Program& program, BeginMode mode = BeginMode::Triangles);

    Program& program() const;

    BeginMode mode() const;
    void setMode(BeginMode mode);

private:
    Program& program_;
    BeginMode mode_;
};

template <typename T>
class VAO : public VAOBase {
public:
    VAO(Program& program, VBO<T>& vbo, BeginMode mode = BeginMode::Triangles)
        : VAOBase(program, mode)
        , vbo_(vbo)
    {
        assert(program.attributeStride() == sizeof(T));
        enableAttributes();
    }

    void draw() const
    {
        bind();
        program().bind();
        glDrawArrays(static_cast<GLenum>(mode()), 0, vbo_.count());
    }

private:
    /// <summary>Automatically enables the correct attributes for the VAO, as specified in the program.</summary>
    void enableAttributes()
    {
        bind();
        vbo_.bind();
        // TODO: Instancing -> multiple VBOs and glVertexAttribDivisor
        for (ShaderAttribute& attribute : program().attributeOrder()) {
            const auto component_count = getDataTypeComponentCount(attribute.type());
            const auto base_type = getBaseDataType(attribute.type());
            const auto component_size = component_count * getDataTypeSize(base_type);

            const auto index = [&attribute](GLuint offset) -> GLuint { return attribute.location() + offset; };
            const GLint size = component_count;
            const GLenum type = static_cast<GLenum>(base_type);
            const GLboolean normalized = GL_FALSE;
            const GLsizei stride = program().attributeStride();
            const auto pointer = [&attribute, component_size](GLuint offset) {
                const auto result = static_cast<std::uintptr_t>(offset) * component_size + attribute.offset();
                return reinterpret_cast<const void*>(result);
            };

            // matrices take up one location per column
            // arrays take up one location per index
            GLuint location_count = getDataTypeColumnCount(attribute.type()) * attribute.count();

            for (GLuint offset = 0; offset < location_count; offset++)
                glEnableVertexAttribArray(index(offset));

            switch (base_type) {
            case DataType::Float:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribPointer(index(offset), size, type, normalized, stride, pointer(offset));
                break;

            case DataType::Double:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribLPointer(index(offset), size, type, stride, pointer(offset));
                break;

            case DataType::Bool:
            case DataType::Int:
            case DataType::UInt:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribIPointer(index(offset), size, type, stride, pointer(offset));
            }
        }
    }

    VBO<T>& vbo_;
};

}
