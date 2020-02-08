#pragma once

namespace dang::gl
{

class ObjectBase abstract {
protected:
    GLuint handle_;
};

template <class TInfo>
class Object abstract : public ObjectBase {
public:
    Object()
    {
        handle_ = TInfo::create();
    }

    ~Object()
    {
        TInfo::destroy(handle_);
    }
};

}
