#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>

#include "dang-utils/utils.h"

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

enum class EnumSetIteration { Forward, Bidirectional };

struct All {};
inline constexpr All all;

template <typename T, EnumSetIteration Iteration = EnumSetIteration::Forward>
class EnumSet {
public:
    static constexpr std::size_t Size = EnumCountV<T>;

    using Word = std::conditional_t<
        Size <= 8,
        std::uint8_t,
        std::conditional_t<Size <= 16, std::uint16_t, std::conditional_t<Size <= 32, std::uint32_t, std::uint64_t>>>;

    static constexpr std::size_t WordBits = sizeof(Word) * CHAR_BIT;
    static constexpr std::size_t WordCount = (Size + WordBits - 1) / WordBits;
    static constexpr std::size_t PaddingBits = WordCount * WordBits - Size;

    class iterator_base {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        constexpr iterator_base() = default;

        constexpr iterator_base(std::optional<T> value)
            : value_(value)
        {}

        constexpr reference operator*() const { return *value_; }
        constexpr pointer operator->() const { return &*value_; }

        friend constexpr bool operator==(const iterator_base& lhs, const iterator_base& rhs)
        {
            return lhs.value_ == rhs.value_;
        }

        friend constexpr bool operator!=(const iterator_base& lhs, const iterator_base& rhs) { return !(lhs == rhs); }

        friend constexpr bool operator<(const iterator_base& lhs, const iterator_base& rhs)
        {
            if (!rhs.value_)
                return false;
            if (!lhs.value_)
                return true;
            return static_cast<std::underlying_type_t<T>>(*lhs.value_) <
                   static_cast<std::underlying_type_t<T>>(*rhs.value_);
        }

        friend constexpr bool operator<=(const iterator_base& lhs, const iterator_base& rhs) { return !(rhs < lhs); }

        friend constexpr bool operator>(const iterator_base& lhs, const iterator_base& rhs) { return rhs < lhs; }

        friend constexpr bool operator>=(const iterator_base& lhs, const iterator_base& rhs) { return !(lhs < rhs); }

    protected:
        std::optional<T> value_;
    };

    class forward_iterator : public iterator_base {
    public:
        using iterator_category = std::forward_iterator_tag;

        forward_iterator() = default;

        explicit constexpr forward_iterator(EnumSet set)
            : iterator_base(set.first())
            , set_(set)
        {
            if (this->value_)
                set_.reset(*this->value_);
        }

        constexpr forward_iterator& operator++()
        {
            assert(this->value_);
            this->value_ = set_.first();
            if (this->value_)
                set_.reset(*this->value_);
            return *this;
        }

        constexpr forward_iterator operator++(int)
        {
            auto old = *this;
            ++*this;
            return old;
        }

    private:
        EnumSet set_;
    };

    class bidirectional_iterator : public iterator_base {
    public:
        using iterator_category = std::bidirectional_iterator_tag;

        bidirectional_iterator() = default;

        explicit constexpr bidirectional_iterator(EnumSet set)
            : iterator_base(set.first())
            , set_(set)
        {}

        constexpr bidirectional_iterator(EnumSet set, std::optional<T> value)
            : iterator_base(value)
            , set_(set)
        {}

        constexpr bidirectional_iterator& operator++()
        {
            assert(this->value_);
            do {
                if (this->value_ == static_cast<T>(EnumCountV<T> - 1)) {
                    this->value_ = std::nullopt;
                    break;
                }
                this->value_ = static_cast<T>(static_cast<std::underlying_type_t<T>>(*this->value_) + 1);
            } while (!set_[*this->value_]);
            return *this;
        }

        constexpr bidirectional_iterator operator++(int)
        {
            auto old = *this;
            ++*this;
            return old;
        }

        constexpr bidirectional_iterator& operator--()
        {
            if (!this->value_)
                this->value_ = static_cast<T>(EnumCountV<T> - 1);
            while (!set_[*this->value_])
                this->value_ = static_cast<T>(static_cast<std::underlying_type_t<T>>(*this->value_) - 1);
            return *this;
        }

