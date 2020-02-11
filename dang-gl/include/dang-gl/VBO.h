#pragma once

#include "Object.h"

namespace dang::gl
{

struct VBOInfo : public ObjectInfo {
    static GLuint create();
    static void destroy(GLuint handle);
    static void bind(GLuint handle);

    static constexpr ObjectType Type = ObjectType::Buffer;
};

class VBOBase : public Object<VBOInfo> {

};

template <typename T>
class VBO : public VBOBase {
    static_assert(std::is_pod_v<T>, "VBO-Data must be a pod type");
};

}
