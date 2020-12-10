#pragma once

#include "dang-gl/Image/PixelInternalFormat.h"
#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/Objects/Object.h"
#include "dang-gl/Objects/ObjectType.h"
#include "dang-gl/Objects/RenderbufferContext.h"
#include "dang-gl/global.h"

namespace dang::gl {

/// <summary>A renderbuffer object containing image data, specifially used together with framebuffer objects.</summary>
/// <remarks>Natively supports multisampling.</remarks>
class RBO : public ObjectBindable<ObjectType::Renderbuffer> {
public:
    /// <summary>Initializes the renderbuffer with the given size, format and optional multisampling-count.</summary>
    explicit RBO(svec2 size, GLsizei samples, PixelInternalFormat format);
    ~RBO() = default;

    RBO(const RBO&) = delete;
    RBO(RBO&&) = default;
    RBO& operator=(const RBO&) = delete;
    RBO& operator=(RBO&&) = default;

    static RBO color(svec2 size, GLsizei samples = 0);
    static RBO depth(svec2 size, GLsizei samples = 0);
    static RBO depthStencil(svec2 size, GLsizei samples = 0);
    static RBO stencil(svec2 size, GLsizei samples = 0);

    /// <summary>Returns the width and height of the renderbuffer.</summary>
    svec2 size() const;
    /// <summary>Returns the sample count for multisampled renderbuffers or zero for non-multisampled ones.</summary>
    GLsizei samples() const;
    /// <summary>Returns the pixel format of the renderbuffer.</summary>
    PixelInternalFormat format() const;

private:
    svec2 size_;
    GLsizei samples_;
    PixelInternalFormat format_;
};

} // namespace dang::gl
