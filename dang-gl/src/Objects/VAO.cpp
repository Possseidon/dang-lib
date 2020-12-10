#include "Objects/VAO.h"

namespace dang::gl {

VAOBase::VAOBase(Program& program, BeginMode mode)
    : program_(&program)
    , mode_(mode)
{}

Program& VAOBase::program() const { return *program_; }

BeginMode VAOBase::mode() const { return mode_; }

void VAOBase::setMode(BeginMode mode) { mode_ = mode; }

} // namespace dang::gl
