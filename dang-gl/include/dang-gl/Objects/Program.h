#pragma once

#include "dang-gl/General/GLConstants.h"
#include "dang-gl/Objects/DataTypes.h"
#include "dang-gl/Objects/Object.h"
#include "dang-gl/Objects/ObjectContext.h"
#include "dang-gl/Objects/ObjectHandle.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/ProgramContext.h"
#include "dang-gl/Objects/Texture.h"
#include "dang-gl/Objects/UniformWrapper.h"
#include "dang-gl/global.h"
#include "dang-utils/enum.h"

namespace dang::gl {

/// @brief The different possible shader stages with vertex and fragment being the most common.
enum class ShaderType {
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEvaluation,
    Compute,

    COUNT
};

} // namespace dang::gl

namespace dang::utils {

template <>
struct enum_count<dang::gl::ShaderType> : default_enum_count<dang::gl::ShaderType> {};

} // namespace dang::utils

namespace dang::gl {

/// @brief A mapping to the GL-Constants for each shader stage.
template <>
inline constexpr dutils::EnumArray<ShaderType, GLenum> gl_constants<ShaderType> = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_COMPUTE_SHADER,
};

/// @brief Human-readable names for each shader stage.
const dutils::EnumArray<ShaderType, std::string> shader_type_names = {
    "Vertex-Shader",
    "Fragment-Shader",
    "Geometry-Shader",
    "Tesselation-Control-Shader",
    "Tesselation-Evaluation-Shader",
    "Compute-Shader",
};

/// @brief Base class for shader errors with an info log.
class ShaderError : public std::runtime_error {
public:
    ShaderError(const std::string& info_log)
        : runtime_error(info_log)
    {}
};

/// @brief Thrown, when a shader has compilation errors.
class ShaderCompilationError : public ShaderError {
public:
    /// @brief Creates an error message using the name of the shader stage, followed by the info log.
    ShaderCompilationError(ShaderType type, const std::string& info_log)
        : ShaderError(shader_type_names[type] + "\n" + info_log)
        , type_(type)
    {}

    /// @brief The associated shader type.
    ShaderType type() const { return type_; }

private:
    ShaderType type_;
};

/// @brief Thrown, when the shader stages of a program cannot be linked.
class ShaderLinkError : public ShaderError {
public:
    /// @brief Creates an error message using the info log with a header.
    ShaderLinkError(const std::string& info_log)
        : ShaderError("Shader-Linking\n" + info_log)
    {}
};

/// @brief Thrown, when the requested type or count of a uniform does not match with the shader source code.
class ShaderUniformError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

/// @brief Thrown, when the specified shader attributes do not match the shader source, possibly because they got
/// optimized away.
class ShaderAttributeError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

/// @brief Thrown, when a shader file cannot be found at the given path.
class ShaderFileNotFound : public std::runtime_error {
public:
    ShaderFileNotFound(const fs::path& path)
        : runtime_error("Shader file not found: " + path.string())
    {}
};

class Program;

/// @brief Used for shader introspection for both attributes and uniforms.
class ShaderVariable {
public:
    /// @brief Initializes a shader variable wrapper with the given introspection information.
    ShaderVariable(const Program& program, GLint count, DataType type, std::string name, GLint location);

    /// @brief Binds the associated program.
    void bindProgram() const;

    /// @brief The length of arrays, 1 for the usual non-array types.
    GLint count() const;
    /// @brief The size in bytes of the value.
    GLsizei size() const;
    /// @brief The data type of the variable.
    DataType type() const;
    /// @brief The name of the variable.
    const std::string& name() const;
    /// @brief The location of the variable.
    GLint location() const;

private:
    ObjectContext<ObjectType::Program>* context_;
    ObjectHandle<ObjectType::Program> program_;
    GLint count_;
    DataType type_;
    std::string name_;
    GLint location_;
};

/// @brief A shader attribute, which additionally stores the byte-offset, which gets set by the program.
class ShaderAttribute : public ShaderVariable {
public:
    friend class Program;

    /// @brief Initializes a shader attribute wrapper with the given introspection information.
    ShaderAttribute(const Program& program, GLint count, DataType type, std::string name);

    /// @brief The byte-offset of the variable, set by the program.
    GLsizei offset() const;

private:
    GLsizei offset_ = -1;
};

/// @brief A polymorphic base class for uniform variables of any type.
class ShaderUniformBase : public ShaderVariable {
public:
    /// @brief Initializes a shader uniform wrapper with the given introspection information.
    ShaderUniformBase(const Program& program, GLint count, DataType type, std::string name);
    /// @brief Virtual destructor for polymorphism.
    virtual ~ShaderUniformBase() {}

