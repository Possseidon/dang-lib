#include "pch.h"
#include "Program.h"

#include "dang-math/vector.h"
#include "dang-math/matrix.h"

#include <iostream>

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
        std::string info_log(static_cast<std::size_t>(info_log_length) - 1, '\0');
        glGetShaderInfoLog(shader_handle, info_log_length, nullptr, &info_log[0]);

        if (status)
            std::cerr << info_log;
        else
            throw ShaderCompilationError(type, info_log);
    }
    else if (!status)
        throw ShaderCompilationError(type, "unknown error");
}

void Program::checkLinkStatusAndInfoLog()
{
    GLint status;
    glGetProgramiv(handle(), GL_LINK_STATUS, &status);

    GLint info_log_length;
    glGetProgramiv(handle(), GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::string info_log(static_cast<std::size_t>(info_log_length) - 1, '\0');
        glGetProgramInfoLog(handle(), info_log_length, nullptr, &info_log[0]);

        if (status)
            std::cerr << info_log;
        else
            throw ShaderLinkError(info_log);
    }
    else if (!status)
        throw ShaderLinkError("unknown error");
}

void Program::loadAttributeLocations()
{
    GLint active_attributes;
    glGetProgramiv(handle(), GL_ACTIVE_ATTRIBUTES, &active_attributes);
    if (active_attributes == 0)
        return;

    GLint max_length;
    glGetProgramiv(handle(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_length);
    for (GLint i = 0; i < active_attributes; i++) {
        std::string name(static_cast<std::size_t>(max_length) - 1, '\0');
        GLsizei actual_length;
        GLint data_size;
        GLenum data_type;
        glGetActiveAttrib(handle(), static_cast<GLuint>(i), max_length, &actual_length, &data_size, &data_type, &name[0]);
        name.resize(static_cast<std::size_t>(actual_length));
        attributes_.emplace(name, ShaderAttribute(*this, data_size, static_cast<DataType>(data_type), name));
    }
}

void Program::loadUniformLocations()
{
    GLint active_uniforms;
    glGetProgramiv(handle(), GL_ACTIVE_UNIFORMS, &active_uniforms);
    if (active_uniforms == 0)
        return;

    GLint max_length;
    glGetProgramiv(handle(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
    for (GLint i = 0; i < active_uniforms; i++) {
        std::string name(static_cast<std::size_t>(max_length) - 1, '\0');
        GLsizei actual_length;
        GLint data_size;
        GLenum data_type;
        glGetActiveUniform(handle(), static_cast<GLuint>(i), max_length, &actual_length, &data_size, &data_type, &name[0]);
        name.resize(static_cast<std::size_t>(actual_length));

        uniforms_.emplace(name, ShaderUniformBase::create(*this, data_size, static_cast<DataType>(data_type), name));
    }
}

void Program::addShader(ShaderType type, std::string shader_code)
{
    GLuint shader_handle = glCreateShader(ShaderTypesGL[type]);
    shader_handles_.push_back(shader_handle);

    std::vector<const GLchar*> full_code;
    full_code.push_back(shader_code.c_str());

    glShaderSource(shader_handle, static_cast<GLsizei>(full_code.size()), &full_code[0], nullptr);
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
    loadAttributeLocations();
    loadUniformLocations();
}

ShaderVariable::ShaderVariable(Program& program, GLint count, DataType type, std::string name, GLint location)
    : program_(program)
    , count_(count)
    , type_(type)
    , name_(std::move(name))
    , location_(location)
{
}

Program& ShaderVariable::program() const
{
    return program_;
}

GLint ShaderVariable::count() const
{
    return count_;
}

DataType ShaderVariable::type() const
{
    return type_;
}

const std::string& ShaderVariable::name() const
{
    return name_;
}

GLint ShaderVariable::location() const
{
    return location_;
}

ShaderUniformBase::ShaderUniformBase(Program& program, GLint count, DataType type, std::string name)
    : ShaderVariable(program, count, type, name, glGetUniformLocation(program.handle(), name.c_str()))
{
}

std::unique_ptr<ShaderUniformBase> ShaderUniformBase::create(Program& program, GLint count, DataType type, std::string name)
{
    switch (type) {
    case dang::gl::DataType::Float:
        return std::make_unique<ShaderUniform<GLfloat>>(program, count, type, name);
    case dang::gl::DataType::Vec2:
        return std::make_unique<ShaderUniform<dmath::vec2>>(program, count, type, name);
    case dang::gl::DataType::Vec3:
        return std::make_unique<ShaderUniform<dmath::vec3>>(program, count, type, name);
    case dang::gl::DataType::Vec4:
        return std::make_unique<ShaderUniform<dmath::vec4>>(program, count, type, name);
    case dang::gl::DataType::Double:
        return std::make_unique<ShaderUniform<GLdouble>>(program, count, type, name);
    case dang::gl::DataType::DVec2:
        return std::make_unique<ShaderUniform<dmath::dvec2>>(program, count, type, name);
    case dang::gl::DataType::DVec3:
        return std::make_unique<ShaderUniform<dmath::dvec3>>(program, count, type, name);
    case dang::gl::DataType::DVec4:
        return std::make_unique<ShaderUniform<dmath::dvec4>>(program, count, type, name);
    case dang::gl::DataType::Int:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IVec2:
        return std::make_unique<ShaderUniform<dmath::ivec2>>(program, count, type, name);
    case dang::gl::DataType::IVec3:
        return std::make_unique<ShaderUniform<dmath::ivec3>>(program, count, type, name);
    case dang::gl::DataType::IVec4:
        return std::make_unique<ShaderUniform<dmath::ivec4>>(program, count, type, name);
    case dang::gl::DataType::UInt:
        return std::make_unique<ShaderUniform<GLuint>>(program, count, type, name);
    case dang::gl::DataType::UVec2:
        return std::make_unique<ShaderUniform<dmath::uvec2>>(program, count, type, name);
    case dang::gl::DataType::UVec3:
        return std::make_unique<ShaderUniform<dmath::uvec3>>(program, count, type, name);
    case dang::gl::DataType::UVec4:
        return std::make_unique<ShaderUniform<dmath::uvec4>>(program, count, type, name);
    case dang::gl::DataType::Bool:
        return std::make_unique<ShaderUniform<bool>>(program, count, type, name);
    case dang::gl::DataType::BVec2:
        return std::make_unique<ShaderUniform<dmath::bvec2>>(program, count, type, name);
    case dang::gl::DataType::BVec3:
        return std::make_unique<ShaderUniform<dmath::bvec3>>(program, count, type, name);
    case dang::gl::DataType::BVec4:
        return std::make_unique<ShaderUniform<dmath::bvec4>>(program, count, type, name);
    case dang::gl::DataType::Mat2:
        return std::make_unique<ShaderUniform<dmath::mat2>>(program, count, type, name);
    case dang::gl::DataType::Mat3:
        return std::make_unique<ShaderUniform<dmath::mat3>>(program, count, type, name);
    case dang::gl::DataType::Mat4:
        return std::make_unique<ShaderUniform<dmath::mat4>>(program, count, type, name);
    case dang::gl::DataType::Mat2x3:
        return std::make_unique<ShaderUniform<dmath::mat2x3>>(program, count, type, name);
    case dang::gl::DataType::Mat2x4:
        return std::make_unique<ShaderUniform<dmath::mat2x4>>(program, count, type, name);
    case dang::gl::DataType::Mat3x2:
        return std::make_unique<ShaderUniform<dmath::mat3x2>>(program, count, type, name);
    case dang::gl::DataType::Mat3x4:
        return std::make_unique<ShaderUniform<dmath::mat3x4>>(program, count, type, name);
    case dang::gl::DataType::Mat4x2:
        return std::make_unique<ShaderUniform<dmath::mat4x2>>(program, count, type, name);
    case dang::gl::DataType::Mat4x3:
        return std::make_unique<ShaderUniform<dmath::mat4x3>>(program, count, type, name);
    case dang::gl::DataType::DMat2:
        return std::make_unique<ShaderUniform<dmath::dmat2>>(program, count, type, name);
    case dang::gl::DataType::DMat3:
        return std::make_unique<ShaderUniform<dmath::dmat3>>(program, count, type, name);
    case dang::gl::DataType::DMat4:
        return std::make_unique<ShaderUniform<dmath::dmat4>>(program, count, type, name);
    case dang::gl::DataType::DMat2x3:
        return std::make_unique<ShaderUniform<dmath::dmat2x3>>(program, count, type, name);
    case dang::gl::DataType::DMat2x4:
        return std::make_unique<ShaderUniform<dmath::dmat2x4>>(program, count, type, name);
    case dang::gl::DataType::DMat3x2:
        return std::make_unique<ShaderUniform<dmath::dmat3x2>>(program, count, type, name);
    case dang::gl::DataType::DMat3x4:
        return std::make_unique<ShaderUniform<dmath::dmat3x4>>(program, count, type, name);
    case dang::gl::DataType::DMat4x2:
        return std::make_unique<ShaderUniform<dmath::dmat4x2>>(program, count, type, name);
    case dang::gl::DataType::DMat4x3:
        return std::make_unique<ShaderUniform<dmath::dmat4x3>>(program, count, type, name);
    case dang::gl::DataType::Sampler1D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler3D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::SamplerCube:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler1DShadow:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2DShadow:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler1DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler1DArrayShadow:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2DArrayShadow:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2DMS:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2DMSArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::SamplerCubeShadow:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::SamplerBuffer:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2DRect:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Sampler2DRectShadow:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler1D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler2D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler3D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISamplerCube:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler1DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler2DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler2DMS:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler2DMSArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISamplerBuffer:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ISampler2DRect:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler1D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler2D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler3D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USamplerCube:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler1DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler2DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler2DMS:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler2DMSArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USamplerBuffer:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::USampler2DRect:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image1D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image2D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image3D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image2DRect:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ImageCube:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::ImageBuffer:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image1DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image2DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image2DMS:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::Image2DMSArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage1D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage2D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage3D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage2DRect:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImageCube:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImageBuffer:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage1DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage2DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage2DMS:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::IImage2DMSArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage1D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage2D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage3D:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage2DRect:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImageCube:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImageBuffer:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage1DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage2DArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage2DMS:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::UImage2DMSArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case dang::gl::DataType::AtomicUInt:
        return std::make_unique<ShaderUniform<GLuint>>(program, count, type, name);
    }
    return std::make_unique<ShaderUniformBase>(program, count, type, name);
}

ShaderAttribute::ShaderAttribute(Program& program, GLint count, DataType type, std::string name)
    : ShaderVariable(program, count, type, std::move(name), glGetAttribLocation(program.handle(), name.c_str()))
{
}

}
