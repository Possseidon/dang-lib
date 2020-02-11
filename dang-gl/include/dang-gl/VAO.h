#pragma once

#include "Object.h"

#include "VBO.h"

namespace dang::gl
{

class Program;

struct VAOInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr ObjectType Type = ObjectType::Buffer;
};

class VAOBase : public Object<VAOInfo> {
public:
    VAOBase(Program& program);

private:
    Program& program_;
};

template <typename T>
class VAO : public VAOBase {
public:

private:
    VBO<T>& vbo_;
};

}
