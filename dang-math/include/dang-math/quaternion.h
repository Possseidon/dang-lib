#pragma once

#include "utils.h"

#include "matrix.h"
#include "vector.h"

namespace dang::math
{

/// <summary>A quaternion, which usually represents an arbitrary rotation in three-dimensional space.</summary>
/// <remarks>
/// <para>The rotation is expressed using an axis (vector/xyz) and a rotation distance (scalar/w).</para>
/// <para>The quaternion needs to be normalized, before applying it (using multiplication).</para>
/// </remarks>
template <typename T>
struct Quaternion : private Vector<T, 4> {
    using Base = Vector<T, 4>;

    /// <summary>Initializes the quaternion to the zero quaternion, which cannot be normalized or used directly.</summary>
    constexpr Quaternion() : Base() {}
    /// <summary>Initializes the quaternion from a four-dimensional vector.</summary>
    constexpr Quaternion(const Base& vector) : Base(vector) {}
    /// <summary>Initializes the quaternion from scalar and vector without normalization.</summary>
    constexpr Quaternion(T scalar, const Vector<T, 3>& vector) : Quaternion(Base(vector, scalar)) {}
    /// <summary>Initializes the quaternion from w-scalar and xyz-vector without normalization.</summary>
    constexpr Quaternion(T w, T x, T y, T z) : Quaternion(Base(x, y, z, w)) {}

    /// <summary>Returns the zero-quaternion, which cannot be normalized or used directly.</summary>
    static constexpr Quaternion zero()
    {
        return Quaternion(0, 0, 0, 0);
    }

    /// <summary>Returns the identity-quaternion, which is normalized and, when applied, does not do anything.</summary>
    static constexpr Quaternion identity()
    {
        return Quaternion(1, 0, 0, 0);
    }

    /// <summary>Returns a quaternion from the given rotation, specified as rotation-axis and angle in radians.</summary>
    static constexpr Quaternion fromAxisRad(const Vector<T, 3>& normal, T radians)
    {
        radians /= 2;
        T sin_radians = std::sin(radians);
        return Quaternion(
            std::cos(radians),
            normal.x() * sin_radians,
            normal.y() * sin_radians,
            normal.z() * sin_radians);
    }

    /// <summary>Returns a quaternion from the given rotation, specified as rotation-axis and angle in degrees.</summary>
    static constexpr Quaternion fromAxis(const Vector<T, 3>& normal, T degrees)
    {
        return fromAxisRad(normal, degToRad(degrees));
    }

    /// <summary>Returns a quaternion with all euler angles in radians applied in the given order.</summary>
    template <std::size_t AngleCount>
    static constexpr Quaternion fromEulerRad(const Vector<T, AngleCount>& radians, const std::array<Axis3, AngleCount>& order)
    {
        Quaternion result = Quaternion::identity();
        for (std::size_t i = 0; i < AngleCount; i++)
            result *= fromAxisRad(static_cast<Vector<T, 3>>(AxisVector3[order[i]]), radians[i]);
        return result;
    }

    /// <summary>Returns a quaternion with all euler angles in degrees applied in the given order.</summary>
    template <std::size_t AngleCount>
    static constexpr Quaternion fromEuler(const Vector<T, AngleCount>& degrees, const std::array<Axis3, AngleCount>& order)
    {
        return fromEulerRad(degrees.degToRad(), order);
    }

    /// <summary>Returns a quaternion with all euler angles in radians applied in YXZ-order.</summary>
    static constexpr Quaternion fromEulerRad(const Vector<T, 3>& radians)
    {
        return fromEulerRad(radians, { Axis3::Y, Axis3::X, Axis3::Z });
    }

    /// <summary>Returns a quaternion with all euler angles in degrees applied in YXZ-order.</summary>
    static constexpr Quaternion fromEuler(const Vector<T, 3>& degrees)
    {
        return fromEulerRad(degrees.degToRad());
    }

    /// <summary>Returns a quaternion with all euler angles in radians applied in YX-order.</summary>
    static constexpr Quaternion fromEulerRad(const Vector<T, 2>& radians)
    {
        return fromEulerRad(radians, { Axis3::Y, Axis3::X });
    }

