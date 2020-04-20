#pragma once

#include "Object.h"
#include "Program.h"
#include "VBO.h"

namespace dang::gl
{

/// <summary>A list of all supported modes on how to draw vertex data.</summary>
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

/// <summary>Info struct to create, destroy and bind VAOs.</summary>
struct VAOInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr ObjectType ObjectType = ObjectType::VertexArray;
    static constexpr BindingPoint BindingPoint = BindingPoint::VertexArray;
};

/// <summary>A base class for all vertex array objects, which is not templated yet.</summary>
class VAOBase : public Object<VAOInfo> {
public:
    /// <summary>Initializes the VAO base with the given GL-Program and optional render mode, which defaults to the most commonly used "triangles" mode.</summary>
    VAOBase(Program& program, BeginMode mode = BeginMode::Triangles);

    /// <summary>The GL-Program associated with the VAO.</summary>
    Program& program() const;

    /// <summary>Returns the current render mode, which is used in draw calls.</summary>
    BeginMode mode() const;
    /// <summary>Although not always senseful, allows to modify the render mode after construction.</summary>
    /// <remarks>Different render modes require very different data layouts, often making it impossible to use the same data with different modes.</remarks>
    void setMode(BeginMode mode);

private:
    Program& program_;
    BeginMode mode_;
};

/// <summary>A vertex array object, which basically combines a VBO with a GL-Program, making it drawable.</summary>
template <typename T>
class VAO : public VAOBase {
public:
    /// <summary>Creates a new VAO and binds it to the given GL-Program and VBO.</summary>
    /// <remarks>A debug assertion checks, that the size of the Data struct matches the program stride.</remarks>
    VAO(Program& program, VBO<T>& vbo, BeginMode mode = BeginMode::Triangles)
        : VAOBase(program, mode)
        , vbo_(vbo)
    {
        assert(program.attributeStride() == sizeof(T));
        enableAttributes();
    }

    /// <summary>Draws the full content off the associated VBO using the likewise associated GL-Program.</summary>
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
        for (const ShaderAttribute& attribute : program().attributeOrder()) {
            const auto component_count = getDataTypeComponentCount(attribute.type());
            const auto base_type = getBaseDataType(attribute.type());
            const auto component_size = component_count * getDataTypeSize(base_type);

            const auto index = [&attribute](GLuint offset) -> GLuint { return attribute.location() + offset; };
            const GLint size = component_count;
            const GLenum type = static_cast<GLenum>(base_type);
            constexpr GLboolean normalized = GL_FALSE;
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
