#include "pch.h"

#include "Program.h"

namespace dang::gl
{

GLuint ProgramInfo::create()
{
    return glCreateProgram();
}

void ProgramInfo::destroy(GLuint handle)
{
    return glDeleteProgram(handle);
}

}
