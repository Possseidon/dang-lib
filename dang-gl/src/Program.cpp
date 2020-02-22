#include "pch.h"
#include "Program.h"

#include "dang-math/vector.h"
#include "dang-math/matrix.h"

#include "Types.h"

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

void Program::setAttributeOrder(const std::vector<std::string>& attribute_order)
{
    attribute_stride_ = 0;
    for (auto& name : attribute_order) {
        auto pos = attributes_.find(name);
        if (pos == attributes_.end())
            throw ShaderAttributeError("Shader-Attribute missing or optimized: " + name);
        auto& attribute = pos->second;
        attribute.offset_ = attribute_stride_;
        attribute_order_.push_back(attribute);
        attribute_stride_ += attribute.size();
    }

    for (auto& [name, attribute] : attributes_)
        if (attribute.offset_ == -1)
            throw ShaderAttributeError("Shader-Attribute not specified in order: " + name);
}

void Program::addShader(ShaderType type, const std::string& shader_code)
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

void Program::link(const std::vector<std::string>& attribute_order)
{
    glLinkProgram(handle());
    checkLinkStatusAndInfoLog();
    for (GLuint shader_handle : shader_handles_) {
        glDetachShader(handle(), shader_handle);
        glDeleteShader(shader_handle);
    }
    shader_handles_.clear();
    loadAttributeLocations();
    loadUniformLocations();
    setAttributeOrder(attribute_order);
}

GLsizei Program::attributeStride() const
{
    return attribute_stride_;
}