    /// <summary>Returns a quaternion with all euler angles in degrees applied in YX-order.</summary>
    static constexpr Quaternion fromEuler(const Vector<T, 2>& degrees)
    {
        return fromEulerRad(degrees.degToRad());
    }

    /// <summary>Returns the scalar/w part of the quaternion.</summary>
    constexpr T scalar() const { return Base::w(); }
    /// <summary>Returns the scalar/w part of the quaternion.</summary>
    constexpr T w() const { return Base::w(); }

    /// <summary>Returns the vector/xyz part of the quaternion.</summary>
    constexpr Vector<T, 3> vector() const { return Base::xyz(); };
    /// <summary>Returns the vector x-part of the quaternion.</summary>
    constexpr T x() const { return Base::x(); }
    /// <summary>Returns the vector y-part of the quaternion.</summary>
    constexpr T y() const { return Base::y(); }
    /// <summary>Returns the vector z-part of the quaternion.</summary>
    constexpr T z() const { return Base::z(); }

    using Base::dot;
    using Base::sqrdot;

    /// <summary>Returns the normalized quaternion, which can safely be applied using multiplication.</summary>
    constexpr Quaternion normalize() const
    {
        return Base::normalize();
    }

    /// <summary>Returns the magnitude of the quaternion, which is simply the length of the xyzw-vector.</summary>
    constexpr T magnitude() const
    {
        return Base::length();
    }

    /// <summary>Returns the conjugate of the quaternion, which simply has the vector-part negated.</summary>
    constexpr Quaternion conjugate() const
    {
        return Quaternion(scalar(), -vector());
    }

    /// <summary>Returns the inverse of the quaternion, assuming it is normalized, for which it simply calculates the conjugate.</summary>
    constexpr Quaternion inverseFast() const
    {
        return conjugate();
    }

    /// <summary>Returns the inverse of the quaternion, even if the quaternion is not normalized.</summary>
    constexpr Quaternion inverseSafe() const
    {
        return conjugate() / Base::sqrdot();
    }

    /// <summary>Converts the quaternion into a simple xyzw-vector.</summary>
    constexpr const Vector<T, 4>& asVector() const
    {
        return *this;
    }

    /// <summary>Converts the quaternion into a simple xyzw-vector.</summary>
    constexpr Vector<T, 4>& asVector()
    {
        return *this;
    }

    /// <summary>Converts the quaternion into a rotation-matrix and returns it.</summary>
    constexpr Matrix<T, 3> toMatrix() const
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

    /// <summary>Returns the quaternion itself.</summary>
    constexpr Quaternion operator+() const
    {
        return *this;
    }

    /// <summary>Returns the quaternion with both scalar and vector negated.</summary>
    constexpr Quaternion operator-() const
    {
        return Base::operator-();
    }

    /// <summary>Adds both quaternions component-wise.</summary>
    friend constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base(lhs) + Base(rhs);
    }

    /// <summary>Adds both quaternions component-wise.</summary>
    friend constexpr Quaternion& operator+=(Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs = lhs + rhs;
    }

