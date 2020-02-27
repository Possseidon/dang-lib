#pragma once

#include "utils.h"

#include <type_traits>

#define META_INTF_ABSTRACT(name) \
protected: virtual dang::utils::BaseClassRef metaclass_v() const = 0; \
public: dang::utils::ClassRef<name> metaclass() const; \
private:

#define META_INTF(name) \
protected: virtual dang::utils::BaseClassRef metaclass_v() const; \
public: dang::utils::ClassRef<name> metaclass() const; \
private:

#define META_IMPL(name) \
dang::utils::BaseClassRef name::metaclass_v() const { return dang::utils::ClassOf<name>; } \
dang::utils::ClassRef<name> name::metaclass() const { return static_cast<dang::utils::ClassRef<name>>(metaclass_v()); } 

#define META_IMPL_ABSTRACT(name) \
dang::utils::ClassRef<name> name::metaclass() const { return static_cast<dang::utils::ClassRef<name>>(metaclass_v()); } 

namespace dang::utils
{

template <class T>
struct Class;

template <class T>
using ClassRef = const Class<T>&;

template <class T>
using ClassPtr = const Class<T>*;

struct BaseClass {
    BaseClass() = default;
    BaseClass(const BaseClass&) = delete;
    BaseClass(BaseClass&&) = delete;
    BaseClass& operator=(const BaseClass&) = delete;
    BaseClass& operator=(BaseClass&&) = delete;
    virtual ~BaseClass() = 0;
};

inline BaseClass::~BaseClass()
{
}

using BaseClassRef = const BaseClass&;

using BaseClassPtr = const BaseClass*;

template <class T, typename = std::enable_if_t<std::is_base_of_v<BaseClass, Class<T>>>>
const Class<T> ClassOf;

}
