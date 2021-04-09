#pragma once

#include "dang-math/global.h"
#include "dang-math/matrix.h"
#include "dang-math/utils.h"
#include "dang-math/vector.h"

namespace dang::math {

/// @brief A quaternion, which usually represents an arbitrary rotation in three-dimensional space.
/// @remark The rotation is expressed using an axis (vector/xyz) and a rotation distance (scalar/w).
/// @remark The quaternion needs to be normalized, before applying it (using multiplication).
template <typename T>
struct Quaternion : private Vector<T, 4> {
    using Base = Vector<T, 4>;

    using Type = T;

    /// @brief Initializes the quaternion to the zero quaternion, which cannot be normalized or used directly.
    constexpr Quaternion()
        : Base()
    {}
    /// @brief Initializes the quaternion from a four-dimensional vector.
    constexpr Quaternion(const Base& vector)
        : Base(vector)
    {}
    /// @brief Initializes the quaternion from scalar and vector without normalization.
    constexpr Quaternion(T scalar, const Vector<T, 3>& vector)
        : Quaternion(Base(vector, scalar))
    {}
    /// @brief Initializes the quaternion from w-scalar and xyz-vector without normalization.
    constexpr Quaternion(T w, T x, T y, T z)
        : Quaternion(Base(x, y, z, w))
    {}

    /// @brief Returns the zero-quaternion, which cannot be normalized or used directly.
    static constexpr Quaternion zero() { return Quaternion(0, 0, 0, 0); }

    /// @brief Returns the identity-quaternion, which is normalized and, when applied, does not do anything.
    static constexpr Quaternion identity() { return Quaternion(1, 0, 0, 0); }

    /// @brief Returns a quaternion from the given rotation, specified as rotation-axis and angle in radians.
    static constexpr Quaternion fromAxisRad(const Vector<T, 3>& normal, T radians)
    {
        radians /= 2;
        T sin_radians = std::sin(radians);
        return Quaternion(
            std::cos(radians), normal.x() * sin_radians, normal.y() * sin_radians, normal.z() * sin_radians);
    }

    /// @brief Returns a quaternion from the given rotation, specified as rotation-axis and angle in degrees.
    static constexpr Quaternion fromAxis(const Vector<T, 3>& normal, T degrees)
    {
        return fromAxisRad(normal, radians(degrees));
    }

    /// @brief Returns a quaternion with all euler angles in radians applied in the given order.
    template <std::size_t v_angle_count>
    static constexpr Quaternion fromEulerRad(const Vector<T, v_angle_count>& radians,
                                             const std::array<Axis3, v_angle_count>& order)
    {
        Quaternion result = Quaternion::identity();
        for (std::size_t i = 0; i < v_angle_count; i++)
            result *= fromAxisRad(static_cast<Vector<T, 3>>(axis_vector_3[order[i]]), radians[i]);
        return result;
    }

    /// @brief Returns a quaternion with all euler angles in degrees applied in the given order.
    template <std::size_t v_angle_count>
    static constexpr Quaternion fromEuler(const Vector<T, v_angle_count>& degrees,
                                          const std::array<Axis3, v_angle_count>& order)
    {
        return fromEulerRad(degrees.radians(), order);
    }

    /// @brief Returns a quaternion with all euler angles in radians applied in YXZ-order.
    static constexpr Quaternion fromEulerRad(const Vector<T, 3>& radians)
    {
        return fromEulerRad(radians, {Axis3::Y, Axis3::X, Axis3::Z});
    }

    /// @brief Returns a quaternion with all euler angles in degrees applied in YXZ-order.
    static constexpr Quaternion fromEuler(const Vector<T, 3>& degrees) { return fromEulerRad(degrees.radians()); }

    /// @brief Returns a quaternion with all euler angles in radians applied in YX-order.
    static constexpr Quaternion fromEulerRad(const Vector<T, 2>& radians)
    {
        return fromEulerRad(radians, {Axis3::Y, Axis3::X});
    }

    /// @brief Returns a quaternion with all euler angles in degrees applied in YX-order.
    static constexpr Quaternion fromEuler(const Vector<T, 2>& degrees) { return fromEulerRad(degrees.radians()); }

    /// @brief Returns the scalar/w part of the quaternion.
    constexpr T scalar() const { return Base::w(); }
    /// @brief Returns the scalar/w part of the quaternion.
    constexpr T w() const { return Base::w(); }

