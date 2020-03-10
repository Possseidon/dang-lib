#pragma once

#include "StateTypes.h"

namespace dang::gl
{

class State;

namespace detail
{

template <typename T>
class StateProperty;

class StatePropertyBase {
public:
    friend class State;

    StatePropertyBase(State& state);

protected:
    template <typename T>
    void backupValue(detail::StateProperty<T>& property);

    State& state_;
    std::size_t index_;
};

template <typename T>
class StateProperty : public StatePropertyBase {
public:
    StateProperty(State& state, const T& default_value = T())
        : StatePropertyBase(state)
        , default_value_(default_value)
        , value_(default_value)
    {
    }

    StateProperty(const StateProperty&) = delete;
    StateProperty(StateProperty&&) = delete;
    StateProperty& operator=(const StateProperty&) = delete;
    StateProperty& operator=(StateProperty&&) = delete;

    StateProperty& operator=(const T& value);

    operator const T& () const
    {
        return value_;
    }

    const T& operator*() const
    {
        return value_;
    }

    const T* operator->() const
    {
        return &value_;
    }

    const T& value() const
    {
        return value_;
    }

    const T& defaultValue() const
    {
        return default_value_;
    }

    void reset()
    {
        *this = default_value_;
    }

protected:
    virtual void update() = 0;

private:
    T default_value_;
    T value_;
};

template <GLenum Flag>
class StateFlag : public StateProperty<bool> {
public:
    using StateProperty<bool>::StateProperty;

    StateFlag& operator=(bool value)
    {
        StateProperty<bool>::operator=(value);
        return *this;
    }

protected:
    void update() override
    {
        if (*this)
            glEnable(Flag);
        else
            glDisable(Flag);
    }
};

template <auto Func, typename T>
class StateFunc : public StateProperty<T> {
public:
    using StateProperty<T>::StateProperty;

    StateFunc& operator=(const T& value)
    {
        StateProperty<T>::operator=(value);
        return *this;
    }

protected:
    void update() override
    {
        if constexpr (std::is_enum_v<T>)
            (*Func)(static_cast<std::underlying_type_t<T>>(**this));
        else if constexpr (std::is_arithmetic_v<T>)
            (*Func)(**this);
        else
            std::apply(*Func, (*this)->toTuple());
    }
};

class StateBackupBase {
public:
    virtual ~StateBackupBase() = 0;
};

using StateBackupSet = std::map<std::size_t, std::unique_ptr<StateBackupBase>>;

template <typename T>
class StateBackup : public StateBackupBase {
public:
    StateBackup(StateProperty<T>& property)
        : property_(property)
        , old_value_(property)
    {
    }

    ~StateBackup() override
    {
        property_ = old_value_;
    }

private:
    StateProperty<T>& property_;
    T old_value_;
};

}

class ScopedState {
public:
    ScopedState(State& state);
    ~ScopedState();

    State& operator*() const;
    State* operator->() const;

private:
    State& state_;
};

class State {
public:
    friend class detail::StatePropertyBase;

    void push();
    void pop();

    ScopedState scoped();

    detail::StateFlag<GL_BLEND> blend{ *this };
    detail::StateFlag<GL_COLOR_LOGIC_OP> color_logic_op{ *this };
    detail::StateFlag<GL_CULL_FACE> cull_face{ *this };
    detail::StateFlag<GL_DEPTH_TEST> depth_test{ *this };
    detail::StateFlag<GL_DITHER> dither{ *this, true };
    detail::StateFlag<GL_LINE_SMOOTH> line_smooth{ *this };
    detail::StateFlag<GL_MULTISAMPLE> multisample{ *this };
    detail::StateFlag<GL_POLYGON_SMOOTH> polygon_smooth{ *this };
    detail::StateFlag<GL_POLYGON_OFFSET_FILL> polygon_offset_fill{ *this };
    detail::StateFlag<GL_POLYGON_OFFSET_LINE> polygon_offset_line{ *this };
    detail::StateFlag<GL_POLYGON_OFFSET_POINT> polygon_offset_point{ *this };
    detail::StateFlag<GL_SAMPLE_ALPHA_TO_COVERAGE> sample_alpha_to_coverage{ *this };
    detail::StateFlag<GL_SAMPLE_ALPHA_TO_ONE> sample_alpha_to_one{ *this };
    detail::StateFlag<GL_SAMPLE_COVERAGE> sample_coverage{ *this };
    detail::StateFlag<GL_SCISSOR_TEST> scissor_test{ *this };
    detail::StateFlag<GL_STENCIL_TEST> stencil_test{ *this };
    detail::StateFlag<GL_TEXTURE_1D> texture_1d{ *this };
    detail::StateFlag<GL_TEXTURE_2D> texture_2d{ *this };
    detail::StateFlag<GL_TEXTURE_3D> texture_3d{ *this };
    detail::StateFlag<GL_TEXTURE_CUBE_MAP> texture_cube_map{ *this };
    detail::StateFlag<GL_VERTEX_ARRAY> vertex_array{ *this };
    detail::StateFlag<GL_VERTEX_PROGRAM_POINT_SIZE> vertex_program_point_size{ *this };

    detail::StateFunc<&glBlendFunc, BlendFactor> blend_func{ *this, { BlendFactorSrc::One, BlendFactorDst::Zero} };
    detail::StateFunc<&glLogicOp, LogicOp> logic_op{ *this, LogicOp::Copy };
    detail::StateFunc<&glCullFace, CullFaceMode> cull_face_mode{ *this, CullFaceMode::Back };
    detail::StateFunc<&glLineWidth, GLfloat> line_width{ *this, 1.0f };
    detail::StateFunc<&glSampleCoverage, SampleCoverage> sample_coverage_value{ *this, { 1.0f, GL_FALSE } };
    detail::StateFunc<&glScissor, Scissor> scissor{ *this, { { 0, 0 } } }; // TODO: Set to window size
    detail::StateFunc<&glStencilFunc, StencilFunc> stencil_func{ *this, { CompareFunc::Always, 0, GLuint(-1) } };
    detail::StateFunc<&glStencilOp, StencilOp> stencil_op{ *this, { StencilAction::Keep, StencilAction::Keep, StencilAction::Keep } };

private:
    template <typename T>
    void backupValue(detail::StateProperty<T>& property);

    std::stack<detail::StateBackupSet> state_backup_;
    std::size_t property_count_ = 0;
};

template<typename T>
inline void detail::StatePropertyBase::backupValue(StateProperty<T>& property)
{
    state_.backupValue(property);
}

template<typename T>
inline detail::StateProperty<T>& detail::StateProperty<T>::operator=(const T& value)
{
    if (value_ != value) {
        StatePropertyBase::backupValue(*this);
        value_ = value;
        update();
    }
    return *this;
}

inline detail::StateBackupBase::~StateBackupBase()
{
}

template<typename T>
inline void State::backupValue(detail::StateProperty<T>& property)
{
    if (state_backup_.empty())
        return;

    detail::StateBackupSet& change_set = state_backup_.top();
    if (change_set.find(property.index_) != change_set.end())
        return;

    change_set.emplace(property.index_, std::make_unique<detail::StateBackup<T>>(property));
}

}
