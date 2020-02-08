#pragma once

#include "dang-utils/enum.h"

#include "GLFW.h"

namespace dang::gl
{

enum class ShaderType {
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEvaluation,
    Compute,
    COUNT
};
      
constexpr dutils::EnumArray<ShaderType, GLenum> ShaderTypesGL
{
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_COMPUTE_SHADER
};

const dutils::EnumArray<ShaderType, std::string> ShaderTypeNames
{
    "Vertex-Shader",
    "Fragment-Shader",
    "Geometry-Shader",
    "Tesselation-Control-Shader",
    "Tesselation-Evaluation-Shader",
    "Compute-Shader"
};

class ShaderError : public std::runtime_error {
public:
    ShaderError(const std::string& info_log) : std::runtime_error(info_log) {}
};

class ShaderCompilationError : public ShaderError {
public:
    ShaderCompilationError(ShaderType type, const std::string& info_log) : ShaderError(info_log), type_(type) {}

    ShaderType type() { return type_; }

private:
    ShaderType type_;
};

class ShaderLinkError : public ShaderError {
public:
    ShaderLinkError() = default;
    ShaderLinkError(const std::string& info_log) : ShaderError(info_log) {}
};

struct ProgramInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr ObjectType Type = ObjectType::Program;
};

class Program : public Object<ProgramInfo> {
public:
    void checkShaderStatusAndInfoLog(GLuint shader_handle, ShaderType type);
    void checkLinkStatusAndInfoLog();
    void addShader(ShaderType type, std::string shader_code);

    void link();

private:
    std::vector<GLuint> shader_handles_;
};

}
