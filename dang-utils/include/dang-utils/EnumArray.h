#pragma once

#include "utils.h"

#include <array>
#include <experimental/xutility>

namespace dang::utils
{
   
template <typename T>
constexpr std::size_t getEnumCount()
{
    return static_cast<std::size_t>(T::COUNT);
}

template <typename T>
constexpr std::size_t EnumCount = getEnumCount<T>();

template <typename T>
constexpr std::array<T, EnumCount<T>> getEnumValues()
{
    std::array<T, EnumCount<T>> result{};
    for (std::size_t i = 0; i < EnumCount<T>; i++)
        result[i] = static_cast<T>(i);
    return result;
}

template <typename T>
constexpr std::array<T, EnumCount<T>> EnumValues = getEnumValues<T>();

template <typename TEnum, typename TValue>
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

}
