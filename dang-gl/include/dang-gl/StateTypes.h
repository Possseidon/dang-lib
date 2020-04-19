#pragma once

#include "Types.h"

namespace dang::gl
{

enum class BlendFactorSrc : GLenum {
    Zero = GL_ZERO,
    One = GL_ONE,
    SrcColor = GL_SRC_COLOR,
    OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
    DstColor = GL_DST_COLOR,
    OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
    SrcAlpha = GL_SRC_ALPHA,
    OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
    DstAlpha = GL_DST_ALPHA,
    OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
    ConstantColor = GL_CONSTANT_COLOR,
    OneMinusConstantColor = GL_ONE_MINUS_CONSTANT_COLOR,
    ConstantAlpha = GL_CONSTANT_ALPHA,
    OneMinusConstantAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
    SrcAlphaSaturate = GL_SRC_ALPHA_SATURATE
};

enum class BlendFactorDst : GLenum {
    Zero = GL_ZERO,
    One = GL_ONE,
    SrcColor = GL_SRC_COLOR,
    OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
    DstColor = GL_DST_COLOR,
    OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
    SrcAlpha = GL_SRC_ALPHA,
    OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
    DstAlpha = GL_DST_ALPHA,
    OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
    ConstantColor = GL_CONSTANT_COLOR,
    OneMinusConstantColor = GL_ONE_MINUS_CONSTANT_COLOR,
    ConstantAlpha = GL_CONSTANT_ALPHA,
    OneMinusConstantAlpha = GL_ONE_MINUS_CONSTANT_ALPHA
};

enum class CompareFunc : GLenum {
    Never = GL_NEVER,
    Less = GL_LESS,
    LessEqual = GL_LEQUAL,
    Greater = GL_GREATER,
    GreaterEqual = GL_GEQUAL,
    Equal = GL_EQUAL,
    NotEqual = GL_NOTEQUAL,
    Always = GL_ALWAYS
};

enum class CullFaceMode : GLenum {
    Front = GL_FRONT,
    Back = GL_BACK,
    FrontAndBack = GL_FRONT_AND_BACK
};

enum class LogicOp : GLenum {
    Clear = GL_CLEAR,
    Set = GL_SET,
    Copy = GL_COPY,
    Copy_inverted = GL_COPY_INVERTED,
    Noop = GL_NOOP,
    Invert = GL_INVERT,
    And = GL_AND,
    Nand = GL_NAND,
    Or = GL_OR,
    Nor = GL_NOR,
    Xor = GL_XOR,
    Equiv = GL_EQUIV,
    AndReverse = GL_AND_REVERSE,
    AndInverted = GL_AND_INVERTED,
    OrReverse = GL_OR_REVERSE,
    OrInverted = GL_OR_INVERTED
};

enum class PolygonSide : GLenum {
    Front = GL_FRONT,
    Back = GL_BACK
};

enum class PolygonMode : GLenum {
    Point = GL_POINT,
    Line = GL_LINE,
    Fill = GL_FILL
};

enum class StencilAction : GLenum {
    Keep = GL_KEEP,
    Zero = GL_ZERO,
    Replace = GL_REPLACE,
    Incr = GL_INCR,
    IncrWrap = GL_INCR_WRAP,
    Decr = GL_DECR,
    DecrWrap = GL_DECR_WRAP,
    Invert = GL_INVERT
};

struct BlendFactor {
    BlendFactorSrc src;
    BlendFactorDst dst;

    friend bool operator==(const BlendFactor& lhs, const BlendFactor& rhs);
    friend bool operator!=(const BlendFactor& lhs, const BlendFactor& rhs);

    std::tuple<GLenum, GLenum> toTuple() const;
};

template <PolygonSide Side>
struct PolygonSideMode {
    PolygonMode mode;

    PolygonSideMode(PolygonMode mode) : mode(mode) {}

    friend bool operator==(const PolygonSideMode& lhs, const PolygonSideMode& rhs) { return lhs.mode == rhs.mode; }
    friend bool operator!=(const PolygonSideMode& lhs, const PolygonSideMode& rhs) { return !(lhs == rhs); }

    std::tuple<GLenum, GLenum> toTuple() const { return { static_cast<GLenum>(Side), static_cast<GLenum>(mode) }; }
};

struct PolygonOffset {
    GLfloat factor;
    GLfloat units;

    friend bool operator==(const PolygonOffset& lhs, const PolygonOffset& rhs);
    friend bool operator!=(const PolygonOffset& lhs, const PolygonOffset& rhs);

    std::tuple<GLfloat, GLfloat> toTuple() const;
};

struct SampleCoverage {
    GLclampf value;
    GLboolean invert;

    friend bool operator==(const SampleCoverage& lhs, const SampleCoverage& rhs);
    friend bool operator!=(const SampleCoverage& lhs, const SampleCoverage& rhs);

    std::tuple<GLclampf, GLboolean> toTuple() const;
};

struct Scissor {
    ibounds2 bounds;

    friend bool operator==(const Scissor& lhs, const Scissor& rhs);
    friend bool operator!=(const Scissor& lhs, const Scissor& rhs);

    std::tuple<GLint, GLint, GLsizei, GLsizei> toTuple() const;
};

struct StencilFunc {
    CompareFunc func;
    GLint ref;
    GLuint mask;

    friend bool operator==(const StencilFunc& lhs, const StencilFunc& rhs);
    friend bool operator!=(const StencilFunc& lhs, const StencilFunc& rhs);

    std::tuple<GLenum, GLint, GLuint> toTuple() const;
};

struct StencilOp {
    StencilAction sfail;
    StencilAction dpfail;
    StencilAction dppass;

    friend bool operator==(const StencilOp& lhs, const StencilOp& rhs);
    friend bool operator!=(const StencilOp& lhs, const StencilOp& rhs);

    std::tuple<GLenum, GLenum, GLenum> toTuple() const;
};

}
