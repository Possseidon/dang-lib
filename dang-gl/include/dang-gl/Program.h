#pragma once

#include "dang-utils/enum.h"

#include "BindingPoint.h"
#include "DataType.h"
#include "Object.h"
#include "UniformWrapper.h"

namespace dang::gl
{

enum class DataType;

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

/// <summary>A mapping to the GL-Constants for each shader stage.</summary>
constexpr dutils::EnumArray<ShaderType, GLenum> ShaderTypesGL
{
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_COMPUTE_SHADER
};

/// <summary>Human-readable names for each sahder stage.</summary>
const dutils::EnumArray<ShaderType, std::string> ShaderTypeNames
{
    "Vertex-Shader",
    "Fragment-Shader",
    "Geometry-Shader",
    "Tesselation-Control-Shader",
    "Tesselation-Evaluation-Shader",
    "Compute-Shader"
};

/// <summary>Base class for shader errors with an info log.</summary>
class ShaderError : public std::runtime_error {
public:
    ShaderError(const std::string& info_log) : std::runtime_error(info_log) {}
};

/// <summary>Thrown, when a shader has compilation errors.</summary>
class ShaderCompilationError : public ShaderError {
public:
    ShaderCompilationError(ShaderType type, const std::string& info_log)
        : ShaderError(ShaderTypeNames[type] + "\n" + info_log)
        , type_(type)
    {
    }

    ShaderType type() const
    {
        return type_;
    }

private:
    ShaderType type_;
};

/// <summary>Thrown, when the shader stages of a program cannot be linked.</summary>
class ShaderLinkError : public ShaderError {
public:
    ShaderLinkError(const std::string& info_log)
        : ShaderError("Shader-Linking\n" + info_log)
    {
    }
};

/// <summary>Thrown, when the requested type or count of a uniform does not match with the shader source code.</summary>
class ShaderUniformError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// <summary>Thrown, when the specified shader attributes do not match the shader source, possibly because they got optimized away.</summary>
class ShaderAttributeError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// <summary>Thrown, when a shader file cannot be found at the given path.</summary>
class ShaderFileNotFound : public std::runtime_error {
public:
    ShaderFileNotFound(const fs::path& path)
        : std::runtime_error("Shader file not found: " + path.string())
    {
    }
};

/// <summary>Info struct to create, destroy and bind (actually use) a program object.</summary>
struct ProgramInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr BindingPoint BindingPoint = BindingPoint::Program;
};

class Program;

/// <summary>Used for shader introspection for both attributes and uniforms.</summary>
class ShaderVariable {
public:
    /// <summary>Initializes a shader variable wrapper with the given introspection information.</summary>
    ShaderVariable(Program& program, GLint count, DataType type, std::string name, GLint location);

    /// <summary>The associated program.</summary>
    Program& program() const;

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
    Program& program_;
    GLint count_;
    DataType type_;
    std::string name_;
    GLint location_;
};

/// <summary>A shader attribute, which additionally stores the byte-offset, which gets set by the program.</summary>
class ShaderAttribute : public ShaderVariable {
public:
    /// <summary>Initializes a shader attribute wrapper with the given introspection information.</summary>
    ShaderAttribute(Program& program, GLint count, DataType type, std::string name);

    friend class Program;
    /// <summary>The byte-offset of the variable, set by the program.</summary>
    GLsizei offset() const;

private:
    GLsizei offset_ = -1;
};

/// <summary>A polymorphic base class for uniform variables of any type.</summary>
class ShaderUniformBase : public ShaderVariable {
public:
    /// <summary>Initializes a shader uniform wrapper with the given introspection information.</summary>
    ShaderUniformBase(Program& program, GLint count, DataType type, std::string name);
    /// <summary>Virtual destructor for polymorphism.</summary>
    virtual ~ShaderUniformBase() {}

    ShaderUniformBase(const ShaderUniformBase&) = delete;
    ShaderUniformBase(ShaderUniformBase&&) = delete;
    ShaderUniformBase& operator=(const ShaderUniformBase&) = delete;
    ShaderUniformBase& operator=(ShaderUniformBase&&) = delete;

    /// <summary>Creates a shader uniform wrapper depending on the given data type.</summary>
    static std::unique_ptr<ShaderUniformBase> create(Program& program, GLint count, DataType type, std::string name);
};

