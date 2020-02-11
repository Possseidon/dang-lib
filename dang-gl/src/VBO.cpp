#include "pch.h"
#include "VBO.h"

namespace dang::gl
{

GLuint VBOInfo::create()
{
    GLuint handle;
    glGenBuffers(1, &handle);
    return handle;
}

void VBOInfo::destroy(GLuint handle)
{
    glDeleteBuffers(1, &handle);
}

void VBOInfo::bind(GLuint handle)
{
    glBindBuffer(GL_ARRAY_BUFFER, handle);
}

}
