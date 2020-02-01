#pragma once

#include "utils.h"

#include <array>

namespace dang::utils
{

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::COUNT>
constexpr std::size_t EnumCount = static_cast<std::size_t>(T::COUNT);

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::COUNT>
constexpr std::array<T, EnumCount<T>> getEnumValues()
{
    std::array<T, EnumCount<T>> result{};
    for (std::size_t i = 0; i < EnumCount<T>; i++)
        result[i] = static_cast<T>(i);
    return result;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::COUNT>
constexpr std::array<T, EnumCount<T>> EnumValues = getEnumValues<T>();

template <typename TEnum, typename TValue, typename = std::enable_if_t<std::is_enum_v<TEnum>>, TEnum = TEnum::COUNT>
class EnumArray : public std::array<TValue, EnumCount<TEnum>> {
    static_assert(std::is_enum_v<TEnum>, "EnumArray can only be used with enum types");

    using Base = std::array<TValue, EnumCount<TEnum>>;

public:
    inline constexpr TValue& operator[](TEnum pos) noexcept
    {
        return Base::operator[](static_cast<typename Base::size_type>(pos));
    }

    inline constexpr const TValue& operator[](TEnum pos) const noexcept
    {
        return Base::operator[](static_cast<typename Base::size_type>(pos));
    }

    inline constexpr TValue& at(TEnum pos)
    {
        return Base::at(static_cast<typename Base::size_type>(pos));
    }

    inline constexpr const TValue& at(TEnum pos) const
    {
        return Base::at(static_cast<typename Base::size_type>(pos));
    }
};

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
class EnumSetIterator : public std::iterator<std::forward_iterator_tag, T> {
public:
    inline constexpr EnumSetIterator() = default;
    inline constexpr EnumSetIterator(T set)
        : set_(static_cast<std::underlying_type_t<T>>(set))
        , value_(1)
    {
        while (set_ && !(set_ & 1)) {
            set_ >>= 1;
            value_ <<= 1;
        }
    }

    inline constexpr EnumSetIterator<T>& operator++()
    {
        do {
            set_ >>= 1;
            value_ <<= 1;
        } while (set_ && !(set_ & 1));
        return *this;
    }

    inline constexpr EnumSetIterator<T> operator++(int)
    {
        auto result = *this;
        ++(*this);
        return result;
    }

    inline constexpr bool operator==(EnumSetIterator<T> other)
    {
        return set_ == other->set_;
    }

    inline constexpr bool operator!=(EnumSetIterator<T> other)
    {
        return set_ != other.set_;
    }

    inline constexpr T operator*()
    {
        return static_cast<T>(value_);
    }
    
private:
    std::underlying_type_t<T> set_{};
    std::underlying_type_t<T> value_{};
};

}

namespace std
{

// Allow enum iteration in range based for loops
// for (auto value : MyEnum())

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::COUNT>
inline constexpr auto begin(T)
{
    return dutils::EnumValues<T>.begin();
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::COUNT>
inline constexpr auto end(T)
{
    return dutils::EnumValues<T>.end();
}

// Allow enum-set iteration in range based for loops

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr auto begin(T set)
{
    return dutils::EnumSetIterator<T>(set);
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr auto end(T)
{
    return dutils::EnumSetIterator<T>();
}

}

// Enable enum-set bitwise operations

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T operator|(T lhs, T rhs)
{
    using BaseType = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<BaseType>(lhs) | static_cast<BaseType>(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T& operator|=(T& lhs, T rhs)
{
    return lhs = lhs | rhs;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T operator&(T lhs, T rhs)
{
    using BaseType = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<BaseType>(lhs)& static_cast<BaseType>(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T& operator&=(T& lhs, T rhs)
{
    return lhs = lhs & rhs;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T& operator~(T& value)
{
    using BaseType = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<BaseType>(T::ALL) & ~static_cast<BaseType>(value));
}
                       
template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T operator^(T lhs, T rhs)
{
    using BaseType = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<BaseType>(lhs) ^ static_cast<BaseType>(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T& operator^=(T& lhs, T rhs)
{
    return lhs = lhs ^ rhs;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T operator-(T lhs, T rhs)
{
    using BaseType = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<BaseType>(lhs) & ~static_cast<BaseType>(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
inline constexpr T& operator-=(T& lhs, T rhs)
{
    return lhs = lhs - rhs;
}