        constexpr bidirectional_iterator operator--(int)
        {
            auto old = *this;
            --*this;
            return old;
        }

    private:
        EnumSet set_;
    };

    using Iterator =
        std::conditional_t<Iteration == EnumSetIteration::Forward, forward_iterator, bidirectional_iterator>;

    EnumSet() = default;

    template <EnumSetIteration OtherIteration>
    constexpr EnumSet(const EnumSet<T, OtherIteration>& other)
        : words_(other.words())
    {}

    constexpr EnumSet(T value) { set(value); }

    constexpr EnumSet(std::initializer_list<T> values)
    {
        for (auto value : values)
            set(value);
    }

    constexpr EnumSet(All) { set(); }

    static constexpr EnumSet allValues()
    {
        EnumSet result;
        result.set();
        return result;
    }

    template <typename TIntegral>
    static constexpr EnumSet fromBits(TIntegral bits)
    {
        static_assert(WordCount == 1);
        static_assert(std::is_integral_v<TIntegral>);
        EnumSet result;
        result.words_[0] = static_cast<Word>(bits);
        assert(result.trimmed());
        // TIntegral might've been bigger, some bits might've gotten chopped off
        assert(static_cast<TIntegral>(result.words_[0]) == bits);
        return result;
    }

    static constexpr EnumSet fromBits(void* first, std::size_t bytes)
    {
        // TODO: C++20 use bit_cast?
        assert(bytes <= sizeof(EnumSet::words_));
        EnumSet result;
        std::memcpy(result.words_.data(), first, bytes);
        assert(result.trimmed());
        return result;
    }

    constexpr EnumSet<T, EnumSetIteration::Bidirectional> bidirectional() { return *this; }

    // --- bitset operations

    constexpr bool operator[](T value) const { return test(value); }

    constexpr bool test(T value) const
    {
        auto shifted = static_cast<Word>(words_[wordIndex(value)] >> wordOffset(value));
        return static_cast<Word>(shifted & Word{1}) == Word{1};
    }

    constexpr bool all()
    {
        for (auto word : words_)
            if (word != static_cast<Word>(~Word{}))
                return false;
        return true;
    }

    constexpr bool any()
    {
        for (auto word : words_)
            if (word != Word{})
                return true;
        return false;
    }

    constexpr bool none()
    {
        for (auto word : words_)
            if (word != Word{})
                return false;
        return true;
    }

    constexpr auto count() const
    {
        std::size_t count = 0;
        for (auto word : words_)
            count += popcount(word);
        return count;
    }

    constexpr EnumSet& set(bool on = true)
    {
        if (on) {
            for (auto& word : words_)
                word = static_cast<Word>(~Word{});
            trim();
        }
        else {
            for (auto& word : words_)
                word = Word{};
        }
        return *this;
    }

    constexpr EnumSet& set(T value, bool on = true)
    {
        if (on)
            words_[wordIndex(value)] |= static_cast<Word>(Word{1} << wordOffset(value));
        else
            words_[wordIndex(value)] &= static_cast<Word>(~static_cast<Word>(Word{1} << wordOffset(value)));
        return *this;
    }

    constexpr EnumSet& reset()
    {
        set(false);
        return *this;
    }

    constexpr EnumSet& reset(T value)
    {
        set(value, false);
        return *this;
    }

    constexpr EnumSet& flip()
    {
        for (auto& word : words_)
            word ^= static_cast<Word>(~Word{});
        trim();
        return *this;
    }

    constexpr EnumSet& flip(T value)
    {
        for (auto& word : words_)
            word ^= static_cast<Word>(Word{1} << wordOffset(value));
        return *this;
    }

    friend constexpr EnumSet operator|(EnumSet lhs, const EnumSet& rhs) { return lhs |= rhs; }
    friend constexpr EnumSet operator|(EnumSet lhs, T rhs) { return lhs |= rhs; }
    friend constexpr EnumSet operator|(T lhs, EnumSet rhs) { return rhs |= rhs; }

    constexpr EnumSet& operator|=(const EnumSet& other)
    {
        for (std::size_t i = 0; i < WordCount; i++)
            words_[i] |= other.words_[i];
        return *this;
    }

