#pragma once

#include "GLFW.h"

namespace dang::gl
{

struct ProgramInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr ObjectType Type = ObjectType::Program;
};

class Program : public Object<ProgramInfo> {
public:

};

}