const std::vector<std::reference_wrapper<ShaderAttribute>>& Program::attributeOrder() const
{
    return attribute_order_;
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

GLsizei ShaderVariable::size() const
{
    return count_ * getDataTypeSize(type_);
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
    case DataType::Float:
        return std::make_unique<ShaderUniform<GLfloat>>(program, count, type, name);
    case DataType::Vec2:
        return std::make_unique<ShaderUniform<dgl::vec2>>(program, count, type, name);
    case DataType::Vec3:
        return std::make_unique<ShaderUniform<dgl::vec3>>(program, count, type, name);
    case DataType::Vec4:
        return std::make_unique<ShaderUniform<dgl::vec4>>(program, count, type, name);

    case DataType::Double:
        return std::make_unique<ShaderUniform<GLdouble>>(program, count, type, name);
    case DataType::DVec2:
        return std::make_unique<ShaderUniform<dgl::dvec2>>(program, count, type, name);
    case DataType::DVec3:
        return std::make_unique<ShaderUniform<dgl::dvec3>>(program, count, type, name);
    case DataType::DVec4:
        return std::make_unique<ShaderUniform<dgl::dvec4>>(program, count, type, name);

    case DataType::Int:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case DataType::IVec2:
        return std::make_unique<ShaderUniform<dgl::ivec2>>(program, count, type, name);
    case DataType::IVec3:
        return std::make_unique<ShaderUniform<dgl::ivec3>>(program, count, type, name);
    case DataType::IVec4:
        return std::make_unique<ShaderUniform<dgl::ivec4>>(program, count, type, name);

    case DataType::UInt:
        return std::make_unique<ShaderUniform<GLuint>>(program, count, type, name);
    case DataType::UVec2:
        return std::make_unique<ShaderUniform<dgl::uvec2>>(program, count, type, name);
    case DataType::UVec3:
        return std::make_unique<ShaderUniform<dgl::uvec3>>(program, count, type, name);
    case DataType::UVec4:
        return std::make_unique<ShaderUniform<dgl::uvec4>>(program, count, type, name);

    case DataType::Bool:
        return std::make_unique<ShaderUniform<GLboolean>>(program, count, type, name);
    case DataType::BVec2:
        return std::make_unique<ShaderUniform<dgl::bvec2>>(program, count, type, name);
    case DataType::BVec3:
        return std::make_unique<ShaderUniform<dgl::bvec3>>(program, count, type, name);
    case DataType::BVec4:
        return std::make_unique<ShaderUniform<dgl::bvec4>>(program, count, type, name);

    case DataType::Mat2:
        return std::make_unique<ShaderUniform<dgl::mat2>>(program, count, type, name);
    case DataType::Mat3:
        return std::make_unique<ShaderUniform<dgl::mat3>>(program, count, type, name);
    case DataType::Mat4:
        return std::make_unique<ShaderUniform<dgl::mat4>>(program, count, type, name);
    case DataType::Mat2x3:
        return std::make_unique<ShaderUniform<dgl::mat2x3>>(program, count, type, name);
    case DataType::Mat2x4:
        return std::make_unique<ShaderUniform<dgl::mat2x4>>(program, count, type, name);
    case DataType::Mat3x2:
        return std::make_unique<ShaderUniform<dgl::mat3x2>>(program, count, type, name);
    case DataType::Mat3x4:
        return std::make_unique<ShaderUniform<dgl::mat3x4>>(program, count, type, name);
    case DataType::Mat4x2:
        return std::make_unique<ShaderUniform<dgl::mat4x2>>(program, count, type, name);
    case DataType::Mat4x3:
        return std::make_unique<ShaderUniform<dgl::mat4x3>>(program, count, type, name);

    case DataType::DMat2:
        return std::make_unique<ShaderUniform<dgl::dmat2>>(program, count, type, name);
    case DataType::DMat3:
        return std::make_unique<ShaderUniform<dgl::dmat3>>(program, count, type, name);
    case DataType::DMat4:
        return std::make_unique<ShaderUniform<dgl::dmat4>>(program, count, type, name);
    case DataType::DMat2x3:
        return std::make_unique<ShaderUniform<dgl::dmat2x3>>(program, count, type, name);
    case DataType::DMat2x4:
        return std::make_unique<ShaderUniform<dgl::dmat2x4>>(program, count, type, name);
    case DataType::DMat3x2:
        return std::make_unique<ShaderUniform<dgl::dmat3x2>>(program, count, type, name);
    case DataType::DMat3x4:
        return std::make_unique<ShaderUniform<dgl::dmat3x4>>(program, count, type, name);
    case DataType::DMat4x2:
        return std::make_unique<ShaderUniform<dgl::dmat4x2>>(program, count, type, name);
    case DataType::DMat4x3:
        return std::make_unique<ShaderUniform<dgl::dmat4x3>>(program, count, type, name);

    case DataType::Sampler1D:
    case DataType::Sampler2D:
    case DataType::Sampler3D:
    case DataType::SamplerCube:
    case DataType::Sampler1DShadow:
    case DataType::Sampler2DShadow:
    case DataType::Sampler1DArray:
    case DataType::Sampler2DArray:
    case DataType::Sampler1DArrayShadow:
    case DataType::Sampler2DArrayShadow:
    case DataType::Sampler2DMS:
    case DataType::Sampler2DMSArray:
    case DataType::SamplerCubeShadow:
    case DataType::SamplerBuffer:
    case DataType::Sampler2DRect:
    case DataType::Sampler2DRectShadow:
    case DataType::ISampler1D:
    case DataType::ISampler2D:
    case DataType::ISampler3D:
    case DataType::ISamplerCube:
    case DataType::ISampler1DArray:
    case DataType::ISampler2DArray:
    case DataType::ISampler2DMS:
    case DataType::ISampler2DMSArray:
    case DataType::ISamplerBuffer:
    case DataType::ISampler2DRect:
    case DataType::USampler1D:
    case DataType::USampler2D:
    case DataType::USampler3D:
    case DataType::USamplerCube:
    case DataType::USampler1DArray:
    case DataType::USampler2DArray:
    case DataType::USampler2DMS:
    case DataType::USampler2DMSArray:
    case DataType::USamplerBuffer:
    case DataType::USampler2DRect:
    case DataType::Image1D:
    case DataType::Image2D:
    case DataType::Image3D:
    case DataType::Image2DRect:
    case DataType::ImageCube:
    case DataType::ImageBuffer:
    case DataType::Image1DArray:
    case DataType::Image2DArray:
    case DataType::Image2DMS:
    case DataType::Image2DMSArray:
    case DataType::IImage1D:
    case DataType::IImage2D:
    case DataType::IImage3D:
    case DataType::IImage2DRect:
    case DataType::IImageCube:
    case DataType::IImageBuffer:
    case DataType::IImage1DArray:
    case DataType::IImage2DArray:
    case DataType::IImage2DMS:
    case DataType::IImage2DMSArray:
    case DataType::UImage1D:
    case DataType::UImage2D:
    case DataType::UImage3D:
    case DataType::UImage2DRect:
    case DataType::UImageCube:
    case DataType::UImageBuffer:
    case DataType::UImage1DArray:
    case DataType::UImage2DArray:
    case DataType::UImage2DMS:
    case DataType::UImage2DMSArray:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);

    case DataType::AtomicUInt:
        return std::make_unique<ShaderUniform<GLuint>>(program, count, type, name);
    }
    return std::make_unique<ShaderUniformBase>(program, count, type, name);
}

ShaderAttribute::ShaderAttribute(Program& program, GLint count, DataType type, std::string name)
    : ShaderVariable(program, count, type, name, glGetAttribLocation(program.handle(), name.c_str()))
{
}

GLsizei ShaderAttribute::offset() const
{
    return offset_;
}

}