/// <summary>A wrapper for uniform variables of the template specified type.</summary>
template <typename T>
class ShaderUniform : public ShaderUniformBase {
public:
    /// <summary>Initializes a shader uniform wrapper with the given introspection information.</summary>
    ShaderUniform(Program& program, GLint count, DataType type, std::string name);
    /// <summary>Initializes a dummy shader uniform wrapper, which does not actually exist in the shader.</summary>
    ShaderUniform(Program& program, GLint count, std::string name);

    /// <summary>Forces the value using glUniform calls.</summary>
    void force(const T& value, GLint index = 0);
    /// <summary>Updates the uniform, if it differs from the cached value.</summary>
    void set(const T& value, GLint index = 0);
    /// <summary>Returns the cached value of the uniform, which is queried once at creation.</summary>
    T get(GLint index = 0);

    /// <summary>Allows for implicit assignment using a call to set.</summary>
    ShaderUniform& operator=(const T& value);
    /// <summary>Allows for implicit conversion using a call to get.</summary>
    operator T();

private:
    std::vector<T> values_;
};

/// <summary>A GL-Program, built up of various shader stages which get linked together.</summary>
class Program : public Object<ProgramInfo> {
public:
    friend class ShaderPreprocessor;

    Program() = default;
    Program(Program&&) = delete;

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
    void link(const std::vector<std::string>& attribute_order = {});

    /// <summary>Should be the same as the size of the data struct, used in the VBO.</summary>
    GLsizei attributeStride() const;
    /// <summary>Should return the attributes in the same order as they show up in the Data struct, used in the VBO.</summary>
    const std::vector<std::reference_wrapper<ShaderAttribute>>& attributeOrder() const;

    /// <summary>Returns a wrapper to a uniform of the templated type, name and optional array size.</summary>
    /// <remarks>Will throw ShaderUniformError if the type or count doesn't match.</remarks>
    template <typename T>
    ShaderUniform<T>& uniform(const std::string& name, GLint count = 1);

private:
    /// <summary>Replaces source integer with the actual name of the source file.</summary>
    /// <remarks>Supports NVIDIA's 1(23) and Intel's 1:23 style.</remarks>
    std::string replaceInfoLogShaderNames(std::string info_log) const;

    /// <summary>Performs various cleanup, which is possible after linking.</summary>
    void postLinkCleanup();

    /// <summary>Throws ShaderCompilationError if the shader could not compile or writes to std::cerr, in case of success but an existing info log.</summary>
    void checkShaderStatusAndInfoLog(GLuint shader_handle, ShaderType type);
    /// <summary>Throws ShaderLinkError if the program could not link or writes to std::cerr, in case of success but an existing info log.</summary>
    void checkLinkStatusAndInfoLog();

    /// <summary>Queries all attributes after the program has been linked successfully.</summary>
    void loadAttributeLocations();
    /// <summary>Queries all uniforms after the program has been linked successfully.</summary>
    void loadUniformLocations();
    /// <summary>Sets the order of attributes, which should be the order of the Data struct, used in the VBO.</summary>
    void setAttributeOrder(const std::vector<std::string>& attribute_order);

    std::vector<GLuint> shader_handles_;
    std::map<std::string, std::string> includes_;
    std::map<std::string, ShaderAttribute> attributes_;
    std::map<std::string, std::unique_ptr<ShaderUniformBase>> uniforms_;
    std::vector<std::reference_wrapper<ShaderAttribute>> attribute_order_;
    GLsizei attribute_stride_ = 0;
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

template<typename T>
inline ShaderUniform<T>::ShaderUniform(Program& program, GLint count, DataType type, std::string name)
    : ShaderUniformBase(program, count, type, name)
    , values_(count)
{
    for (GLint index = 0; index < count; index++)
        values_[index] = UniformWrapper<T>::get(program.handle(), location() + index);
}

template<typename T>
inline ShaderUniform<T>::ShaderUniform(Program& program, GLint count, std::string name)
    : ShaderUniformBase(program, count, DataType::None, std::move(name))
    , values_(count)
{
}

template<typename T>
inline void ShaderUniform<T>::force(const T& value, GLint index)
{
    if (location() != -1) {
        program().bind();
        UniformWrapper<T>::set(location() + index, value);
    }
    values_[index] = value;
}

template<typename T>
inline void ShaderUniform<T>::set(const T& value, GLint index)
{
    if (value == values_[index])
        return;
    force(value);
}

template<typename T>
inline T ShaderUniform<T>::get(GLint index)
{
    return values_[index];
}

template<typename T>
inline ShaderUniform<T>& ShaderUniform<T>::operator=(const T& value)
{
    set(value);
    return *this;
}

template<typename T>
inline ShaderUniform<T>::operator T()
{
    return get();
}

template<typename T>
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

}