    /// @brief Returns the vector/xyz part of the quaternion.
    constexpr Vector<T, 3> vector() const { return Base::xyz(); };
    /// @brief Returns the vector x-part of the quaternion.
    constexpr T x() const { return Base::x(); }
    /// @brief Returns the vector y-part of the quaternion.
    constexpr T y() const { return Base::y(); }
    /// @brief Returns the vector z-part of the quaternion.
    constexpr T z() const { return Base::z(); }

    using Base::dot;
    using Base::sqrdot;

    /// @brief Returns the normalized quaternion, which can safely be applied using multiplication.
    constexpr Quaternion normalize() const { return Base::normalize(); }

    /// @brief Returns the magnitude of the quaternion, which is simply the length of the xyzw-vector.
    constexpr T magnitude() const { return Base::length(); }

    /// @brief Returns the conjugate of the quaternion, which simply has the vector-part negated.
    constexpr Quaternion conjugate() const { return Quaternion(scalar(), -vector()); }

    /// @brief Returns the inverse of the quaternion, assuming it is normalized, for which it simply calculates the
    /// conjugate.
    constexpr Quaternion inverseFast() const { return conjugate(); }

    /// @brief Returns the inverse of the quaternion, even if the quaternion is not normalized.
    constexpr Quaternion inverseSafe() const { return conjugate() / sqrdot(); }

    /// @brief Converts the quaternion into a simple xyzw-vector.
    constexpr const Vector<T, 4>& asVector() const { return *this; }

    /// @brief Converts the quaternion into a simple xyzw-vector.
    constexpr Vector<T, 4>& asVector() { return *this; }

    /// @brief Converts the quaternion into a rotation-matrix and returns it.
    constexpr Matrix<T, 3> toMatrix() const
    {
        const T& w = this->w();
        const T& x = this->x();
        const T& y = this->y();
        const T& z = this->z();
        return Matrix<T, 3>(
            {{T(1) - T(2) * y * y - T(2) * z * z, T(2) * x * y + T(2) * z * w, T(2) * x * z - T(2) * y * w},
             {T(2) * x * y - T(2) * z * w, T(1) - T(2) * x * x - T(2) * z * z, T(2) * y * z + T(2) * x * w},
             {T(2) * x * z + T(2) * y * w, T(2) * y * z - T(2) * x * w, T(1) - T(2) * x * x - T(2) * y * y}});
    }

    /// @brief Returns the quaternion itself.
    constexpr Quaternion operator+() const { return *this; }

    /// @brief Returns the quaternion with both scalar and vector negated.
    constexpr Quaternion operator-() const { return Base::operator-(); }

    /// @brief Adds both quaternions component-wise.
    friend constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base(lhs) + Base(rhs);
    }

    /// @brief Adds both quaternions component-wise.
    friend constexpr Quaternion& operator+=(Quaternion& lhs, const Quaternion& rhs) { return lhs = lhs + rhs; }

