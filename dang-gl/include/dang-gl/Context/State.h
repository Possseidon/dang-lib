#pragma once

#include "dang-gl/Context/StateTypes.h"
#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/global.h"

#include "dang-math/vector.h"

// TODO: Consider using template specialization instead of polymorphism for properties, which should be sufficient (I
// think?)
//       Backup still needs to be polymorphic, as they get stored in a vector

namespace dang::gl {

class State;

namespace detail {

template <typename T>
class StateProperty;

/// @brief A base class for all different OpenGL states.
class StatePropertyBase {
public:
    friend class dang::gl::State;

    /// @brief Initializes the property, automatically incrementing the property count on the state itself.
    StatePropertyBase(State& state);

protected:
    /// @brief Simply delegates to the backup function of the state.
    template <typename T>
    void backupValue(detail::StateProperty<T>& property);

    State& state_;
    std::size_t index_;
};

/// @brief A polymorphic templated state property to provide a type-safe, but uniform access to OpenGL states.
template <typename T>
class StateProperty : public StatePropertyBase {
public:
    /// @brief Initializes the property with the given state and an optional default value, which default to the actual
    /// default value of the type.
    /// @remark The supplied default value should match the actual default value of the OpenGL state.
    StateProperty(State& state, const T& default_value = T())
        : StatePropertyBase(state)
        , default_value_(default_value)
        , value_(default_value)
    {}

    /// @brief Ensure the correct destructors are called, as the class is polymorphic.
    virtual ~StateProperty() = default;

    StateProperty(const StateProperty&) = delete;
    StateProperty(StateProperty&&) = delete;
    StateProperty& operator=(const StateProperty&) = delete;
    StateProperty& operator=(StateProperty&&) = delete;

    /// @brief Allows for implicit assignment of the value type.
    StateProperty& operator=(const T& value);

    /// @brief Allows for implicit conversion to the cached value.
    operator const T&() const { return value_; }

    /// @brief Returns the cached value.
    const T& operator*() const { return value_; }

    /// @brief Returns the cached value.
    const T* operator->() const { return &value_; }

    /// @brief Returns the cached value.
    const T& value() const { return value_; }

    /// @brief Returns the default value.
    const T& defaultValue() const { return default_value_; }

    /// @brief Resets the state to its default value.
    void reset() { *this = default_value_; }

protected:
    /// @brief Virtual
    virtual void update() = 0;

private:
    T default_value_;
    T value_;
};

/// @brief State flags that can be enabled and disabled with glEnable and glDisable respectively.
template <GLenum Flag>
class StateFlag : public StateProperty<bool> {
public:
    using StateProperty<bool>::StateProperty;

    /// @brief Allows for implicit assignment of the flag.
    StateFlag& operator=(bool value)
    {
        StateProperty<bool>::operator=(value);
        return *this;
    }

protected:
    /// @brief Calls glEnable and glDisable.
    void update() override
    {
        if (*this)
            glEnable(Flag);
        else
            glDisable(Flag);
    }
};

/// @brief A state property, which calls a template supplied function with a single enum, arithmetic value or struct
/// with a toTuple method.
template <auto Func, typename T>
class StateFunc : public StateProperty<T> {
public:
    using StateProperty<T>::StateProperty;

    /// @brief Allows for implicit assignment of the state.
    StateFunc& operator=(const T& value)
    {
        StateProperty<T>::operator=(value);
        return *this;
    }

protected:
    /// @brief Calls the template specified function with the current value.
    void update() override
    {
        if constexpr (std::is_enum_v<T>)
            (*Func)(toGLConstant(**this));
        else if constexpr (std::is_arithmetic_v<T>)
            (*Func)(**this);
        else
            std::apply(*Func, (*this)->toTuple());
    }
};

/// @brief A state property, which calls the template supplied function with each vector component as a separate
/// parameter.
template <auto Func, typename T, std::size_t Dim>
class StateVector : public StateProperty<dmath::Vector<T, Dim>> {
public:
    using StateProperty<dmath::Vector<T, Dim>>::StateProperty;

    /// @brief Allows for implicit assignment of the state.
    StateVector& operator=(const dmath::Vector<T, Dim>& value)
    {
        StateProperty<dmath::Vector<T, Dim>>::operator=(value);
        return *this;
    }

protected:
    /// @brief Calls the template specified function with the current vector components.
    void update() override { std::apply(*Func, **this); }
};

/// @brief A polymorphic base class for state backups.
class StateBackupBase {
public:
    virtual ~StateBackupBase() = 0;
};

/// @brief A map from property index to state backup.
using StateBackupSet = std::map<std::size_t, std::unique_ptr<StateBackupBase>>;

/// @brief A templated state backup, which automatically resets a state to its original value on destruction.
template <typename T>
class StateBackup : public StateBackupBase {
public:
    /// @brief Stores the current value of the state.
    StateBackup(StateProperty<T>& property)
        : property_(property)
        , old_value_(property)
    {}

