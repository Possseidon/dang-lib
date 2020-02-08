#pragma once

#include "Object.h"

namespace dang::gl
{

struct ProgramInfo {       
    static GLuint create();
    static void destroy(GLuint handle);
};

class Program : public Object<ProgramInfo> {

};

}