    /// @brief Adds both quaternions component-wise.
    friend constexpr Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base(lhs) - Base(rhs);
    }

    /// @brief Adds both quaternions component-wise.
    friend constexpr Quaternion& operator-=(Quaternion& lhs, const Quaternion& rhs) { return lhs = lhs - rhs; }

    /// @brief Combines the transformation of both quaternions.
    friend constexpr Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Quaternion((lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z()),
                          (lhs.w() * rhs.x() + lhs.x() * rhs.w() + lhs.y() * rhs.z() - lhs.z() * rhs.y()),
                          (lhs.w() * rhs.y() - lhs.x() * rhs.z() + lhs.y() * rhs.w() + lhs.z() * rhs.x()),
                          (lhs.w() * rhs.z() + lhs.x() * rhs.y() - lhs.y() * rhs.x() + lhs.z() * rhs.w()));
    }

    /// @brief Combines the transformation of rhs onto lhs.
    friend constexpr Quaternion& operator*=(Quaternion& lhs, const Quaternion& rhs) { return lhs = lhs * rhs; }

    /// @brief Combines the transformation of lhs with the inverse of rhs.
    friend constexpr Quaternion operator/(const Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs * rhs.inverseSafe();
    }

    /// @brief Combines the transformation of the inverse of rhs onto lhs.
    friend constexpr Quaternion& operator/=(Quaternion& lhs, const Quaternion& rhs) { return lhs = lhs / rhs; }

    /// @brief Applies the quaternion transformation to the given vector.
    friend constexpr Vector<T, 3> operator*(const Quaternion& quaternion, const Vector<T, 3>& vector)
    {
        Vector<T, 3> u = quaternion.vector();
        Vector<T, 3> uv = u.cross(vector);
        return vector + T(2) * ((quaternion.scalar() * uv) + u.cross(uv));
    }

    /// @brief Applies the transformation of the conjugated quaternion to the given vector.
    friend constexpr Vector<T, 3> operator*(const Vector<T, 3>& vector, const Quaternion& quaternion)
    {
        return quaternion.conjugate() * vector;
    }

    /// @brief Applies the transformation of the conjugated quaternion onto the given vector.
    friend constexpr Vector<T, 3>& operator*=(Vector<T, 3>& vector, const Quaternion& quaternion)
    {
        return vector = vector * quaternion;
    }

    /// @brief Scales the whole quaternion with the given factor.
    friend constexpr Quaternion operator*(const Quaternion& quaternion, T factor) { return Base(quaternion) * factor; }

    /// @brief Scales the whole quaternion with the given factor.
    friend constexpr Quaternion operator*(T factor, const Quaternion& quaternion) { return Base(quaternion) * factor; }

    /// @brief Scales the whole quaternion with the given factor.
    friend constexpr Quaternion& operator*=(Quaternion& quaternion, T factor)
    {
        return quaternion = quaternion * factor;
    }

    /// @brief Scales the whole quaternion with the given factor.
    friend constexpr Quaternion operator/(const Quaternion& quaternion, T factor) { return Base(quaternion) / factor; }

    /// @brief Scales the whole quaternion with the given factor.
    friend constexpr Quaternion operator/(T factor, const Quaternion& quaternion) { return factor / Base(quaternion); }

    /// @brief Scales the whole quaternion with the given factor.
    friend constexpr Quaternion& operator/=(Quaternion& quaternion, T factor)
    {
        return quaternion = quaternion / factor;
    }

    /// @brief Returns true, if both quaternions are identical.
    friend constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base::operator==(lhs, rhs);
    }

    /// @brief Returns true, if the quaternions differ.
    friend constexpr bool operator!=(const Quaternion& lhs, const Quaternion& rhs)
    {
        return Base::operator!=(lhs, rhs);
    }

    struct SlerpResult {
        T source_factor;
        T target_factor;
        bool requires_normalization;
    };

    /// @brief Helper for slerp, which returns source and target factor and whether the result needs to be normalized.
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
            return {T(1) - factor, target_sign * factor, requires_normalization};
        }

        T theta_0 = std::acos(dot_result);
        T theta = theta_0 * factor;

        T sin_theta_0 = std::sin(theta_0);
        T sin_theta = std::sin(theta);

        T target_factor = sin_theta / sin_theta_0;
        T source_factor = std::cos(theta) - dot_result * target_factor;

        return {source_factor, target_sign * target_factor, requires_normalization};
    }

    /// @brief Performs a spherical interpolation, which has constant velocity, compared to a regular linear
    /// interpolation.
    /// @remark Both quaternions should be normalized for correct results.
    constexpr Quaternion slerp(Quaternion target, T factor)
    {
        auto slerp_result = slerpHelper(target, factor);
        Quaternion result = slerp_result.source_factor * *this + slerp_result.target_factor * target;
        if (slerp_result.requires_normalization)
            return result.normalize();
        return result;
    }
};

/// @brief A dual-quaternion, which can represent both rotation (real) and translation (dual).
template <typename T>
struct DualQuaternion {
    using Type = T;

    /// @brief The real-part (rotation) of the dual-quaternion.
    Quaternion<T> real;
    /// @brief The dual-part (translation) of the dual-quaternion.
    Quaternion<T> dual;

    /// @brief Initializes the dual-quaternion with the identity rotation and zero translation.
    constexpr DualQuaternion()
        : real(Quaternion<T>::identity())
        , dual(Quaternion<T>::zero())
    {}

    /// @brief Initializes the dual-quaternion with the given rotation and translation quaternions.
    /// @remark The translation is optional and defaults to zero.
    explicit constexpr DualQuaternion(const Quaternion<T>& real, const Quaternion<T>& dual = Quaternion<T>::zero())
        : real(real)
        , dual(dual)
    {}

    /// @brief Initializes the dual-quaternion with the given translation quaternion (and identity rotation).
    /// @remark The vector does NOT describe the actual translation, but is instead copied unmodified into the dual
    /// part.
    explicit constexpr DualQuaternion(const Vector<T, 3>& dual)
        : real(Quaternion<T>::identity())
        , dual(T(), dual)
    {}

    /// @brief Returns a dual-quaternion from the given rotation, specified as rotation-axis and angle in radians.
    static constexpr DualQuaternion fromAxisRad(const Vector<T, 3>& normal, T radians)
    {
        return Quaternion<T>::fromAxisRad(normal, radians);
    }

