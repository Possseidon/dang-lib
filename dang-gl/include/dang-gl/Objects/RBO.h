#pragma once

#include "dang-gl/Image/PixelInternalFormat.h"
#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/Objects/Object.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/RenderbufferContext.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// @brief A renderbuffer object containing image data, specifically used together with framebuffer objects.
/// @remark Natively supports multisampling.
class RBO : public ObjectBindable<ObjectType::Renderbuffer> {
public:
    RBO(EmptyObject)
        : ObjectBindable<ObjectType::Renderbuffer>(empty_object)
    {}

    /// @brief Initializes the renderbuffer with the given size, format and optional multisampling-count.
    RBO(svec2 size, GLsizei samples, PixelInternalFormat format);

    ~RBO() = default;

    RBO(const RBO&) = delete;
    RBO(RBO&&) = default;
    RBO& operator=(const RBO&) = delete;
    RBO& operator=(RBO&&) = default;

    static RBO color(svec2 size, GLsizei samples = 0);
    static RBO depth(svec2 size, GLsizei samples = 0);
    static RBO depthStencil(svec2 size, GLsizei samples = 0);
    static RBO stencil(svec2 size, GLsizei samples = 0);

    /// @brief Returns the width and height of the renderbuffer.
    svec2 size() const;
    /// @brief Returns the sample count for multisampled renderbuffers or zero for non-multisampled ones.
    GLsizei samples() const;
    /// @brief Returns the pixel format of the renderbuffer.
    PixelInternalFormat format() const;

private:
    svec2 size_;
    GLsizei samples_;
    PixelInternalFormat format_;
};

} // namespace dang::gl
