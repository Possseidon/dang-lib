#pragma once

#include "dang-utils/enum.h"

#include "DataTypes.h"
#include "GLConstants.h"
#include "Object.h"
#include "ObjectContext.h"
#include "ObjectType.h"
#include "ProgramContext.h"
#include "Texture.h"
#include "UniformWrapper.h"

namespace dang::gl {

/// <summary>The different possible shader stages with vertex and fragment being the most common.</summary>
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
struct EnumCount<dang::gl::ShaderType> : DefaultEnumCount<dang::gl::ShaderType> {};

} // namespace dang::utils

namespace dang::gl {

/// <summary>A mapping to the GL-Constants for each shader stage.</summary>
template <>
inline constexpr dutils::EnumArray<ShaderType, GLenum> GLConstants<ShaderType> = {GL_VERTEX_SHADER,
                                                                                  GL_FRAGMENT_SHADER,
                                                                                  GL_GEOMETRY_SHADER,
                                                                                  GL_TESS_CONTROL_SHADER,
                                                                                  GL_TESS_EVALUATION_SHADER,
                                                                                  GL_COMPUTE_SHADER};

/// <summary>Human-readable names for each sahder stage.</summary>
const dutils::EnumArray<ShaderType, std::string> ShaderTypeNames{"Vertex-Shader",
                                                                 "Fragment-Shader",
                                                                 "Geometry-Shader",
                                                                 "Tesselation-Control-Shader",
                                                                 "Tesselation-Evaluation-Shader",
                                                                 "Compute-Shader"};

/// <summary>Base class for shader errors with an info log.</summary>
class ShaderError : public std::runtime_error {
public:
    ShaderError(const std::string& info_log)
        : runtime_error(info_log)
    {}
};

/// <summary>Thrown, when a shader has compilation errors.</summary>
class ShaderCompilationError : public ShaderError {
public:
    /// <summary>Creates an error message using the name of the shader stage, followed by the info log.</summary>
    ShaderCompilationError(ShaderType type, const std::string& info_log)
        : ShaderError(ShaderTypeNames[type] + "\n" + info_log)
        , type_(type)
    {}

    /// <summary>The associated shader type.</summary>
    ShaderType type() const { return type_; }

private:
    ShaderType type_;
};

/// <summary>Thrown, when the shader stages of a program cannot be linked.</summary>
class ShaderLinkError : public ShaderError {
public:
    /// <summary>Creates an error message using the info log with a header.</summary>
    ShaderLinkError(const std::string& info_log)
        : ShaderError("Shader-Linking\n" + info_log)
    {}
};

/// <summary>Thrown, when the requested type or count of a uniform does not match with the shader source code.</summary>
class ShaderUniformError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

/// <summary>Thrown, when the specified shader attributes do not match the shader source, possibly because they got optimized away.</summary>
class ShaderAttributeError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

/// <summary>Thrown, when a shader file cannot be found at the given path.</summary>
class ShaderFileNotFound : public std::runtime_error {
public:
    ShaderFileNotFound(const fs::path& path)
        : runtime_error("Shader file not found: " + path.string())
    {}
};

class Program;

/// <summary>Used for shader introspection for both attributes and uniforms.</summary>
class ShaderVariable {
public:
    /// <summary>Initializes a shader variable wrapper with the given introspection information.</summary>
    ShaderVariable(const Program& program, GLint count, DataType type, std::string name, GLint location);

    /// <summary>Binds the associated program.</summary>
    void bindProgram() const;

    /// <summary>The length of arrays, 1 for the usual non-array types.</summary>
    GLint count() const;
    /// <summary>The size in bytes of the value.</summary>
    GLsizei size() const;
    /// <summary>The data type of the variable.</summary>
    DataType type() const;
    /// <summary>The name of the variable.</summary>
    const std::string& name() const;
    /// <summary>The location of the variable.</summary>
    GLint location() const;

private:
    ObjectContext<ObjectType::Program>* context_;
    ObjectHandle<ObjectType::Program> program_;
    GLint count_;
    DataType type_;
    std::string name_;
    GLint location_;
};

/// <summary>A shader attribute, which additionally stores the byte-offset, which gets set by the program.</summary>
class ShaderAttribute : public ShaderVariable {
public:
    friend class Program;

    /// <summary>Initializes a shader attribute wrapper with the given introspection information.</summary>
    ShaderAttribute(const Program& program, GLint count, DataType type, std::string name);

    /// <summary>The byte-offset of the variable, set by the program.</summary>
    GLsizei offset() const;

private:
    GLsizei offset_ = -1;
};

/// <summary>A polymorphic base class for uniform variables of any type.</summary>
class ShaderUniformBase : public ShaderVariable {
public:
    /// <summary>Initializes a shader uniform wrapper with the given introspection information.</summary>
    ShaderUniformBase(const Program& program, GLint count, DataType type, std::string name);
    /// <summary>Virtual destructor for polymorphism.</summary>
    virtual ~ShaderUniformBase() {}

    ShaderUniformBase(const ShaderUniformBase&) = delete;
    ShaderUniformBase(ShaderUniformBase&&) = delete;
    ShaderUniformBase& operator=(const ShaderUniformBase&) = delete;
    ShaderUniformBase& operator=(ShaderUniformBase&&) = delete;

    /// <summary>Creates a shader uniform wrapper depending on the given data type.</summary>
    static std::unique_ptr<ShaderUniformBase> create(const Program& program,
                                                     GLint count,
                                                     DataType type,
                                                     std::string name);
};

/// <summary>A wrapper for uniform variables of the template specified type.</summary>
template <typename T>
class ShaderUniform : public ShaderUniformBase {
public:
    /// <summary>Initializes a shader uniform wrapper with the given introspection information.</summary>
    ShaderUniform(const Program& program, GLint count, DataType type, std::string name);
    /// <summary>Initializes a dummy shader uniform wrapper, which does not actually exist in the shader.</summary>
    ShaderUniform(const Program& program, GLint count, std::string name);