    /// <summary>Adds both quaternions component-wise.</summary>
    friend constexpr Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base(lhs) - Base(rhs);
    }

    /// <summary>Adds both quaternions component-wise.</summary>
    friend constexpr Quaternion& operator-=(Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs = lhs - rhs;
    }

    /// <summary>Combines the transformation of both quaternions.</summary>
    friend constexpr Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Quaternion(
            (lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z()),
            (lhs.w() * rhs.x() + lhs.x() * rhs.w() + lhs.y() * rhs.z() - lhs.z() * rhs.y()),
            (lhs.w() * rhs.y() - lhs.x() * rhs.z() + lhs.y() * rhs.w() + lhs.z() * rhs.x()),
            (lhs.w() * rhs.z() + lhs.x() * rhs.y() - lhs.y() * rhs.x() + lhs.z() * rhs.w()));
    }

    /// <summary>Combines the transformation of rhs onto lhs.</summary>
    friend constexpr Quaternion& operator*=(Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs = lhs * rhs;
    }

    /// <summary>Combines the transformation of lhs with the inverse of rhs.</summary>
    friend constexpr Quaternion operator/(const Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs * rhs.inverseSafe();
    }

    /// <summary>Combines the transformation of the inverse of rhs onto lhs.</summary>
    friend constexpr Quaternion& operator/=(Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs = lhs / rhs;
    }

    /// <summary>Applies the quaternion transformation to the given vector.</summary>
    friend constexpr Vector<T, 3> operator*(const Quaternion& quaternion, const Vector<T, 3>& vector)
    {
        Vector<T, 3> u = quaternion.vector();
        Vector<T, 3> uv = u.cross(vector);
        return vector + T(2) * ((quaternion.scalar() * uv) + u.cross(uv));
    }

    /// <summary>Applies the transformation of the conjuagted quaternion to the given vector.</summary>
    friend constexpr Vector<T, 3> operator*(const Vector<T, 3>& vector, const Quaternion& quaternion)
    {
        return quaternion.conjugate() * vector;
    }

    /// <summary>Applies the transformation of the conjuagted quaternion onto the given vector.</summary>
    friend constexpr Vector<T, 3>& operator*=(Vector<T, 3>& vector, const Quaternion& quaternion)
    {
        return vector = vector * quaternion;
    }

    /// <summary>Scales the whole quaternion with the given factor.</summary>
    friend constexpr Quaternion operator*(const Quaternion& quaternion, T factor)
    {
        return Base(quaternion) * factor;
    }

    /// <summary>Scales the whole quaternion with the given factor.</summary>
    friend constexpr Quaternion operator*(T factor, const Quaternion& quaternion)
    {
        return Base(quaternion) * factor;
    }

    /// <summary>Scales the whole quaternion with the given factor.</summary>
    friend constexpr Quaternion& operator*=(Quaternion& quaternion, T factor)
    {
        return quaternion = quaternion * factor;
    }

    /// <summary>Scales the whole quaternion with the given factor.</summary>
    friend constexpr Quaternion operator/(const Quaternion& quaternion, T factor)
    {
        return Base(quaternion) / factor;
    }

    /// <summary>Scales the whole quaternion with the given factor.</summary>
    friend constexpr Quaternion operator/(T factor, const Quaternion& quaternion)
    {
        return factor / Base(quaternion);
    }

    /// <summary>Scales the whole quaternion with the given factor.</summary>
    friend constexpr Quaternion& operator/=(Quaternion& quaternion, T factor)
    {
        return quaternion = quaternion / factor;
    }

    /// <summary>Returns true, if both quaternions are identical.</summary>
    friend constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base::operator==(lhs, rhs);
    }

    /// <summary>Returns true, if the quaternions differ.</summary>
    friend constexpr bool operator!=(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base::operator!=(lhs, rhs);
    }

    struct SlerpResult {
        T source_factor;
        T target_factor;
        bool requires_normalization;
    };

    /// <summary>Helper for slerp, which returns source and target factor and whether the result needs to be normalized.</summary>
    constexpr SlerpResult slerpHelper(Quaternion target, T factor)
    {
        // Following article was used as a base:
        // https://en.wikipedia.org/wiki/Slerp#Source_code

        // If the dot product is negative, slerp won't take the shorter path.
        // Note that target and -target are equivalent when the negation is applied to all four components.
        // Fix by negating the resulting target factor.
        T dot_result = dot(target);
        T target_sign = T(1);
        if (dot_result < T()) {
            target_sign = T(-1);
            dot_result = -dot_result;
        }

        constexpr T epsilon = T(1) - T(1e-5);
        bool requires_normalization = dot_result > epsilon;
        if (requires_normalization) {
            // If the inputs are too close for comfort, use classic lerp and normalize.
            return { T(1) - factor, target_sign * factor, requires_normalization };
        }

        T theta_0 = std::acos(dot_result);
        T theta = theta_0 * factor;

        T sin_theta_0 = std::sin(theta_0);
        T sin_theta = std::sin(theta);

        T target_factor = sin_theta / sin_theta_0;
        T source_factor = std::cos(theta) - dot_result * target_factor;

        return { source_factor, target_sign * target_factor, requires_normalization };
    }

    /// <summary>Performs a spherical interpolation, which has constant velocity, compared to a regular linear interpolation.</summary>
    /// <remarks>Both quaternions should be normalized for correct results.</remarks>
    constexpr Quaternion slerp(Quaternion target, T factor)
    {
        auto slerp_result = slerpHelper(target, factor);
        Quaternion result = slerp_result.source_factor * *this + slerp_result.target_factor * target;
        if (slerp_result.requires_normalization)
            return result.normalize();
        return result;
    }
};

