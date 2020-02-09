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

enum class DataType {
    Float = GL_FLOAT,
    Vec2 = GL_FLOAT_VEC2,
    Vec3 = GL_FLOAT_VEC3,
    Vec4 = GL_FLOAT_VEC4,

    Double = GL_DOUBLE,
    DVec2 = GL_DOUBLE_VEC2,
    DVec3 = GL_DOUBLE_VEC3,
    DVec4 = GL_DOUBLE_VEC4,

    Int = GL_INT,
    IVec2 = GL_INT_VEC2,
    IVec3 = GL_INT_VEC3,
    IVec4 = GL_INT_VEC4,

    UInt = GL_UNSIGNED_INT,
    UVec2 = GL_UNSIGNED_INT_VEC2,
    UVec3 = GL_UNSIGNED_INT_VEC3,
    UVec4 = GL_UNSIGNED_INT_VEC4,

    Bool = GL_BOOL,
    BVec2 = GL_BOOL_VEC2,
    BVec3 = GL_BOOL_VEC3,
    BVec4 = GL_BOOL_VEC4,

    Mat2 = GL_FLOAT_MAT2,
    Mat3 = GL_FLOAT_MAT3,
    Mat4 = GL_FLOAT_MAT4,
    Mat2x3 = GL_FLOAT_MAT2x3,
    Mat2x4 = GL_FLOAT_MAT2x4,
    Mat3x2 = GL_FLOAT_MAT3x2,
    Mat3x4 = GL_FLOAT_MAT3x4,
    Mat4x2 = GL_FLOAT_MAT4x2,
    Mat4x3 = GL_FLOAT_MAT4x3,

    DMat2 = GL_DOUBLE_MAT2,
    DMat3 = GL_DOUBLE_MAT3,
    DMat4 = GL_DOUBLE_MAT4,
    DMat2x3 = GL_DOUBLE_MAT2x3,
    DMat2x4 = GL_DOUBLE_MAT2x4,
    DMat3x2 = GL_DOUBLE_MAT3x2,
    DMat3x4 = GL_DOUBLE_MAT3x4,
    DMat4x2 = GL_DOUBLE_MAT4x2,
    DMat4x3 = GL_DOUBLE_MAT4x3,

    Sampler1D = GL_SAMPLER_1D,
    Sampler2D = GL_SAMPLER_2D,
    Sampler3D = GL_SAMPLER_3D,
    SamplerCube = GL_SAMPLER_CUBE,
    Sampler1DShadow = GL_SAMPLER_1D_SHADOW,
    Sampler2DShadow = GL_SAMPLER_2D_SHADOW,
    Sampler1DArray = GL_SAMPLER_1D_ARRAY,
    Sampler2DArray = GL_SAMPLER_2D_ARRAY,
    Sampler1DArrayShadow = GL_SAMPLER_1D_ARRAY_SHADOW,
    Sampler2DArrayShadow = GL_SAMPLER_2D_ARRAY_SHADOW,
    Sampler2DMS = GL_SAMPLER_2D_MULTISAMPLE,
    Sampler2DMSArray = GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
    SamplerCubeShadow = GL_SAMPLER_CUBE_SHADOW,
    SamplerBuffer = GL_SAMPLER_BUFFER,
    Sampler2DRect = GL_SAMPLER_2D_RECT,
    Sampler2DRectShadow = GL_SAMPLER_2D_RECT_SHADOW,

    ISampler1D = GL_INT_SAMPLER_1D,
    ISampler2D = GL_INT_SAMPLER_2D,
    ISampler3D = GL_INT_SAMPLER_3D,
    ISamplerCube = GL_INT_SAMPLER_CUBE,
    ISampler1DArray = GL_INT_SAMPLER_1D_ARRAY,
    ISampler2DArray = GL_INT_SAMPLER_2D_ARRAY,
    ISampler2DMS = GL_INT_SAMPLER_2D_MULTISAMPLE,
    ISampler2DMSArray = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    ISamplerBuffer = GL_INT_SAMPLER_BUFFER,
    ISampler2DRect = GL_INT_SAMPLER_2D_RECT,

