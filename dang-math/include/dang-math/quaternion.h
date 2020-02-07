#pragma once

#include "utils.h"

#include "vector.h"
#include "matrix.h"

namespace dang::math
{

template <typename T>
struct Quaternion : private Vector<T, 4> {
    using Base = Vector<T, 4>;

    inline constexpr Quaternion() : Base() {}
    inline constexpr Quaternion(const Base& vector) : Base(vector) {}
    inline constexpr Quaternion(T scalar, const Vector<T, 3>& vector) : Quaternion(Base(vector, scalar)) {}
    inline constexpr Quaternion(T w, T x, T y, T z) : Quaternion(Base(x, y, z, w)) {}

    static inline constexpr Quaternion zero()
    {
        return Quaternion(0, 0, 0, 0);
    }

    static inline constexpr Quaternion identity()
    {
        return Quaternion(1, 0, 0, 0);
    }

    static inline constexpr Quaternion fromAxisRad(const Vector<T, 3>& normal, T radians)
    {
        radians /= T(2);
        T sin_radians = std::sin(radians);
        return Quaternion(
            std::cos(radians),
            normal.x() * sin_radians,
            normal.y() * sin_radians,
            normal.z() * sin_radians);
    }

    static inline constexpr Quaternion fromAxis(const Vector<T, 3>& normal, T degrees)
    {
        return fromAxisRad(normal, degToRad(degrees));
    }

    inline constexpr T scalar() const { return Base::w(); }
    inline constexpr T w() const { return Base::w(); }

    inline constexpr Vector<T, 3> vector() const { return Base::xyz(); };
    inline constexpr T x() const { return Base::x(); }
    inline constexpr T y() const { return Base::y(); }
    inline constexpr T z() const { return Base::z(); }

    using Base::dot;
    using Base::sqrdot;

    inline constexpr Quaternion normalize() const
    {
        return Base::normalize();
    }

    inline constexpr T magnitude() const
    {
        return Base::length();
    }

    inline constexpr Quaternion conjugate() const
    {
        return Quaternion(scalar(), -vector());
    }

    inline constexpr Quaternion inverse() const
    {
        return conjugate() / Base::sqrdot();
    }

    inline constexpr Matrix<T, 3> toMatrix() const
    {
        const T& w = this->w();
        const T& x = this->x();
        const T& y = this->y();
        const T& z = this->z();
        return Matrix<T, 3>({
            { T(1) - T(2) * y * y - T(2) * z * z, T(2) * x * y + T(2) * z * w, T(2) * x * z - T(2) * y * w },
            { T(2) * x * y - T(2) * z * w, T(1) - T(2) * x * x - T(2) * z * z, T(2) * y * z + T(2) * x * w },
            { T(2) * x * z + T(2) * y * w, T(2) * y * z - T(2) * x * w, T(1) - T(2) * x * x - T(2) * y * y } });
    }

    inline constexpr Quaternion operator+() const
    {
        return *this;
    }

    inline constexpr Quaternion operator-() const
    {
        return Base::operator-();
    }

    friend inline constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base(lhs) + Base(rhs);
    }

    friend inline constexpr Quaternion& operator+=(Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs = lhs + rhs;
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
        return vector + T(2) * ((quaternion.scalar() * uv) + u.cross(uv));
    }

    friend inline constexpr Vector<T, 3> operator*(const Vector<T, 3>& vector, const Quaternion& quaternion)
    {
        return quaternion.conjugate() * vector;
    }

