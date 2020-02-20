#pragma once

#include "Object.h"

#include "VBO.h"

namespace dang::gl
{

class Program;

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

    static constexpr ObjectType Type = ObjectType::VertexArray;
};

class VAOBase : public Object<VAOInfo> {
public:
    VAOBase(Program& program, BeginMode mode = BeginMode::Triangles);

    Program& program() const;

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
    }

private:
    VBO<T>& vbo_;
};

}