    constexpr EnumSet& operator|=(T value) { return set(value); }

    friend constexpr EnumSet operator&(EnumSet lhs, const EnumSet& rhs) { return lhs &= rhs; }

    constexpr EnumSet& operator&=(const EnumSet& other)
    {
        for (std::size_t i = 0; i < WordCount; i++)
            words_[i] &= other.words_[i];
        return *this;
    }

    friend constexpr EnumSet operator^(EnumSet lhs, const EnumSet& rhs) { return lhs ^= rhs; }
    friend constexpr EnumSet operator^(EnumSet lhs, T rhs) { return lhs ^= rhs; }
    friend constexpr EnumSet operator^(T lhs, EnumSet rhs) { return rhs ^= lhs; }

    constexpr EnumSet& operator^=(const EnumSet& other)
    {
        for (std::size_t i = 0; i < WordCount; i++)
            words_[i] ^= other.words_[i];
        return *this;
    }

    constexpr EnumSet& operator^=(T value) { return flip(value); }

    friend constexpr EnumSet operator-(EnumSet lhs, const EnumSet& rhs) { return lhs -= rhs; }
    friend constexpr EnumSet operator-(EnumSet lhs, T rhs) { return lhs -= rhs; }

    constexpr EnumSet& operator-=(const EnumSet& other)
    {
        for (std::size_t i = 0; i < WordCount; i++)
            words_[i] &= static_cast<Word>(~other.words_[i]);
        return *this;
    }

    constexpr EnumSet& operator-=(T value) { return reset(value); }

    constexpr EnumSet operator~() { return EnumSet(*this).flip(); }

    // --- container operations

    constexpr Iterator begin() const { return Iterator(*this); }

    constexpr Iterator end() const
    {
        if constexpr (Iteration == EnumSetIteration::Forward)
            return Iterator();
        else
            return Iterator(*this, std::nullopt);
    }

    constexpr auto empty() const { return none(); }
    constexpr auto size() const { return count(); }
    constexpr auto max_size() const { return Size; }

    constexpr void clear() { reset(); }

    constexpr auto insert(T value)
    {
        bool exists = test(value);
        if (!exists)
            set(value);
        return std::pair{forward_iterator(*this, value), !exists};
    }

    constexpr auto emplace(T value) { return insert(value); }

    constexpr void erase(T value) { reset(value); }

    constexpr void swap(EnumSet& other)
    {
        using std::swap;
        swap(words_, other.words_);
    }

    friend constexpr void swap(EnumSet& lhs, EnumSet& rhs) { lhs.swap(rhs); }

    constexpr bidirectional_iterator find(T value) const
    {
        return bidirectional_iterator(*this, test(value) ? std::optional{value} : std::nullopt);
    }

    constexpr bool contains(T value) const { return test(value); }

    constexpr T front() const { return *first(); }
    constexpr T back() const { return *last(); }

    friend constexpr bool operator==(const EnumSet& lhs, const EnumSet& rhs)
    {
        for (std::size_t i = 0; i < WordCount; i++)
            if (lhs.words_[i] != rhs.words_[i])
                return false;
        return true;
    }

    friend constexpr bool operator!=(const EnumSet& lhs, const EnumSet& rhs) { return !(lhs.words_ == rhs.words_); }

    friend constexpr bool operator<(const EnumSet& lhs, const EnumSet& rhs)
    {
        for (std::size_t i = 0; i < WordCount; i++)
            if (lhs.words_[i] >= rhs.words_[i])
                return false;
        return true;
    }

    friend constexpr bool operator<=(const EnumSet& lhs, const EnumSet& rhs) { return !(rhs.words_ < lhs.words_); }
    friend constexpr bool operator>(const EnumSet& lhs, const EnumSet& rhs) { return rhs.words_ < lhs.words_; }
    friend constexpr bool operator>=(const EnumSet& lhs, const EnumSet& rhs) { return !(lhs.words_ < rhs.words_); }

    // --- custom operations

