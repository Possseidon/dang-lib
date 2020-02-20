#include "pch.h"
#include "VAO.h"

namespace dang::gl
{

GLuint VAOInfo::create()
{
    GLuint handle;
    glGenVertexArrays(1, &handle);
    return handle;
}

void VAOInfo::destroy(GLuint handle)
{
    glDeleteVertexArrays(1, &handle);
}

void VAOInfo::bind(GLuint handle)
{
    glBindVertexArray(handle);
}

VAOBase::VAOBase(Program& program, BeginMode mode)
    : program_(program)
    , mode_(mode)
{
}

Program& VAOBase::program() const
{
    return program_;
}

}
