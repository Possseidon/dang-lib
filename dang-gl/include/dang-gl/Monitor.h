#pragma once

#include "dang-math/vector.h"
#include "dang-math/bounds.h"

namespace dang::gl
{

using GammaRamp = GLFWgammaramp;
using VideoMode = GLFWvidmode;

class Monitor {
public:
    Monitor() = default;
    Monitor(GLFWmonitor* monitor);

    GLFWmonitor* handle() const;
    operator GLFWmonitor* () const;

    std::string name() const;
    dmath::ivec2 physicalSize() const;
    dmath::vec2 contentScale() const;
    dmath::ivec2 pos() const;
    dmath::ibounds2 workarea() const;

    void setGamma(float gamma) const;
    void setGammaRamp(const GammaRamp* gamma_ramp) const;
    const GammaRamp* gammaRamp() const;

    VideoMode videoMode() const;
    std::vector<VideoMode> videoModes() const;

private:
    GLFWmonitor* handle_ = nullptr;
};

}
