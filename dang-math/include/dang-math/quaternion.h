#pragma once

#include "utils.h"

#include "vector.h"
#include "matrix.h"

namespace dang::math
{

template <typename T>
struct Quaternion : private Vector<T, 4> {
    using Base = Vector<T, 4>;

    inline constexpr Quaternion() : Base(T(1), T(), T(), T()) {}
    inline constexpr Quaternion(const Base& vector) : Base(vector) {}
    inline constexpr Quaternion(T scalar, const Vector<T, 3>& vector) : Quaternion(Base(vector, scalar)) {}
    inline constexpr Quaternion(T w, T x, T y, T z) : Quaternion(Base(x, y, z, w)) {}

    inline constexpr T scalar() const { return Base::w(); }
    inline constexpr T w() const { return Base::w(); }

    inline constexpr Vector<T, 3> vector() const { return Base::xyz(); };
    inline constexpr T x() const { return Base::x(); }
    inline constexpr T y() const { return Base::y(); }
    inline constexpr T z() const { return Base::z(); }

    using Base::dot;
    using Base::normalized;
    using Base::operator+;
    using Base::operator-;

    inline constexpr T conjugate() const
    {
        return Quaternion(scalar(), -vector());
    }

    inline constexpr Quaternion inverse() const
    {
        auto l = Base::sqrdot();
        return Quaternion(scalar() / l, -vector() / l);
    }

    friend inline constexpr Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Quaternion(
            (lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z()),
            (lhs.w() * rhs.x() + lhs.x() * rhs.w() + lhs.y() * rhs.z() - lhs.z() * rhs.y()),
            (lhs.w() * rhs.y() - lhs.x() * rhs.z() + lhs.y() * rhs.w() + lhs.z() * rhs.x()),
            (lhs.w() * rhs.z() + lhs.x() * rhs.y() - lhs.y() * rhs.x() + lhs.z() * rhs.w()));
    }

    friend inline constexpr Quaternion& operator*=(Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs = lhs * rhs;
    }

    friend inline constexpr Quaternion operator/(const Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs * rhs.inverse();
    }

    friend inline constexpr Quaternion& operator/=(Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs = lhs / rhs;
    }

    friend inline constexpr Vector<T, 3> operator*(const Quaternion& quaternion, const Vector<T, 3>& vector)
    {
        Vector<T, 3> u = quaternion.vector();
        Vector<T, 3> uv = u.cross(vector);
        Vector<T, 3> uuv = u.cross(uv);
        return vector + T(2) * ((quaternion.scalar() * uv) + uuv);
    }

    friend inline constexpr Quaternion operator*(const Quaternion& quaternion, T factor)
    {
        return Base(quaternion) * factor;
    }

    friend inline constexpr Quaternion operator*(T factor, const Quaternion& quaternion)
    {
        return Base(quaternion) * factor;
    }

    friend inline constexpr Quaternion& operator*=(Quaternion& quaternion, T factor)
    {
        return quaternion = quaternion * factor;
    }

    friend inline constexpr Quaternion operator/(const Quaternion& quaternion, T factor)
    {
        return Base(quaternion) / factor;
    }

    friend inline constexpr Quaternion operator/(T factor, const Quaternion& quaternion)
    {
        return factor / Base(quaternion);
    }

    friend inline constexpr Quaternion& operator/=(Quaternion& quaternion, T factor)
    {
        return quaternion = quaternion / factor;
    }

    friend inline constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base::operator==(lhs, rhs);
    }

    friend inline constexpr bool operator!=(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base::operator!=(lhs, rhs);
    }
};

template <typename T>
struct DualQuaternion {
    Quaternion<T> real;
    Quaternion<T> dual;

    inline constexpr DualQuaternion() = default;
    inline constexpr DualQuaternion(const Quaternion<T>& real, const Quaternion<T>& dual) : real(real), dual(dual) {}

    inline constexpr DualQuaternion conjugate1()
    {
        return DualQuaternion(real, -dual);
    }

    inline constexpr DualQuaternion conjugate2()
    {
        return DualQuaternion(real.conjugate(), dual.conjugate());
    }

    inline constexpr DualQuaternion conjugate3()
    {
        return DualQuaternion(real.conjugate(), -dual.conjugate());
    }

    inline constexpr const DualQuaternion& operator+()
    {
        return *this;
    }

    inline constexpr DualQuaternion operator-()
    {
        return DualQuaternion(-real, -dual);
    }

    friend inline constexpr DualQuaternion operator+(const DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return DualQuaternion(lhs.real + rhs.real, lhs.dual + rhs.dual);
    }

    friend inline constexpr DualQuaternion operator-(const DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return DualQuaternion(lhs.real - rhs.real, lhs.dual - rhs.dual);
    }
};

using quat = Quaternion<float>;
using dquat = DualQuaternion<float>;

}