    friend inline constexpr Vector<T, 3>& operator*=(Vector<T, 3>& vector, const Quaternion& quaternion)
    {
        return vector = vector * quaternion;
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

    inline constexpr DualQuaternion()
        : real(Quaternion<T>::identity())
        , dual(Quaternion<T>::zero())
    {
    }

    inline constexpr explicit DualQuaternion(const Quaternion<T>& real, const Quaternion<T>& dual = Quaternion<T>::zero())
        : real(real)
        , dual(dual)
    {
    }

    inline constexpr explicit DualQuaternion(const Vector<T, 3>& dual)
        : real(Quaternion<T>::identity())
        , dual(T(), dual)
    {
    }

    static inline constexpr DualQuaternion fromAxisRad(const Vector<T, 3>& normal, T radians)
    {
        return Quaternion<T>::fromAxisRad(normal, radians);
    }

    static inline constexpr DualQuaternion fromAxis(const Vector<T, 3>& normal, T degrees)
    {
        return DualQuaternion(Quaternion<T>::fromAxis(normal, degrees));
    }

    static inline constexpr DualQuaternion fromTranslation(const Vector<T, 3>& offset)
    {
        return DualQuaternion(Quaternion<T>::identity(), Vector<T, 4>(offset / T(2), 0));
    }

    inline constexpr DualQuaternion quatConjugate() const
    {
        return DualQuaternion(real.conjugate(), dual.conjugate());
    }

    inline constexpr DualQuaternion dualConjugate() const
    {
        return DualQuaternion(real, -dual);
    }

    inline constexpr DualQuaternion conjugate() const
    {
        return DualQuaternion(real.conjugate(), -dual.conjugate());
    }

    inline constexpr Quaternion<T> rotation() const
    {
        return real;
    }

    inline constexpr Vector<T, 3> translation() const
    {
        return T(2) * (dual * real.conjugate()).vector();
    }

    inline constexpr DualQuaternion normalize() const
    {
        return *this / real.magnitude();
    }

    inline constexpr Quaternion<T> dot(const DualQuaternion& other) const
    {
        return real.dot(other.real);
    }

    inline constexpr DualQuaternion inverse() const
    {
        Quaternion<T> realInverse = real.inverse();
        return DualQuaternion(realInverse, -realInverse * dual * realInverse);
    }

    inline constexpr DualQuaternion rotateRad(const Vector<T, 3>& normal, T radians) const
    {
        return *this * fromAxisRad(normal, radians);
    }

    inline constexpr DualQuaternion rotate(const Vector<T, 3>& normal, T degrees) const
    {
        return *this * fromAxis(normal, degrees);
    }

    inline constexpr DualQuaternion translate(const Vector<T, 3>& offset) const
    {
        return *this * fromTranslation(offset);
    }

    inline constexpr Matrix<T, 4> toMatrix() const
    {
        Matrix<T, 4> result;
        result.setSubMatrix<0, 0, 3, 3>(real.toMatrix());
        result[3] = Vector<T, 4>(translation(), T(1));
        return result;
    }

    inline constexpr DualQuaternion operator+() const
    {
        return *this;
    }

    inline constexpr DualQuaternion operator-() const
    {
        return DualQuaternion(-real, -dual);
    }

    friend inline constexpr DualQuaternion& operator*=(DualQuaternion& dualquaternion, T factor)
    {
        dualquaternion.real *= factor;
        dualquaternion.dual *= factor;
        return dualquaternion;
    }

    friend inline constexpr DualQuaternion operator*(DualQuaternion dualquaternion, T factor)
    {
        return dualquaternion *= factor;
    }

    friend inline constexpr DualQuaternion& operator/=(DualQuaternion& dualquaternion, T factor)
    {
        dualquaternion.real /= factor;
        dualquaternion.dual /= factor;
        return dualquaternion;
    }

    friend inline constexpr DualQuaternion operator/(DualQuaternion dualquaternion, T factor)
    {
        return dualquaternion /= factor;
    }

    friend inline constexpr DualQuaternion& operator+=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        lhs.real += rhs.real;
        lhs.dual += rhs.dual;
        return lhs;
    }

    friend inline constexpr DualQuaternion operator+(DualQuaternion lhs, const DualQuaternion& rhs)
    {
        return lhs += rhs;
    }

    friend inline constexpr DualQuaternion& operator-=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        lhs.real -= rhs.real;
        lhs.dual -= rhs.dual;
        return lhs;
    }

    friend inline constexpr DualQuaternion operator-(DualQuaternion lhs, const DualQuaternion& rhs)
    {
        return lhs -= rhs;
    }

    friend inline constexpr DualQuaternion operator*(const DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return DualQuaternion(rhs.real * lhs.real, rhs.dual * lhs.real + rhs.real * lhs.dual);
    }

    friend inline constexpr DualQuaternion& operator*=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return lhs = lhs * rhs;
    }

    friend inline constexpr DualQuaternion& operator/=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return lhs *= rhs.inverse;
    }

    friend inline constexpr DualQuaternion operator/(DualQuaternion lhs, const DualQuaternion& rhs)
    {
        return lhs /= rhs;
    }

    friend inline constexpr Vector<T, 3> operator*(const DualQuaternion& dualquaternion, const Vector<T, 3>& vector)
    {
        return (dualquaternion.conjugate() * DualQuaternion(vector) * dualquaternion).dual.vector();
    }
};

using quat = Quaternion<float>;
using dquat = DualQuaternion<float>;

}