    /// @brief Returns a dual-quaternion from the given rotation, specified as rotation-axis and angle in degrees.
    static constexpr DualQuaternion fromAxis(const Vector<T, 3>& normal, T degrees)
    {
        return DualQuaternion(Quaternion<T>::fromAxis(normal, degrees));
    }

    /// @brief Returns a dual-quaternion with all euler angles in radians applied in the given order.
    template <std::size_t v_angle_count>
    static constexpr DualQuaternion fromEulerRad(const Vector<T, v_angle_count>& radians,
                                                 const std::array<Axis3, v_angle_count>& order)
    {
        return DualQuaternion(Quaternion<T>::fromEulerRad<v_angle_count>(radians, order));
    }

    /// @brief Returns a dual-quaternion with all euler angles in degrees applied in the given order.
    template <std::size_t v_angle_count>
    static constexpr DualQuaternion fromEuler(const Vector<T, v_angle_count>& degrees,
                                              const std::array<Axis3, v_angle_count>& order)
    {
        return DualQuaternion(Quaternion<T>::fromEuler<v_angle_count>(degrees, order));
    }

    /// @brief Returns a dual-quaternion with all euler angles in radians applied in YXZ-order.
    static constexpr DualQuaternion fromEulerRadYXZ(const Vector<T, 3>& radians)
    {
        return DualQuaternion(Quaternion<T>::fromEulerRad(radians));
    }

    /// @brief Returns a dual-quaternion with all euler angles in degrees applied in YXZ-order.
    static constexpr DualQuaternion fromEulerYXZ(const Vector<T, 3>& degrees)
    {
        return DualQuaternion(Quaternion<T>::fromEuler(degrees));
    }

    /// @brief Returns a dual-quaternion with all euler angles in radians applied in YX-order.
    static constexpr DualQuaternion fromEulerRadYX(const Vector<T, 2>& radians)
    {
        return DualQuaternion(Quaternion<T>::fromEulerRad(radians));
    }

    /// @brief Returns a dual-quaternion with all euler angles in degrees applied in YX-order.
    static constexpr DualQuaternion fromEulerYX(const Vector<T, 2>& degrees)
    {
        return DualQuaternion(Quaternion<T>::fromEuler(degrees));
    }

    /// @brief Returns a dual-quaternion from the given translation vector.
    static constexpr DualQuaternion fromTranslation(const Vector<T, 3>& offset)
    {
        return DualQuaternion(Quaternion<T>::identity(), Vector<T, 4>(offset / T(2), 0));
    }

    /// @brief Returns the quaternion conjugate by calculating the conjugate for both real and dual part.
    constexpr DualQuaternion quatConjugate() const { return DualQuaternion(real.conjugate(), dual.conjugate()); }

    /// @brief Returns the dual conjugate by negating the dual part.
    constexpr DualQuaternion dualConjugate() const { return DualQuaternion(real, -dual); }

    /// @brief Returns the full conjugate of the dual-quaternion, which is a combination of both quat and dual
    /// conjugates.
    constexpr DualQuaternion conjugate() const { return DualQuaternion(real.conjugate(), -dual.conjugate()); }

    /// @brief Returns the rotation quaternion, which is simply an alias for the real-part.
    constexpr Quaternion<T> rotation() const { return real; }

    /// @brief Returns the translation of the dual-quaternion.
    constexpr Vector<T, 3> translation() const { return T(2) * (dual * real.conjugate()).vector(); }

    /// @brief Returns the normalized dual-quaternion by normalizing the real-part and applying the same factor to the
    /// dual-part.
    constexpr DualQuaternion normalize() const { return *this / real.magnitude(); }

    /// @brief Returns the dot product between the real-parts of the dual-quaternions.
    constexpr Quaternion<T> dot(const DualQuaternion& other) const { return real.dot(other.real); }

    /// @brief Returns the inverse of the dual-quaternion, assuming it is normalized, for which it simply calculates the
    /// conjugate for both parts.
    constexpr DualQuaternion inverseFast() const { return quatConjugate(); }

    /// @brief Returns the inverse of the dual-quaternion, even if the dual-quaternion is not normalized.
    constexpr DualQuaternion inverseSafe() const
    {
        Quaternion<T> realInverse = real.inverseSafe();
        return DualQuaternion(realInverse, -realInverse * dual * realInverse);
    }

    /// @brief Rotates the dual-quaternion around the given rotation-axis and angle in radians.
    constexpr DualQuaternion rotateRad(const Vector<T, 3>& normal, T radians) const
    {
        return *this * fromAxisRad(normal, radians);
    }