    ShaderUniformBase(const ShaderUniformBase&) = delete;
    ShaderUniformBase(ShaderUniformBase&&) = delete;
    ShaderUniformBase& operator=(const ShaderUniformBase&) = delete;
    ShaderUniformBase& operator=(ShaderUniformBase&&) = delete;

    /// @brief Creates a shader uniform wrapper depending on the given data type.
    static std::unique_ptr<ShaderUniformBase> create(const Program& program,
                                                     GLint count,
                                                     DataType type,
                                                     std::string name);
};

/// @brief A wrapper for uniform variables of the template specified type.
template <typename T>
class ShaderUniform : public ShaderUniformBase {
public:
    /// @brief Initializes a shader uniform wrapper with the given introspection information.
    ShaderUniform(const Program& program, GLint count, DataType type, std::string name);
    /// @brief Initializes a dummy shader uniform wrapper, which does not actually exist in the shader.
    ShaderUniform(const Program& program, GLint count, std::string name);

    /// @brief Whether this uniform actually exists in the shader or is merely a dummy.
    bool exists() const;

    /// @brief Forces the value using glUniform calls.
    void force(const T& value, GLint index = 0);
    /// @brief Updates the uniform, if it differs from the cached value.
    void set(const T& value, GLint index = 0);
    /// @brief Returns the cached value of the uniform, which is queried once at creation.
    T get(GLint index = 0) const;

    /// @brief Allows for implicit assignment using a call to set.
    ShaderUniform& operator=(const T& value);
    /// @brief Allows for implicit conversion using a call to get.
    operator T() const;

    /// @brief Automatically binds the texture and assigns the returned slot to the sampler uniform.
    ShaderUniform& operator=(const TextureBase& texture)
    {
        static_assert(std::is_same_v<T, GLint>);
        set(static_cast<GLint>(texture.bind()));
        return *this;
    }

private:
    std::vector<T> values_;
};

using ShaderUniformSampler = ShaderUniform<int>;

/// @brief Contains the attribute order, stride and also supports instance division.
struct AttributeOrder {
    std::vector<std::reference_wrapper<ShaderAttribute>> attributes;
    GLsizei stride = 0;
    GLsizei divisor = 0;
};

/// @brief A GL-Program, built up of various shader stages which get linked together.
class Program : public ObjectBindable<ObjectType::Program> {
public:
    friend class ShaderPreprocessor;

    using AttributeNames = std::vector<std::string>;

    /// @brief Used to supply the attribute order to the link function.
    struct InstancedAttributes {
        GLsizei divisor;
        AttributeNames order;
    };

    using InstancedAttributeNames = std::vector<InstancedAttributes>;

    Program() = default;

    Program(EmptyObject)
        : ObjectBindable<ObjectType::Program>(empty_object)
    {}

    ~Program() = default;

    Program(const Program&) = delete;
    Program(Program&&) = default;
    Program& operator=(const Program&) = delete;
    Program& operator=(Program&&) = default;

    /// @brief Adds an include with the given name and code, which is used by the custom shader preprocessor.
    void addInclude(const std::string& name, std::string code);
    /// @brief Adds an include from the given path, using the filename as include name.
    void addIncludeFromFile(const fs::path& path);
    /// @brief Adds an include from the given path, using the given name as include name.
    void addIncludeFromFile(const fs::path& path, const std::string& name);

    /// @brief Adds a new shader for the specified stage with the given GLSL source code.
    void addShader(ShaderType type, const std::string& shader_code);
    /// @brief Adds a new shader for the specified stage from the given file path.
    void addShaderFromFile(ShaderType type, const fs::path& path);

    /// @brief Links all previously added shader stages together, cleans them up.
    /// @param attribute_order The order of the attributes of the Data struct, used in the VBO.
    /// @param instanced_attribute_order A list of instanced attributes with their respective divisors.
    void link(const AttributeNames& attribute_order = {},
              const InstancedAttributeNames& instanced_attribute_order = {});

    /// @brief Should return the attributes in the same order as they show up in the Data struct, used in the VBO.
    const AttributeOrder& attributeOrder() const;
    /// @brief Should return a list of attribute orders for instanced attributes.
    const std::vector<AttributeOrder>& instancedAttributeOrder() const;

    /// @brief Returns a wrapper to a uniform of the templated type, name and optional array size.
    /// @remark Will throw ShaderUniformError if the type or count doesn't match.
    template <typename T>
    ShaderUniform<T>& uniform(const std::string& name, GLint count = 1);