/// <summary>A dual-quaternion, which can represent both rotation (real) and translation (dual).</summary>
template <typename T>
struct DualQuaternion {
    /// <summary>The real-part (rotation) of the dual-quaternion.</summary>
    Quaternion<T> real;
    /// <summary>The dual-part (translation) of the dual-quaternion.</summary>
    Quaternion<T> dual;

    /// <summary>Initializes the dual-quaternion with the identity rotation and zero translation.</summary>
    constexpr DualQuaternion()
        : real(Quaternion<T>::identity())
        , dual(Quaternion<T>::zero())
    {
    }

    /// <summary>Initializes the dual-quaternion with the given rotation and translation quaternions.</summary>
    /// <remarks>The translation is optional and defaults to zero.</remarks>
    constexpr explicit DualQuaternion(const Quaternion<T>& real, const Quaternion<T>& dual = Quaternion<T>::zero())
        : real(real)
        , dual(dual)
    {
    }

    /// <summary>Initializes the dual-quaternion with the given translation quaternion (and identity rotation).</summary>
    /// <remarks>The vector does NOT describe the actual translation, but is instead copied unmodified into the dual part.</remarks>
    constexpr explicit DualQuaternion(const Vector<T, 3>& dual)
        : real(Quaternion<T>::identity())
        , dual(T(), dual)
    {
    }

    /// <summary>Returns a dual-quaternion from the given rotation, specified as rotation-axis and angle in radians.</summary>
    static constexpr DualQuaternion fromAxisRad(const Vector<T, 3>& normal, T radians)
    {
        return Quaternion<T>::fromAxisRad(normal, radians);
    }

    /// <summary>Returns a dual-quaternion from the given rotation, specified as rotation-axis and angle in degrees.</summary>
    static constexpr DualQuaternion fromAxis(const Vector<T, 3>& normal, T degrees)
    {
        return DualQuaternion(Quaternion<T>::fromAxis(normal, degrees));
    }

    /// <summary>Returns a dual-quaternion with all euler angles in radians applied in the given order.</summary>
    template <std::size_t AngleCount>
    static constexpr DualQuaternion fromEulerRad(const Vector<T, AngleCount>& radians, const std::array<Axis3, AngleCount>& order)
    {
        return DualQuaternion(Quaternion<T>::fromEulerRad<AngleCount>(radians, order));
    }

    /// <summary>Returns a dual-quaternion with all euler angles in degrees applied in the given order.</summary>
    template <std::size_t AngleCount>
    static constexpr DualQuaternion fromEuler(const Vector<T, AngleCount>& degrees, const std::array<Axis3, AngleCount>& order)
    {
        return DualQuaternion(Quaternion<T>::fromEuler<AngleCount>(degrees, order));
    }

    /// <summary>Returns a dual-quaternion with all euler angles in radians applied in YXZ-order.</summary>
    static constexpr DualQuaternion fromEulerRadYXZ(const Vector<T, 3>& radians)
    {
        return DualQuaternion(Quaternion<T>::fromEulerRad(radians));
    }

    /// <summary>Returns a dual-quaternion with all euler angles in degrees applied in YXZ-order.</summary>
    static constexpr DualQuaternion fromEulerYXZ(const Vector<T, 3>& degrees)
    {
        return DualQuaternion(Quaternion<T>::fromEuler(degrees));
    }

    /// <summary>Returns a dual-quaternion with all euler angles in radians applied in YX-order.</summary>
    static constexpr DualQuaternion fromEulerRadYX(const Vector<T, 2>& radians)
    {
        return DualQuaternion(Quaternion<T>::fromEulerRad(radians));
    }

