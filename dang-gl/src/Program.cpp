#include "pch.h"

#include <iostream>

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

void ProgramInfo::bind(GLuint handle)
{
    glUseProgram(handle);
}

void Program::checkShaderStatusAndInfoLog(GLuint shader_handle, ShaderType type)
{
    GLint status;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &status);

    GLint info_log_length;
    glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::string info_log(info_log_length - 1, '\0');
        glGetShaderInfoLog(shader_handle, info_log_length, nullptr, &info_log[0]);

        if (status)
            std::cerr << info_log;
        else
            throw ShaderCompilationError(type, info_log);
    }
    else if (!status) {
        throw ShaderCompilationError(type, "unknown error");
    }
}

void Program::checkLinkStatusAndInfoLog()
{
    GLint status;
    glGetProgramiv(handle(), GL_LINK_STATUS, &status);

    GLint info_log_length;
    glGetProgramiv(handle(), GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::string info_log(info_log_length - 1, '\0');
        glGetProgramInfoLog(handle(), info_log_length, nullptr, &info_log[0]);

        if (status)
            std::cerr << info_log;
        else
            throw ShaderLinkError(info_log);
    }
    else if (!status) {
        throw ShaderLinkError("unknown error");
    }
}

void Program::addShader(ShaderType type, std::string shader_code)
{
    GLuint shader_handle = glCreateShader(ShaderTypesGL[type]);
    shader_handles_.push_back(shader_handle);

    std::vector<const GLchar*> full_code;
    full_code.push_back(shader_code.c_str());

    glShaderSource(shader_handle, full_code.size(), &full_code[0], nullptr);
    glCompileShader(shader_handle);
    checkShaderStatusAndInfoLog(shader_handle, type);
    glAttachShader(handle(), shader_handle);
}

void Program::link()
{
    glLinkProgram(handle());
    checkLinkStatusAndInfoLog();
    for (GLuint shader_handle : shader_handles_) {
        glDetachShader(handle(), shader_handle);
        glDeleteShader(shader_handle);
    }
}

}
