#pragma once

#include "utils.h"

#include <array>
#include <type_traits>
#include <utility>

namespace dang::utils {

/// <summary>An integral constant converting the given enum value to its underlying type.</summary>
template <auto Value>
struct UnderlyingValue
    : std::integral_constant<std::underlying_type_t<decltype(Value)>,
                             static_cast<std::underlying_type_t<decltype(Value)>>(Value)> {};

/// <summary>Simply uses T::COUNT.</summary>
template <typename T>
struct DefaultEnumCount : UnderlyingValue<T::COUNT> {};

/// <summary>Can be specialized to allow for iteration, usage in EnumArray and Flags.</summary>
template <typename T>
struct EnumCount {};

template <typename T>
inline constexpr auto EnumCountV = EnumCount<T>::value;

/// <summary>Returns a std::array of all enum values, given that the enum has EnumCount specialized.</summary>
template <typename T>
inline constexpr std::array<T, EnumCountV<T>> getEnumValues()
{
    std::array<T, EnumCountV<T>> result{};
    for (std::underlying_type_t<T> i = 0; i < EnumCountV<T>; i++)
        result[i] = static_cast<T>(i);
    return result;
}

/// <summary>An array of all enum values, given that the enum has EnumCount specialized.</summary>
template <typename T>
inline constexpr auto EnumValues = getEnumValues<T>();

/// <summary>A wrapper around std::array, allowing the use of an enum as index, if EnumCount is specialized for it.</summary>
template <typename TEnum, typename TValue>
struct EnumArray : std::array<TValue, EnumCountV<TEnum>> {
    using Base = std::array<TValue, EnumCountV<TEnum>>;

    constexpr TValue& operator[](TEnum pos) noexcept
    {
        return Base::operator[](static_cast<typename Base::size_type>(pos));
    }

    constexpr const TValue& operator[](TEnum pos) const noexcept
    {
        return Base::operator[](static_cast<typename Base::size_type>(pos));
    }

    constexpr TValue& at(TEnum pos) { return Base::at(static_cast<typename Base::size_type>(pos)); }

    constexpr const TValue& at(TEnum pos) const { return Base::at(static_cast<typename Base::size_type>(pos)); }
};

/// <summary>An iterator, allowing iteration over all set bits in a flags-enum with NONE and ALL specified.</summary>
template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, T = T::NONE, T = T::ALL>
class EnumSetIterator : public std::iterator<std::forward_iterator_tag, T> {
public:
    constexpr EnumSetIterator() = default;
    constexpr EnumSetIterator(T set)
        : set_(static_cast<std::underlying_type_t<T>>(set))
        , value_(1)
    {
        while (set_ && !(set_ & 1)) {
            set_ >>= 1;
            value_ <<= 1;
        }
    }

    constexpr EnumSetIterator<T>& operator++()
    {
        do {
            set_ >>= 1;
            value_ <<= 1;
        } while (set_ && !(set_ & 1));
        return *this;
    }

    constexpr EnumSetIterator<T> operator++(int)
    {
        auto result = *this;
        ++(*this);
        return result;
    }

    constexpr bool operator==(EnumSetIterator<T> other) { return set_ == other->set_; }

    constexpr bool operator!=(EnumSetIterator<T> other) { return set_ != other.set_; }

    constexpr T operator*() { return static_cast<T>(value_); }

private:
    std::underlying_type_t<T> set_{};
    std::underlying_type_t<T> value_{};
};

/// <summary>Used in the same fashion as std::index_sequence.</summary>
template <typename T, T... Values>
struct EnumSequence {
    static_assert(std::is_enum_v<T>, "Enum sequence requires enumeration type.");
};

namespace detail {

/// <summary>Helper function to create an enum sequences from a given integer sequence.</summary>
template <typename T, std::underlying_type_t<T>... Indices>
constexpr auto makeEnumSequenceHelper(std::integer_sequence<std::underlying_type_t<T>, Indices...>)
{
    return EnumSequence<T, static_cast<T>(Indices)...>();
}

} // namespace detail

/// <summary>Used in the same fashion as std::make_index_sequence.</summary>
template <typename T>
constexpr auto makeEnumSequence()
{
    return detail::makeEnumSequenceHelper<T>(
        std::make_integer_sequence<std::underlying_type_t<T>, static_cast<std::underlying_type_t<T>>(T::COUNT)>());
}

} // namespace dang::utils

namespace std {

// Allow enum iteration in range based for loops
// for (auto value : MyEnum{})

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, auto = dutils::EnumCountV<T>>
constexpr auto begin(T)
{
    return dutils::EnumValues<T>.begin();
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, auto = dutils::EnumCountV<T>>
constexpr auto end(T)
{
    return dutils::EnumValues<T>.end();
}

} // namespace std