    /// @brief Rotates the dual-quaternion around the given rotation-axis and angle in degrees.
    constexpr DualQuaternion rotate(const Vector<T, 3>& normal, T degrees) const
    {
        return *this * fromAxis(normal, degrees);
    }

    /// @brief Translates the dual-quaternion by the given offset.
    constexpr DualQuaternion translate(const Vector<T, 3>& offset) const { return *this * fromTranslation(offset); }

    /// @brief Converts the dual-quaternion into a 2x4 matrix with real and dual parts.
    constexpr Matrix<T, 2, 4> toMatrix2x4() const { return Matrix<T, 2, 4>({real.asVector(), dual.asVector()}); }

    /// @brief Converts the dual-quaternion into a full 4x4 transformation-matrix and returns it.
    constexpr Matrix<T, 4> toMatrix() const
    {
        Matrix<T, 4> result;
        result.setSubMatrix<0, 0, 3, 3>(real.toMatrix());
        result[3] = Vector<T, 4>(translation(), T(1));
        return result;
    }

    /// @brief Returns the dual-quaternion itself.
    constexpr DualQuaternion operator+() const { return *this; }

    /// @brief Returns the dual-quaternion with both real and dual parts negated.
    constexpr DualQuaternion operator-() const { return DualQuaternion(-real, -dual); }

    /// @brief Scales the whole dual-quaternion with the given factor.
    friend constexpr DualQuaternion& operator*=(DualQuaternion& dualquaternion, T factor)
    {
        dualquaternion.real *= factor;
        dualquaternion.dual *= factor;
        return dualquaternion;
    }

    /// @brief Scales the whole dual-quaternion with the given factor.
    friend constexpr DualQuaternion operator*(DualQuaternion dualquaternion, T factor)
    {
        return dualquaternion *= factor;
    }

    /// @brief Scales the whole dual-quaternion with the given factor.
    friend constexpr DualQuaternion operator*(T factor, DualQuaternion dualquaternion)
    {
        return dualquaternion *= factor;
    }

    /// @brief Scales the whole dual-quaternion with the given factor.
    friend constexpr DualQuaternion& operator/=(DualQuaternion& dualquaternion, T factor)
    {
        dualquaternion.real /= factor;
        dualquaternion.dual /= factor;
        return dualquaternion;
    }

    /// @brief Scales the whole dual-quaternion with the given factor.
    friend constexpr DualQuaternion operator/(DualQuaternion dualquaternion, T factor)
    {
        return dualquaternion /= factor;
    }

    /// @brief Adds both dual-quaternions component-wise.
    friend constexpr DualQuaternion& operator+=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        lhs.real += rhs.real;
        lhs.dual += rhs.dual;
        return lhs;
    }

    /// @brief Adds both dual-quaternions component-wise.
    friend constexpr DualQuaternion operator+(DualQuaternion lhs, const DualQuaternion& rhs) { return lhs += rhs; }

    /// @brief Subtracts the dual-quaternions component-wise.
    friend constexpr DualQuaternion& operator-=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        lhs.real -= rhs.real;
        lhs.dual -= rhs.dual;
        return lhs;
    }

    /// @brief Subtracts the dual-quaternions component-wise.
    friend constexpr DualQuaternion operator-(DualQuaternion lhs, const DualQuaternion& rhs) { return lhs -= rhs; }

    /// @brief Combines the transformation of both quaternions.
    friend constexpr DualQuaternion operator*(const DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return DualQuaternion(rhs.real * lhs.real, rhs.dual * lhs.real + rhs.real * lhs.dual);
    }

    /// @brief Combines the transformation of rhs onto lhs.
    friend constexpr DualQuaternion& operator*=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return lhs = lhs * rhs;
    }

    /// @brief Combines the transformation of the inverse of rhs onto lhs.
    friend constexpr DualQuaternion& operator/=(DualQuaternion& lhs, const DualQuaternion& rhs)
    {
        return lhs *= rhs.inverseSafe();
    }

    /// @brief Combines the transformation of lhs with the inverse of rhs.
    friend constexpr DualQuaternion operator/(DualQuaternion lhs, const DualQuaternion& rhs) { return lhs /= rhs; }

    /// @brief Applies the quaternion transformation to the given vector.
    friend constexpr Vector<T, 3> operator*(const DualQuaternion& dualquaternion, const Vector<T, 3>& vector)
    {
        return (dualquaternion.conjugate() * DualQuaternion(vector) * dualquaternion).dual.vector();
    }

    /// @brief Performs a spherical interpolation, which has constant velocity, compared to a regular linear
    /// interpolation.
    /// @remark Both quaternions should be normalized for correct results.
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

} // namespace dang::math
