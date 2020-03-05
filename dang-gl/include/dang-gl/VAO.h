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
    void enableAttributes()
    {
        bind();
        vbo_.bind();
        // TODO: Instancing -> multiple VBOs and glVertexAttribDivisor
        for (ShaderAttribute& attribute : program().attributeOrder()) {
            auto base_type = getBaseDataType(attribute.type());
            auto component_count = getDataTypeComponentCount(attribute.type());
            auto component_size = getDataTypeSize(base_type) * component_count;

            // matrices take up one location per column
            // arrays take up one location per index
            GLuint location_count = getDataTypeColumnCount(attribute.type()) * attribute.count();

            for (GLuint offset = 0; offset < location_count; offset++)
                glEnableVertexAttribArray(attribute.location() + offset);

            switch (base_type) {
            case DataType::Float:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribPointer(
                        attribute.location() + offset,
                        component_count,
                        static_cast<GLenum>(base_type),
                        GL_FALSE,
                        program().attributeStride(),
                        reinterpret_cast<void*>(static_cast<std::uintptr_t>(offset)* component_size + attribute.offset()));
                break;

            case DataType::Double:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribLPointer(
                        attribute.location() + offset,
                        component_count,
                        static_cast<GLenum>(base_type),
                        program().attributeStride(),
                        reinterpret_cast<void*>(static_cast<std::uintptr_t>(offset)* component_size + attribute.offset()));
                break;

            case DataType::Bool:
            case DataType::Int:
            case DataType::UInt:
                for (GLuint offset = 0; offset < location_count; offset++)
                    glVertexAttribIPointer(
                        attribute.location() + offset,
                        component_count,
                        static_cast<GLenum>(base_type),
                        program().attributeStride(),
                        reinterpret_cast<void*>(static_cast<std::uintptr_t>(offset)* component_size + attribute.offset()));
            }
        }
    }

    VBO<T>& vbo_;
};

}