    constexpr std::optional<T> first() const
    {
        static_assert(WordCount > 0);
        if constexpr (WordCount == 1) {
            if (words_[0] != Word{})
                return static_cast<T>(countr_zero(words_[0]));
        }
        else {
            std::underlying_type_t<T> result = 0;
            for (auto word : words_) {
                if (word != Word{})
                    return static_cast<T>(result + countr_zero(word));
                result += WordBits;
            }
        }
        return std::nullopt;
    };

    constexpr std::optional<T> last() const
    {
        static_assert(WordCount > 0);
        if constexpr (WordCount == 1) {
            if (words_[0] != Word{})
                return static_cast<T>(WordBits - countl_zero(words_[0]) - 1);
        }
        else {
            std::underlying_type_t<T> result = WordBits * WordCount;
            for (auto iter = words_.rbegin(); iter != words_.rend(); ++iter) {
                if (*iter != Word{})
                    return static_cast<T>(result - countl_zero(*iter) - 1);
                result -= WordBits;
            }
        }
        return std::nullopt;
    };

    constexpr Word& word()
    {
        static_assert(WordCount == 1);
        return words_[0];
    }

    constexpr Word word() const
    {
        static_assert(WordCount == 1);
        return words_[0];
    }

    constexpr auto& words() { return words_; }
    constexpr const auto& words() const { return words_; }

    template <typename TIntegral>
    constexpr TIntegral toBits() const
    {
        static_assert(WordCount == 1);
        static_assert(std::is_integral_v<TIntegral>);
        static_assert(sizeof(TIntegral) >= sizeof(Word));
        return static_cast<TIntegral>(words_[0]);
    }

private:
    // TODO: C++20 replace with std::popcount
    static constexpr int popcount(Word word)
    {
        // Modified version of an algorithm taken from:
        // https://en.wikipedia.org/wiki/Hamming_weight
        constexpr Word m1 = static_cast<Word>(0x5555555555555555);
        constexpr Word m2 = static_cast<Word>(0x3333333333333333);
        constexpr Word m4 = static_cast<Word>(0x0f0f0f0f0f0f0f0f);
        constexpr Word h01 = static_cast<Word>(0x0101010101010101);
        word -= (word >> 1) & m1;
        word = (word & m2) + ((word >> 2) & m2);
        word = (word + (word >> 4)) & m4;
        return static_cast<int>(static_cast<Word>(word * h01) >> (WordBits - 8));
    }

    // TODO: C++20 replace with std::countl_zero
    static constexpr int countl_zero(Word word)
    {
        int count = 0;
        while (word) {
            word >>= 1;
            count++;
        }
        return WordBits - count;
    }

    // TODO: C++20 replace with std::countr_zero
    static constexpr int countr_zero(Word word)
    {
        int count = 0;
        while (word) {
            word <<= 1;
            count++;
        }
        return WordBits - count;
    }

    static constexpr std::size_t wordIndex(T value) { return static_cast<std::size_t>(value) / WordBits; }

    static constexpr std::size_t wordOffset(T value)
    {
        return static_cast<std::size_t>(value) - wordIndex(value) * WordBits;
    }

    constexpr void trim()
    {
        if constexpr (PaddingBits > 0)
            words_.back() &= static_cast<Word>(static_cast<Word>(~Word{}) >> PaddingBits);
    }

    constexpr bool trimmed() const
    {
        if constexpr (PaddingBits > 0)
            return (words_.back() & (static_cast<Word>(static_cast<Word>(~Word{}) << (WordBits - PaddingBits)))) == 0;
        else
            return true;
    }

    std::array<Word, WordCount> words_{};
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

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, auto = dang::utils::EnumCountV<T>>
constexpr auto begin(T)
{
    return dang::utils::EnumValues<T>.begin();
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, auto = dang::utils::EnumCountV<T>>
constexpr auto end(T)
{
    return dang::utils::EnumValues<T>.end();
}

} // namespace std

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, auto = dang::utils::EnumCountV<T>>
inline constexpr dang::utils::EnumSet<T> operator|(T lhs, T rhs)
{
    return dang::utils::EnumSet<T>{lhs, rhs};
}