    USampler1D = GL_UNSIGNED_INT_SAMPLER_1D,
    USampler2D = GL_UNSIGNED_INT_SAMPLER_2D,
    USampler3D = GL_UNSIGNED_INT_SAMPLER_3D,
    USamplerCube = GL_UNSIGNED_INT_SAMPLER_CUBE,
    USampler1DArray = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,
    USampler2DArray = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
    USampler2DMS = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
    USampler2DMSArray = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
    USamplerBuffer = GL_UNSIGNED_INT_SAMPLER_BUFFER,
    USampler2DRect = GL_UNSIGNED_INT_SAMPLER_2D_RECT,

    Image1D = GL_IMAGE_1D,
    Image2D = GL_IMAGE_2D,
    Image3D = GL_IMAGE_3D,
    Image2DRect = GL_IMAGE_2D_RECT,
    ImageCube = GL_IMAGE_CUBE,
    ImageBuffer = GL_IMAGE_BUFFER,
    Image1DArray = GL_IMAGE_1D_ARRAY,
    Image2DArray = GL_IMAGE_2D_ARRAY,
    Image2DMS = GL_IMAGE_2D_MULTISAMPLE,
    Image2DMSArray = GL_IMAGE_2D_MULTISAMPLE_ARRAY,

    IImage1D = GL_INT_IMAGE_1D,
    IImage2D = GL_INT_IMAGE_2D,
    IImage3D = GL_INT_IMAGE_3D,
    IImage2DRect = GL_INT_IMAGE_2D_RECT,
    IImageCube = GL_INT_IMAGE_CUBE,
    IImageBuffer = GL_INT_IMAGE_BUFFER,
    IImage1DArray = GL_INT_IMAGE_1D_ARRAY,
    IImage2DArray = GL_INT_IMAGE_2D_ARRAY,
    IImage2DMS = GL_INT_IMAGE_2D_MULTISAMPLE,
    IImage2DMSArray = GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

    UImage1D = GL_UNSIGNED_INT_IMAGE_1D,
    UImage2D = GL_UNSIGNED_INT_IMAGE_2D,
    UImage3D = GL_UNSIGNED_INT_IMAGE_3D,
    UImage2DRect = GL_UNSIGNED_INT_IMAGE_2D_RECT,
    UImageCube = GL_UNSIGNED_INT_IMAGE_CUBE,
    UImageBuffer = GL_UNSIGNED_INT_IMAGE_BUFFER,
    UImage1DArray = GL_UNSIGNED_INT_IMAGE_1D_ARRAY,
    UImage2DArray = GL_UNSIGNED_INT_IMAGE_2D_ARRAY,
    UImage2DMS = GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,
    UImage2DMSArray = GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

    AtomicUInt = GL_UNSIGNED_INT_ATOMIC_COUNTER
};

class ShaderError : public std::runtime_error {
public:
    ShaderError(const std::string& info_log) : std::runtime_error(info_log) {}
};

class ShaderCompilationError : public ShaderError {
public:
    ShaderCompilationError(ShaderType type, const std::string& info_log)
        : ShaderError(ShaderTypeNames[type] + "\n" + info_log)
        , type_(type)
    {
    }

    ShaderType type() { return type_; }

private:
    ShaderType type_;
};

class ShaderLinkError : public ShaderError {
public:
    ShaderLinkError() = default;
    ShaderLinkError(const std::string& info_log)
        : ShaderError("Shader-Linking\n" + info_log)
    {
    }
};

struct ProgramInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr ObjectType Type = ObjectType::Program;
};

class Program;

class ShaderVariable {
public:
    ShaderVariable(Program& program, GLint data_size, DataType data_type, std::string name);

    Program& program();

    GLint dataSize();
    DataType dataType();
    const std::string& name();

private:
    Program& program_;
    GLint data_size_;
    DataType data_type_;
    std::string name_;
};

class ShaderAttribute : public ShaderVariable {
public:
    using ShaderVariable::ShaderVariable;
};

class ShaderUniform : public ShaderVariable {
public:
    using ShaderVariable::ShaderVariable;
};

class Program : public Object<ProgramInfo> {
public:
    void addShader(ShaderType type, std::string shader_code);

    void link();

private:
    void checkShaderStatusAndInfoLog(GLuint shader_handle, ShaderType type);
    void checkLinkStatusAndInfoLog();

    void loadAttributeLocations();
    void loadUniformLocations();

    std::vector<GLuint> shader_handles_;
    std::vector<ShaderAttribute> attributes_;
    std::vector<std::unique_ptr<ShaderUniform>> uniforms_;
};

}