    /// @brief Returns a wrapper to a sampler (int) uniform, for the given name and optional array size.
    /// @remark Will throw ShaderUniformError if the type or count doesn't match.
    ShaderUniformSampler& uniformSampler(const std::string& name, GLint count = 1);

private:
    using ShaderHandle = ObjectHandle<ObjectType::Shader>;

    /// @brief Replaces source integer with the actual name of the source file.
    /// @remark Supports NVIDIA's 1(23) and Intel's 1:23 style.
    std::string replaceInfoLogShaderNames(std::string info_log) const;

    /// @brief Performs various cleanup, which is possible after linking.
    void postLinkCleanup();

    /// @brief Throws ShaderCompilationError if the shader could not compile or writes to std::cerr, in case of success
    /// but an existing info log.
    void checkShaderStatusAndInfoLog(ShaderHandle shader_handle, ShaderType type);
    /// @brief Throws ShaderLinkError if the program could not link or writes to std::cerr, in case of success but an
    /// existing info log.
    void checkLinkStatusAndInfoLog();

    /// @brief Queries all attributes after the program has been linked successfully.
    void loadAttributeLocations();
    /// @brief Queries all uniforms after the program has been linked successfully.
    void loadUniformLocations();
    /// @brief Sets the order of attributes, which should be the order of the Data structs, used in the VBO.
    void setAttributeOrder(const AttributeNames& attribute_order,
                           const InstancedAttributeNames& instanced_attribute_order);

    std::vector<ShaderHandle> shader_handles_;
    std::map<std::string, std::string> includes_;
    std::map<std::string, ShaderAttribute> attributes_;
    std::map<std::string, std::unique_ptr<ShaderUniformBase>> uniforms_;
    AttributeOrder attribute_order_;
    std::vector<AttributeOrder> instanced_attribute_order_;
};

/// @brief Processes shader source code for include directives.
class ShaderPreprocessor {
public:
    /// @brief Immediately processes the given code.
    ShaderPreprocessor(const Program& program, const std::string& code);
    /// @brief Returns the final source code with all include directive replaced by source code and line directives.
    std::string result() const;

private:
    /// @brief Processes the given code with the given compilation unit index.
    void process(const std::string& code, std::size_t compilation_unit);

    const Program& program_;
    std::set<std::string> included_;
    std::ostringstream output_;
    std::optional<std::tuple<std::size_t, std::size_t>> next_line_;
};

template <typename T>
inline ShaderUniform<T>::ShaderUniform(const Program& program, GLint count, DataType type, std::string name)
    : ShaderUniformBase(program, count, type, name)
    , values_(count)
{
    for (GLint index = 0; index < count; index++)
        values_[index] = UniformWrapper<T>::get(program.handle(), location() + index);
}

template <typename T>
inline ShaderUniform<T>::ShaderUniform(const Program& program, GLint count, std::string name)
    : ShaderUniformBase(program, count, DataType::None, std::move(name))
    , values_(count)
{}

template <typename T>
inline bool ShaderUniform<T>::exists() const
{
    return location() != -1;
}

template <typename T>
inline void ShaderUniform<T>::force(const T& value, GLint index)
{
    if (exists()) {
        bindProgram();
        UniformWrapper<T>::set(location() + index, value);
    }
    values_[index] = value;
}

template <typename T>
inline void ShaderUniform<T>::set(const T& value, GLint index)
{
    if (value == values_[index])
        return;
    force(value);
}

template <typename T>
inline T ShaderUniform<T>::get(GLint index) const
{
    return values_[index];
}

template <typename T>
inline ShaderUniform<T>& ShaderUniform<T>::operator=(const T& value)
{
    set(value);
    return *this;
}

template <typename T>
inline ShaderUniform<T>::operator T() const
{
    return get();
}

template <typename T>
inline ShaderUniform<T>& Program::uniform(const std::string& name, GLint count)
{
    auto pos = uniforms_.find(name);
    if (pos == uniforms_.end()) {
        auto uniform = std::make_unique<ShaderUniform<T>>(*this, count, name);
        ShaderUniform<T>& result = *uniform;
        uniforms_.emplace(name, std::move(uniform));
        return result;
    }

    ShaderUniformBase* shader_uniform = pos->second.get();

    if (count != shader_uniform->count())
        throw ShaderUniformError("Shader-Uniform count does not match.");

    if (auto typed_uniform = dynamic_cast<ShaderUniform<T>*>(shader_uniform))
        return *typed_uniform;

    throw ShaderUniformError("Shader-Uniform type does not match.");
}

} // namespace dang::gl