    /// <summary>Returns a dual-quaternion with all euler angles in degrees applied in YX-order.</summary>
    static constexpr DualQuaternion fromEulerYX(const Vector<T, 2>& degrees)
    {
        return DualQuaternion(Quaternion<T>::fromEuler(degrees));
    }

    /// <summary>Returns a dual-quaternion from the given translation vector.</summary>
    static constexpr DualQuaternion fromTranslation(const Vector<T, 3>& offset)
    {
        return DualQuaternion(Quaternion<T>::identity(), Vector<T, 4>(offset / T(2), 0));
    }

    /// <summary>Returns the quaternion conjugate by calculating the conjugate for both real and dual part.</summary>
    constexpr DualQuaternion quatConjugate() const
    {
        return DualQuaternion(real.conjugate(), dual.conjugate());
    }

    /// <summary>Returns the dual conjugate by negating the dual part.</summary>
    constexpr DualQuaternion dualConjugate() const
    {
        return DualQuaternion(real, -dual);
    }

    /// <summary>Returns the full conjugate of the dual-quaternion, which is a combination of both quat and dual conjugates.</summary>
    constexpr DualQuaternion conjugate() const
    {
        return DualQuaternion(real.conjugate(), -dual.conjugate());
    }

    /// <summary>Returns the rotation quaternion, which is simply an alias for the real-part.</summary>
    constexpr Quaternion<T> rotation() const
    {
        return real;
    }

    /// <summary>Returns the translation of the dual-quaternion.</summary>
    constexpr Vector<T, 3> translation() const
    {
        return T(2) * (dual * real.conjugate()).vector();
    }

    /// <summary>Returns the normalized dual-quaternion by normalizing the real-part and applying the same factor to the dual-part.</summary>
    constexpr DualQuaternion normalize() const
    {
        return *this / real.magnitude();
    }

    /// <summary>Returns the dot product between the real-parts of the dual-quaternions.</summary>
    constexpr Quaternion<T> dot(const DualQuaternion& other) const
    {
        return real.dot(other.real);
    }

    /// <summary>Returns the inverse of the dual-quaternion, assuming it is normalized, for which it simply calculates the conjugate for both parts.</summary>
    constexpr DualQuaternion inverseFast() const
    {
        return quatConjugate();
    }

    /// <summary>Returns the inverse of the dual-quaternion, even if the dual-quaternion is not normalized.</summary>
    constexpr DualQuaternion inverseSafe() const
    {
        Quaternion<T> realInverse = real.inverseSafe();
        return DualQuaternion(realInverse, -realInverse * dual * realInverse);
    }

    /// <summary>Rotates the dual-quaternion around the given rotation-axis and angle in radians.</summary>
    constexpr DualQuaternion rotateRad(const Vector<T, 3>& normal, T radians) const
    {
        return *this * fromAxisRad(normal, radians);
    }

    /// <summary>Rotates the dual-quaternion around the given rotation-axis and angle in degrees.</summary>
    constexpr DualQuaternion rotate(const Vector<T, 3>& normal, T degrees) const
    {
        return *this * fromAxis(normal, degrees);
    }

    /// <summary>Translates the dual-quaternion by the given offset.</summary>
    constexpr DualQuaternion translate(const Vector<T, 3>& offset) const
    {
        return *this * fromTranslation(offset);
    }

    /// <summary>Converts the dual-quaternion into a 2x4 matrix with real and dual parts.</summary>
    constexpr Matrix<T, 2, 4> toMatrix2x4() const
    {
        return Matrix<T, 2, 4>({ real.asVector(), dual.asVector() });
    }

    /// <summary>Converts the dual-quaternion into a full 4x4 transformation-matrix and returns it.</summary>
    constexpr Matrix<T, 4> toMatrix() const
    {
        Matrix<T, 4> result;
        result.setSubMatrix<0, 0, 3, 3>(real.toMatrix());
        result[3] = Vector<T, 4>(translation(), T(1));
        return result;
    }

    /// <summary>Returns the dual-quaternion itself.</summary>
    constexpr DualQuaternion operator+() const
    {
        return *this;
    }