    /// @brief Automatically resets the state to it old value.
    ~StateBackup() override { property_ = old_value_; }

private:
    StateProperty<T>& property_;
    T old_value_;
};

template <typename T>
inline constexpr auto glGet = nullptr;

template <>
inline constexpr auto& glGet<GLboolean> = glGetBooleanv;
template <>
inline constexpr auto& glGet<GLdouble> = glGetDoublev;
template <>
inline constexpr auto& glGet<GLfloat> = glGetFloatv;
template <>
inline constexpr auto& glGet<GLint> = glGetIntegerv;
template <>
inline constexpr auto& glGet<GLint64> = glGetInteger64v;

/// @brief A constant, which is queried on first use, but cached for further accesses.
template <typename T, GLenum Name>
class Constant {
public:
    /// @brief Calls glGet the first time, but caches the value.
    const T& value() const
    {
        if (!value_) {
            value_.emplace();
            glGet<T>(Name, &*value_);
        }
        return *value_;
    }

    /// @brief Allows for implicit conversion to the value type.
    operator const T&() const { return value(); }

private:
    mutable std::optional<T> value_;
};

template <typename T>
inline constexpr auto glGeti = nullptr;

template <>
inline constexpr auto& glGeti<GLboolean> = glGetBooleani_v;
template <>
inline constexpr auto& glGeti<GLdouble> = glGetDoublei_v;
template <>
inline constexpr auto& glGeti<GLfloat> = glGetFloati_v;
template <>
inline constexpr auto& glGeti<GLint> = glGetIntegeri_v;
template <>
inline constexpr auto& glGeti<GLint64> = glGetInteger64i_v;

/// @brief A list of constants, which is queried on first use, but cached for further accesses.
template <typename T, GLenum Name>
class IndexedConstant {
public:
    /// @brief Queries the given index, but caches all indices.
    const T& operator[](std::size_t index) const
    {
        if (index >= values_.size())
            values_.resize(index + 1);
        else if (values_[index])
            return values_[index];
        auto& value = values_[index].emplace();
        glGeti<T>(Name, index, &value);
        return values_[index];
    }

private:
    mutable std::vector<std::optional<T>> values_;
};

// TODO: Add size for IndexedConstant and create a separate class, which can query the size from another state.

} // namespace detail

/// @brief A scope based state modification, which automatically reverts to the old state, when it goes out of scope.
class ScopedState {
public:
    /// @brief Allows for temporary modifications, which get reverted at the end of the scope.
    ScopedState(State& state);
    /// @brief Automatically reverts all modified states to their old values.
    ~ScopedState();

    /// @brief Returns the actual state.
    State& operator*() const;
    /// @brief Allows access to the state through this wrapper.
    State* operator->() const;

private:
    State& state_;
};

/// @brief Wraps the full state of an OpenGL context and supports efficient push/pop semantics, to temporarily modify a
/// set of states.
class State {
private:
    // Must be initialized before the properties
    std::size_t property_count_ = 0;

public:
    friend class detail::StatePropertyBase;

    State(svec2 size)
        : scissor{*this, Scissor{ibounds2{size}}}
    {}

    /// @brief Allows for temporary modifications, which get reverted by the matching pop call.
    void push();
    /// @brief Reverts all modified states to their old values.
    void pop();

    /// @brief Uses an RAII wrapper, to ensure pop is called at the end of the scope, even in case of exceptions.
    ScopedState scoped();

