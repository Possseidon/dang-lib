#pragma once

#include "dang-utils/enum.h"

#include "GLConstants.h"
#include "MathTypes.h"

namespace dang::gl
{

/// <summary>The source factor, used for the blending function.</summary>
enum class BlendFactorSrc {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,

    COUNT
};

template <>
constexpr dutils::EnumArray<BlendFactorSrc, GLenum> GLConstants<BlendFactorSrc> = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_CONSTANT_COLOR,
    GL_ONE_MINUS_CONSTANT_COLOR,
    GL_CONSTANT_ALPHA,
    GL_ONE_MINUS_CONSTANT_ALPHA,
    GL_SRC_ALPHA_SATURATE
};

/// <summary>The destination factor, used for the blending function.</summary>
enum class BlendFactorDst {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,

    COUNT
};

template <>
constexpr dutils::EnumArray<BlendFactorDst, GLenum> GLConstants<BlendFactorDst> = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_CONSTANT_COLOR,
    GL_ONE_MINUS_CONSTANT_COLOR,
    GL_CONSTANT_ALPHA,
    GL_ONE_MINUS_CONSTANT_ALPHA
};

/// <summary>Used by functions to compare values.</summary>
enum class CompareFunc {
    Never,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    Always,

    COUNT
};

template <>
constexpr dutils::EnumArray<CompareFunc, GLenum> GLConstants<CompareFunc> = {
    GL_NEVER,
    GL_LESS,
    GL_LEQUAL,
    GL_GREATER,
    GL_GEQUAL,
    GL_EQUAL,
    GL_NOTEQUAL,
    GL_ALWAYS
};

/// <summary>Which face to hide/cull.</summary>
enum class CullFaceMode {
    Front,
    Back,
    FrontAndBack,

    COUNT
};

template <>
constexpr dutils::EnumArray<CullFaceMode, GLenum> GLConstants<CullFaceMode> = {
    GL_FRONT,
    GL_BACK,
    GL_FRONT_AND_BACK
};

/// <summary>A list of all binary operations of boolean algebra.</summary>
enum class LogicOp {
    Clear,
    Set,
    Copy,
    Copy_inverted,
    Noop,
    Invert,
    And,
    Nand,
    Or,
    Nor,
    Xor,
    Equiv,
    AndReverse,
    AndInverted,
    OrReverse,
    OrInverted,

    COUNT
};

template <>
constexpr dutils::EnumArray<LogicOp, GLenum> GLConstants<LogicOp> = {
    GL_CLEAR,
    GL_SET,
    GL_COPY,
    GL_COPY_INVERTED,
    GL_NOOP,
    GL_INVERT,
    GL_AND,
    GL_NAND,
    GL_OR,
    GL_NOR,
    GL_XOR,
    GL_EQUIV,
    GL_AND_REVERSE,
    GL_AND_INVERTED,
    GL_OR_REVERSE,
    GL_OR_INVERTED
};

/// <summary>Specifies the side of a polygon.</summary>
enum class PolygonSide {
    Front,
    Back,

    COUNT
};

template <>
constexpr dutils::EnumArray<PolygonSide, GLenum> GLConstants<PolygonSide> = {
    GL_FRONT,
    GL_BACK
};

/// <summary>Specfies, wether polygons should render full faces, the outline or just corner points.</summary>
enum class PolygonMode {
    Point,
    Line,
    Fill,

    COUNT
};

template <>
constexpr dutils::EnumArray<PolygonMode, GLenum> GLConstants<PolygonMode> = {
    GL_POINT,
    GL_LINE,
    GL_FILL,
};

/// <summary>Actions, which can be performed on the stencil buffer.</summary>
enum class StencilAction {
    Keep,
    Zero,
    Replace,
    Incr,
    IncrWrap,
    Decr,
    DecrWrap,
    Invert,

    COUNT
};

template <>
constexpr dutils::EnumArray<StencilAction, GLenum> GLConstants<StencilAction> = {
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,
    GL_INCR_WRAP,
    GL_DECR,
    GL_DECR_WRAP,
    GL_INVERT
};

/// <summary>Combines blending source and destination factors into a single type.</summary>
struct BlendFactor {
    BlendFactorSrc src;
    BlendFactorDst dst;

    friend bool operator==(const BlendFactor& lhs, const BlendFactor& rhs);
    friend bool operator!=(const BlendFactor& lhs, const BlendFactor& rhs);

    std::tuple<GLenum, GLenum> toTuple() const;
};

/// <summary>Which polygon mode to use for a template specified polygon side.</summary>
template <PolygonSide Side>
struct PolygonSideMode {
    PolygonMode mode;

    PolygonSideMode(PolygonMode mode) : mode(mode) {}

    friend bool operator==(const PolygonSideMode& lhs, const PolygonSideMode& rhs) { return lhs.mode == rhs.mode; }
    friend bool operator!=(const PolygonSideMode& lhs, const PolygonSideMode& rhs) { return !(lhs == rhs); }

    std::tuple<GLenum, GLenum> toTuple() const { return { GLConstants<PolygonSide>[Side], GLConstants<PolygonMode>[mode] }; }
};

/// <summary>Polygon offset, consisting of a factor and units value.</summary>
struct PolygonOffset {
    GLfloat factor;
    GLfloat units;

    friend bool operator==(const PolygonOffset& lhs, const PolygonOffset& rhs);
    friend bool operator!=(const PolygonOffset& lhs, const PolygonOffset& rhs);

    std::tuple<GLfloat, GLfloat> toTuple() const;
};

/// <summary>A sample coverage, consisting of a clamped value and an invert flag.</summary>
struct SampleCoverage {
    GLclampf value;
    GLboolean invert;

    friend bool operator==(const SampleCoverage& lhs, const SampleCoverage& rhs);
    friend bool operator!=(const SampleCoverage& lhs, const SampleCoverage& rhs);

    std::tuple<GLclampf, GLboolean> toTuple() const;
};

/// <summary>Two-dimensional integer bounds for a scissor-test.</summary>
struct Scissor {
    ibounds2 bounds;

    friend bool operator==(const Scissor& lhs, const Scissor& rhs);
    friend bool operator!=(const Scissor& lhs, const Scissor& rhs);

    std::tuple<GLint, GLint, GLsizei, GLsizei> toTuple() const;
};

/// <summary>The full parameter-set to change the stencil function with reference value and bit mask.</summary>
struct StencilFunc {
    CompareFunc func;
    GLint ref;
    GLuint mask;

    friend bool operator==(const StencilFunc& lhs, const StencilFunc& rhs);
    friend bool operator!=(const StencilFunc& lhs, const StencilFunc& rhs);

    std::tuple<GLenum, GLint, GLuint> toTuple() const;
};

/// <summary>A set of stencil actions, which should be executed for different checks.</summary>
struct StencilOp {
    StencilAction sfail;
    StencilAction dpfail;
    StencilAction dppass;

    friend bool operator==(const StencilOp& lhs, const StencilOp& rhs);
    friend bool operator!=(const StencilOp& lhs, const StencilOp& rhs);

    std::tuple<GLenum, GLenum, GLenum> toTuple() const;
};

}
