#include "pch.h"
#include "Program.h"

#include "Types.h"

namespace dang::gl
{

GLuint ProgramInfo::create()
{
    return glCreateProgram();
}

void ProgramInfo::destroy(GLuint handle)
{
    glDeleteProgram(handle);
}

void ProgramInfo::bind(GLuint handle)
{
    glUseProgram(handle);
}

std::string Program::replaceInfoLogShaderNames(std::string info_log) const
{
    std::vector<std::string> names{ "main" };
    for (const auto& [name, code] : includes_)
        names.push_back(name);

    for (std::size_t i = 0; i < names.size(); i++) {
        const std::array line_regexes{
            // NVIDIA: 1(23)
            std::regex("\\b" + std::to_string(i) + "\\((\\d+)\\)\\b"),
            // Intel: 1:23
            std::regex("\\b" + std::to_string(i) + ":(\\d+)\\b")
        };

        for (const auto& line_regex : line_regexes)
            info_log = std::regex_replace(info_log, line_regex, names[i] + "($1)");
    }

    return info_log;
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

        info_log = replaceInfoLogShaderNames(info_log);

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

void Program::addInclude(const std::string& name, std::string code)
{
    includes_.emplace(name, std::move(code));
}

void Program::addIncludeFromFile(const fs::path& path)
{
    addIncludeFromFile(path, path.filename().string());
}

void Program::addIncludeFromFile(const fs::path& path, const std::string& name)
{
    std::ifstream file_stream(path);
    if (!file_stream)
        throw ShaderFileNotFound(path);
    std::ostringstream string_stream;
    string_stream << file_stream.rdbuf();
    addInclude(name, string_stream.str());
}

void Program::addShader(ShaderType type, const std::string& shader_code)
{
    GLuint shader_handle = glCreateShader(ShaderTypesGL[type]);
    shader_handles_.push_back(shader_handle);

    std::string preprocessed = ShaderPreprocessor(*this, shader_code);
    const GLchar* full_code = preprocessed.c_str();

    glShaderSource(shader_handle, 1, &full_code, nullptr);
    glCompileShader(shader_handle);
    checkShaderStatusAndInfoLog(shader_handle, type);
    glAttachShader(handle(), shader_handle);
}

void Program::addShaderFromFile(ShaderType type, const fs::path& path)
{
    std::ifstream file_stream(path);
    if (!file_stream)
        throw ShaderFileNotFound(path);
    std::ostringstream string_stream;
    string_stream << file_stream.rdbuf();
    addShader(type, string_stream.str());
}

void Program::link(const std::vector<std::string>& attribute_order)
{
    glLinkProgram(handle());
    checkLinkStatusAndInfoLog();
    postLinkCleanup();
    loadAttributeLocations();
    loadUniformLocations();
    setAttributeOrder(attribute_order);
}

void Program::postLinkCleanup()
{
    for (GLuint shader_handle : shader_handles_) {
        glDetachShader(handle(), shader_handle);
        glDeleteShader(shader_handle);
    }
    shader_handles_.clear();
    includes_.clear();
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
        return std::make_unique<ShaderUniform<vec2>>(program, count, type, name);
    case DataType::Vec3:
        return std::make_unique<ShaderUniform<vec3>>(program, count, type, name);
    case DataType::Vec4:
        return std::make_unique<ShaderUniform<vec4>>(program, count, type, name);

    case DataType::Double:
        return std::make_unique<ShaderUniform<GLdouble>>(program, count, type, name);
    case DataType::DVec2:
        return std::make_unique<ShaderUniform<dvec2>>(program, count, type, name);
    case DataType::DVec3:
        return std::make_unique<ShaderUniform<dvec3>>(program, count, type, name);
    case DataType::DVec4:
        return std::make_unique<ShaderUniform<dvec4>>(program, count, type, name);

    case DataType::Int:
        return std::make_unique<ShaderUniform<GLint>>(program, count, type, name);
    case DataType::IVec2:
        return std::make_unique<ShaderUniform<ivec2>>(program, count, type, name);
    case DataType::IVec3:
        return std::make_unique<ShaderUniform<ivec3>>(program, count, type, name);
    case DataType::IVec4:
        return std::make_unique<ShaderUniform<ivec4>>(program, count, type, name);

    case DataType::UInt:
        return std::make_unique<ShaderUniform<GLuint>>(program, count, type, name);
    case DataType::UVec2:
        return std::make_unique<ShaderUniform<uvec2>>(program, count, type, name);
    case DataType::UVec3:
        return std::make_unique<ShaderUniform<uvec3>>(program, count, type, name);
    case DataType::UVec4:
        return std::make_unique<ShaderUniform<uvec4>>(program, count, type, name);

    case DataType::Bool:
        return std::make_unique<ShaderUniform<GLboolean>>(program, count, type, name);
    case DataType::BVec2:
        return std::make_unique<ShaderUniform<bvec2>>(program, count, type, name);
    case DataType::BVec3:
        return std::make_unique<ShaderUniform<bvec3>>(program, count, type, name);
    case DataType::BVec4:
        return std::make_unique<ShaderUniform<bvec4>>(program, count, type, name);

    case DataType::Mat2:
        return std::make_unique<ShaderUniform<mat2>>(program, count, type, name);
    case DataType::Mat3:
        return std::make_unique<ShaderUniform<mat3>>(program, count, type, name);
    case DataType::Mat4:
        return std::make_unique<ShaderUniform<mat4>>(program, count, type, name);
    case DataType::Mat2x3:
        return std::make_unique<ShaderUniform<mat2x3>>(program, count, type, name);
    case DataType::Mat2x4:
        return std::make_unique<ShaderUniform<mat2x4>>(program, count, type, name);
    case DataType::Mat3x2:
        return std::make_unique<ShaderUniform<mat3x2>>(program, count, type, name);
    case DataType::Mat3x4:
        return std::make_unique<ShaderUniform<mat3x4>>(program, count, type, name);
    case DataType::Mat4x2:
        return std::make_unique<ShaderUniform<mat4x2>>(program, count, type, name);
    case DataType::Mat4x3:
        return std::make_unique<ShaderUniform<mat4x3>>(program, count, type, name);

    case DataType::DMat2:
        return std::make_unique<ShaderUniform<dmat2>>(program, count, type, name);
    case DataType::DMat3:
        return std::make_unique<ShaderUniform<dmat3>>(program, count, type, name);
    case DataType::DMat4:
        return std::make_unique<ShaderUniform<dmat4>>(program, count, type, name);
    case DataType::DMat2x3:
        return std::make_unique<ShaderUniform<dmat2x3>>(program, count, type, name);
    case DataType::DMat2x4:
        return std::make_unique<ShaderUniform<dmat2x4>>(program, count, type, name);
    case DataType::DMat3x2:
        return std::make_unique<ShaderUniform<dmat3x2>>(program, count, type, name);
    case DataType::DMat3x4:
        return std::make_unique<ShaderUniform<dmat3x4>>(program, count, type, name);
    case DataType::DMat4x2:
        return std::make_unique<ShaderUniform<dmat4x2>>(program, count, type, name);
    case DataType::DMat4x3:
        return std::make_unique<ShaderUniform<dmat4x3>>(program, count, type, name);

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

ShaderPreprocessor::ShaderPreprocessor(const Program& program, const std::string& code)
    : program_(program)
{
    process(code, 0);
}

ShaderPreprocessor::operator std::string() const
{
    return output_.str();
}

void ShaderPreprocessor::process(const std::string& code, std::size_t compilation_unit)
{
    static const std::regex include_regex("^ *# *include *\"([A-Za-z0-9_.\\\\/ ]+)\" *$");

    std::string read_line;
    std::string full_line;
    std::vector<std::string> line_buffer;
    std::istringstream input(code);
    std::size_t line_index = 0;
    bool block_comment = false;

    while (std::getline(input, read_line)) {
        line_index++;
        line_buffer.push_back(read_line);

        if (!read_line.empty() && read_line.back() == '\\') {
            read_line.pop_back();
            full_line += read_line;
            continue;
        }

        full_line += read_line;

        std::size_t find_offset = 0;
        while (true) {
            if (block_comment) {
                std::size_t block_comment_end = full_line.find("*/", find_offset);
                if (block_comment_end == std::string::npos)
                    break;

                block_comment = false;
                find_offset = block_comment_end + 2;
            }
            else {
                std::size_t first_slash_pos = full_line.find("/", find_offset);
                if (first_slash_pos == std::string::npos)
                    break;

                std::size_t line_comment_pos = full_line.find("//", first_slash_pos);
                std::size_t block_comment_pos = full_line.find("/*", first_slash_pos);
                if (block_comment_pos == std::string::npos)
                    break;

                if (line_comment_pos != std::string::npos && line_comment_pos < block_comment_pos)
                    break;

                block_comment = true;
                find_offset = block_comment_pos + 1;
            }
        }

        std::smatch match;
        if (!block_comment && std::regex_match(full_line, match, include_regex)) {
            std::string name = match[1];

            if (included_.find(name) != included_.end()) {
                next_line_ = { line_index + 1, compilation_unit };
                continue;
            }
            included_.insert(name);

            auto pos = program_.includes_.find(name);
            if (pos == program_.includes_.end()) {
                output_ << "#error missing include: \"" << name << "\"\n";
                continue;
            }

            std::size_t include_compilation_unit = std::distance(program_.includes_.begin(), pos) + 1;
            next_line_ = { 1, include_compilation_unit };
            process(pos->second, include_compilation_unit);
            next_line_ = { line_index + 1, compilation_unit };
        }
        else {
            if (next_line_) {
                auto [next_line, next_compilation_unit] = *next_line_;
                output_ << "#line " << next_line << " " << next_compilation_unit << "\n";
                next_line_.reset();
            }

            for (const std::string& line : line_buffer)
                output_ << line << "\n";
        }

        line_buffer.clear();
        full_line.clear();
    }
}

}