    /// <summary>Returns the dual-quaternion with both real and dual parts negated.</summary>
    constexpr DualQuaternion operator-() const
    {
        return DualQuaternion(-real, -dual);
    }

    /// <summary>Scales the whole dual-quaternion with the given factor.</summary>
    friend constexpr DualQuaternion& operator*=(DualQuaternion& dualquaternion, T factor)
    {
        dualquaternion.real *= factor;
        dualquaternion.dual *= factor;
        return dualquaternion;
    }

    /// <summary>Scales the whole dual-quaternion with the given factor.</summary>
    friend constexpr DualQuaternion operator*(DualQuaternion dualquaternion, T factor)
    {
        return dualquaternion *= factor;
    }

    /// <summary>Scales the whole dual-quaternion with the given factor.</summary>
    friend constexpr DualQuaternion operator*(T factor, DualQuaternion dualquaternion)
    {
        return dualquaternion *= factor;
    }

    /// <summary>Scales the whole dual-quaternion with the given factor.</summary>
    friend constexpr DualQuaternion& operator/=(DualQuaternion& dualquaternion, T factor)
    {
        dualquaternion.real /= factor;
        dualquaternion.dual /= factor;
        return dualquaternion;
    }

    /// <summary>Scales the whole dual-quaternion with the given factor.</summary>
    friend constexpr DualQuaternion operator/(DualQuaternion dualquaternion, T factor)
    {
        return dualquaternion /= factor;
    }

    /// <summary>Adds both dual-quaternions component-wise.</summary>
    friend constexpr DualQuaternion& operator+=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        lhs.real += rhs.real;
        lhs.dual += rhs.dual;
        return lhs;
    }

    /// <summary>Adds both dual-quaternions component-wise.</summary>
    friend constexpr DualQuaternion operator+(DualQuaternion lhs, const DualQuaternion& rhs)
    {
        return lhs += rhs;
    }

    /// <summary>Subtracts the dual-quaternions component-wise.</summary>
    friend constexpr DualQuaternion& operator-=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        lhs.real -= rhs.real;
        lhs.dual -= rhs.dual;
        return lhs;
    }

    /// <summary>Subtracts the dual-quaternions component-wise.</summary>
    friend constexpr DualQuaternion operator-(DualQuaternion lhs, const DualQuaternion& rhs)
    {
        return lhs -= rhs;
    }

    /// <summary>Combines the transformation of both quaternions.</summary>
    friend constexpr DualQuaternion operator*(const DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return DualQuaternion(rhs.real * lhs.real, rhs.dual * lhs.real + rhs.real * lhs.dual);
    }

    /// <summary>Combines the transformation of rhs onto lhs.</summary>
    friend constexpr DualQuaternion& operator*=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return lhs = lhs * rhs;
    }

    /// <summary>Combines the transformation of the inverse of rhs onto lhs.</summary>
    friend constexpr DualQuaternion& operator/=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return lhs *= rhs.inverseSafe();
    }

    /// <summary>Combines the transformation of lhs with the inverse of rhs.</summary>
    friend constexpr DualQuaternion operator/(DualQuaternion lhs, const DualQuaternion& rhs)
    {
        return lhs /= rhs;
    }

    /// <summary>Applies the quaternion transformation to the given vector.</summary>
    friend constexpr Vector<T, 3> operator*(const DualQuaternion& dualquaternion, const Vector<T, 3>& vector)
    {
        return (dualquaternion.conjugate() * DualQuaternion(vector) * dualquaternion).dual.vector();
    }

    /// <summary>Performs a spherical interpolation, which has constant velocity, compared to a regular linear interpolation.</summary>
    /// <remarks>Both quaternions should be normalized for correct results.</remarks>
    constexpr DualQuaternion slerp(DualQuaternion target, T factor)
    {
        auto slerp_result = real.slerpHelper(target.real, factor);
        DualQuaternion result = slerp_result.source_factor * *this + slerp_result.target_factor * target;
        if (slerp_result.requires_normalization)
            return result.normalize();
        return result;
    }
};

using quat = Quaternion<float>;
using dquat = DualQuaternion<float>;

}