    /// <summary>Whether this uniform actually exists in the shader or is merely a dummy.</summary>
    bool exists() const;

    /// <summary>Forces the value using glUniform calls.</summary>
    void force(const T& value, GLint index = 0);
    /// <summary>Updates the uniform, if it differs from the cached value.</summary>
    void set(const T& value, GLint index = 0);
    /// <summary>Returns the cached value of the uniform, which is queried once at creation.</summary>
    T get(GLint index = 0) const;

    /// <summary>Allows for implicit assignment using a call to set.</summary>
    ShaderUniform& operator=(const T& value);
    /// <summary>Allows for implicit conversion using a call to get.</summary>
    operator T() const;

    /// <summary>Automatically binds the texture and assigns the returned slot to the sampler uniform.</summary>
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

/// <summary>Contains the attribute order, stride and also supports instance division.</summary>
struct AttributeOrder {
    std::vector<std::reference_wrapper<ShaderAttribute>> attributes;
    GLsizei stride = 0;
    GLsizei divisor = 0;
};

/// <summary>A GL-Program, built up of various shader stages which get linked together.</summary>
class Program : public ObjectBindable<ObjectType::Program> {
public:
    friend class ShaderPreprocessor;

    using AttributeNames = std::vector<std::string>;

    /// <summary>Used to supply the attribute order to the link function.</summary>
    struct InstancedAttributes {
        GLsizei divisor;
        AttributeNames order;
    };

    using InstancedAttributeNames = std::vector<InstancedAttributes>;

    Program() = default;
    ~Program() = default;

    Program(const Program&) = delete;
    Program(Program&&) = default;
    Program& operator=(const Program&) = delete;
    Program& operator=(Program&&) = default;

    /// <summary>Adds an include with the given name and code, which is used by the custom shader pre-processor.</summary>
    void addInclude(const std::string& name, std::string code);
    /// <summary>Adds an include from the given path, using the filename as include name.</summary>
    void addIncludeFromFile(const fs::path& path);
    /// <summary>Adds an include from the given path, using the given name as include name.</summary>
    void addIncludeFromFile(const fs::path& path, const std::string& name);

    /// <summary>Adds a new shader for the specified stage with the given GLSL source code.</summary>
    void addShader(ShaderType type, const std::string& shader_code);
    /// <summary>Adds a new shader for the specified stage from the given file path.</summary>
    void addShaderFromFile(ShaderType type, const fs::path& path);

    /// <summary>Links all previously added shader stages together, cleans them up.</summary>
    /// <param name="attribute_order">The order of the attributes of the Data struct, used in the VBO.</param>
    void link(const AttributeNames& attribute_order = {},
              const InstancedAttributeNames& instanced_attribute_order = {});

    /// <summary>Should return the attributes in the same order as they show up in the Data struct, used in the VBO.</summary>
    const AttributeOrder& attributeOrder() const;
    /// <summary>TODO</summary>
    const std::vector<AttributeOrder>& instancedAttributeOrder() const;

    /// <summary>Returns a wrapper to a uniform of the templated type, name and optional array size.</summary>
    /// <remarks>Will throw ShaderUniformError if the type or count doesn't match.</remarks>
    template <typename T>
    ShaderUniform<T>& uniform(const std::string& name, GLint count = 1);

    /// <summary>Returns a wrapper to a sampler (int) uniform, for the given name and optional array size.</summary>
    /// <remarks>Will throw ShaderUniformError if the type or count doesn't match.</remarks>
    ShaderUniformSampler& uniformSampler(const std::string& name, GLint count = 1);

private:
    /// <summary>Replaces source integer with the actual name of the source file.</summary>
    /// <remarks>Supports NVIDIA's 1(23) and Intel's 1:23 style.</remarks>
    std::string replaceInfoLogShaderNames(std::string info_log) const;

    /// <summary>Performs various cleanup, which is possible after linking.</summary>
    void postLinkCleanup();

    /// <summary>Throws ShaderCompilationError if the shader could not compile or writes to std::cerr, in case of success but an existing info log.</summary>
    void checkShaderStatusAndInfoLog(Handle shader_handle, ShaderType type);
    /// <summary>Throws ShaderLinkError if the program could not link or writes to std::cerr, in case of success but an existing info log.</summary>
    void checkLinkStatusAndInfoLog();

    /// <summary>Queries all attributes after the program has been linked successfully.</summary>
    void loadAttributeLocations();
    /// <summary>Queries all uniforms after the program has been linked successfully.</summary>
    void loadUniformLocations();
    /// <summary>Sets the order of attributes, which should be the order of the Data structs, used in the VBO.</summary>
    void setAttributeOrder(const AttributeNames& attribute_order,
                           const InstancedAttributeNames& instanced_attribute_order);

    std::vector<Handle> shader_handles_;
    std::map<std::string, std::string> includes_;
    std::map<std::string, ShaderAttribute> attributes_;
    std::map<std::string, std::unique_ptr<ShaderUniformBase>> uniforms_;
    AttributeOrder attribute_order_;
    std::vector<AttributeOrder> instanced_attribute_order_;
};

/// <summary>Processes shader source code for #include directives.</summary>
class ShaderPreprocessor {
public:
    /// <summary>Immediately processes the given code.</summary>
    ShaderPreprocessor(const Program& program, const std::string& code);
    /// <summary>Returns the final source code with all #include directive replaced by source code and #line directives.</summary>
    std::string result() const;

private:
    /// <summary>Processes the given code with the given compilation unit index.</summary>
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