    detail::StateFlag<GL_BLEND> blend{*this};
    detail::StateFlag<GL_COLOR_LOGIC_OP> color_logic_op{*this};
    detail::StateFlag<GL_CULL_FACE> cull_face{*this};
    detail::StateFlag<GL_DEBUG_OUTPUT> debug_output{*this};
    detail::StateFlag<GL_DEBUG_OUTPUT_SYNCHRONOUS> debug_output_synchronous{*this};
    detail::StateFlag<GL_DEPTH_CLAMP> depth_clamp{*this};
    detail::StateFlag<GL_DEPTH_TEST> depth_test{*this};
    detail::StateFlag<GL_DITHER> dither{*this, true};
    detail::StateFlag<GL_FRAMEBUFFER_SRGB> framebuffer_srgb{*this};
    detail::StateFlag<GL_LINE_SMOOTH> line_smooth{*this};
    detail::StateFlag<GL_MULTISAMPLE> multisample{*this};
    detail::StateFlag<GL_POLYGON_SMOOTH> polygon_smooth{*this};
    detail::StateFlag<GL_POLYGON_OFFSET_FILL> polygon_offset_fill{*this};
    detail::StateFlag<GL_POLYGON_OFFSET_LINE> polygon_offset_line{*this};
    detail::StateFlag<GL_POLYGON_OFFSET_POINT> polygon_offset_point{*this};
    detail::StateFlag<GL_PROGRAM_POINT_SIZE> program_point_size{*this};
    detail::StateFlag<GL_PRIMITIVE_RESTART> primitive_restart{*this};
    detail::StateFlag<GL_SAMPLE_ALPHA_TO_COVERAGE> sample_alpha_to_coverage{*this};
    detail::StateFlag<GL_SAMPLE_ALPHA_TO_ONE> sample_alpha_to_one{*this};
    detail::StateFlag<GL_SAMPLE_COVERAGE> sample_coverage{*this};
    detail::StateFlag<GL_SAMPLE_MASK> sample_mask{*this};
    detail::StateFlag<GL_SCISSOR_TEST> scissor_test{*this};
    detail::StateFlag<GL_STENCIL_TEST> stencil_test{*this};
    detail::StateFlag<GL_TEXTURE_CUBE_MAP_SEAMLESS> texture_cube_map_seamless{*this};

    // TODO: GL_CLIP_DISTANCEi

    detail::StateFunc<&glBlendFunc, BlendFactor> blend_func{*this, {BlendFactorSrc::One, BlendFactorDst::Zero}};
    detail::StateFunc<&glCullFace, CullFaceMode> cull_face_mode{*this, CullFaceMode::Back};
    detail::StateFunc<&glLineWidth, GLfloat> line_width{*this, 1.0f};
    detail::StateFunc<&glLogicOp, LogicOp> logic_op{*this, LogicOp::Copy};
    detail::StateFunc<&glPolygonMode, PolygonSideMode<PolygonSide::Front>> polygon_mode_front{*this,
                                                                                              {PolygonMode::Fill}};
    detail::StateFunc<&glPolygonMode, PolygonSideMode<PolygonSide::Back>> polygon_mode_back{*this, {PolygonMode::Fill}};
    detail::StateFunc<&glPolygonOffset, PolygonOffset> polygon_offset{*this, {0.0f, 0.0f}};
    detail::StateFunc<&glPrimitiveRestartIndex, GLuint> primitive_restart_index{*this, 0};
    detail::StateFunc<&glSampleCoverage, SampleCoverage> sample_coverage_value{*this, {1.0f, GL_FALSE}};
    detail::StateFunc<&glScissor, Scissor> scissor; // defaults to framebuffer size, set in constructor
    detail::StateFunc<&glStencilFunc, StencilFunc> stencil_func{*this, {CompareFunc::Always, 0, GLuint(-1)}};
    detail::StateFunc<&glStencilOp, StencilOp> stencil_op{
        *this, {StencilAction::Keep, StencilAction::Keep, StencilAction::Keep}};

    detail::StateVector<&glClearColor, GLfloat, 4> clear_color{*this, {0.0f, 0.0f, 0.0f, 0.0f}};
    detail::StateFunc<&glClearDepth, GLfloat> clear_depth{*this, 0.0f};
    detail::StateFunc<&glClearStencil, GLint> clear_stencil{*this, 0};

    detail::Constant<GLint, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS> max_combined_texture_image_units;
    detail::Constant<GLint, GL_MAX_COLOR_ATTACHMENTS> max_color_attachments;

private:
    /// @brief If the property hasn't been backed up yet, it gets added to the top of the state backup stack.
    template <typename T>
    void backupValue(detail::StateProperty<T>& property);

    std::stack<detail::StateBackupSet> state_backup_;
};

template <typename T>
inline void detail::StatePropertyBase::backupValue(StateProperty<T>& property)
{
    state_.backupValue(property);
}

template <typename T>
inline detail::StateProperty<T>& detail::StateProperty<T>::operator=(const T& value)
{
    if (value_ != value) {
        StatePropertyBase::backupValue(*this);
        value_ = value;
        update();
    }
    return *this;
}

inline detail::StateBackupBase::~StateBackupBase() {}

template <typename T>
inline void State::backupValue(detail::StateProperty<T>& property)
{
    if (state_backup_.empty())
        return;

    detail::StateBackupSet& change_set = state_backup_.top();
    if (change_set.find(property.index_) != change_set.end())
        return;

    change_set.emplace(property.index_, std::make_unique<detail::StateBackup<T>>(property));
}

} // namespace dang::gl
