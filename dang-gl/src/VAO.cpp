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

}
