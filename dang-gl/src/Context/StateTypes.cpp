#include "dang-gl/Context/StateTypes.h"

namespace dang::gl {

bool operator==(const BlendFactor& lhs, const BlendFactor& rhs)
{
    return std::tie(lhs.src, lhs.dst) == std::tie(rhs.src, rhs.dst);
}

bool operator!=(const BlendFactor& lhs, const BlendFactor& rhs) { return !(lhs == rhs); }

std::tuple<GLenum, GLenum> BlendFactor::toTuple() const { return {toGLConstant(src), toGLConstant(dst)}; }

bool operator==(const PolygonOffset& lhs, const PolygonOffset& rhs)
{
    return std::tie(lhs.factor, lhs.units) == std::tie(rhs.factor, rhs.units);
}

bool operator!=(const PolygonOffset& lhs, const PolygonOffset& rhs) { return !(lhs == rhs); }

std::tuple<GLfloat, GLfloat> PolygonOffset::toTuple() const { return {factor, units}; }

bool operator==(const SampleCoverage& lhs, const SampleCoverage& rhs)
{
    return std::tie(lhs.value, lhs.invert) == std::tie(rhs.value, rhs.invert);
}

bool operator!=(const SampleCoverage& lhs, const SampleCoverage& rhs) { return !(lhs == rhs); }

std::tuple<GLclampf, GLboolean> SampleCoverage::toTuple() const { return {value, invert}; }

bool operator==(const Scissor& lhs, const Scissor& rhs) { return lhs.bounds == rhs.bounds; }

bool operator!=(const Scissor& lhs, const Scissor& rhs) { return !(lhs == rhs); }

std::tuple<GLint, GLint, GLsizei, GLsizei> Scissor::toTuple() const
{
    const auto& size = bounds.size();
    return {bounds.low.x(), bounds.low.y(), size.x(), size.y()};
}

bool operator==(const StencilFunc& lhs, const StencilFunc& rhs)
{
    return std::tie(lhs.func, lhs.ref, lhs.mask) == std::tie(rhs.func, rhs.ref, rhs.mask);
}

bool operator!=(const StencilFunc& lhs, const StencilFunc& rhs) { return !(lhs == rhs); }

std::tuple<GLenum, GLint, GLuint> StencilFunc::toTuple() const { return {toGLConstant(func), ref, mask}; }

bool operator==(const StencilOp& lhs, const StencilOp& rhs)
{
    return std::tie(lhs.sfail, lhs.dpfail, lhs.dppass) == std::tie(rhs.sfail, rhs.dpfail, rhs.dppass);
}

bool operator!=(const StencilOp& lhs, const StencilOp& rhs) { return !(lhs == rhs); }

std::tuple<GLenum, GLenum, GLenum> StencilOp::toTuple() const
{
    return {toGLConstant(sfail), toGLConstant(dpfail), toGLConstant(dppass)};
}

} // namespace dang::gl
