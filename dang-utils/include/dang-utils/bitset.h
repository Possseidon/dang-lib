#pragma once

#include <algorithm>
#include <bit>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

#include "dang-utils/global.h"
#include "dang-utils/utils.h"

namespace dang::utils {

// TODO: C++20 Replace `typename TWord` with `BitSetWord TWord`

// template <typename T>
// concept BitSetWord = std::is_unsigned_v<T> && !std::same_as<T, bool>;

template <typename T>
struct is_bit_set_word
    : std::conjunction<std::is_integral<T>,
                       std::negation<std::is_const<T>>,
                       std::negation<std::is_volatile<T>>,
                       std::negation<std::is_same<T, bool>>> {};

template <typename T>
static constexpr bool is_bit_set_word_v = is_bit_set_word<T>::value;

enum class BitOperation { Set, Clear, Mask, Flip };
enum class BitTest { None, Any, All };

using BitSetDefaultWord = std::size_t;

// --- Forward Declarations

template <typename TWord>
class BitSetMeta;

template <typename TWord>
class BitSetIterator;

template <typename TWord>
class BitSetRefUnsized;

template <typename TWord>
class BitSetRefSized;

template <typename TWord>
class BitProxy;

template <typename TWord>
class BitSetIterator;

template <bool v_iterated_bits, bool v_reverse, typename TWord>
class BitIterator;

template <bool v_iterated_bits, typename TWord>
class Bits;

template <typename TWord = BitSetDefaultWord, typename TAllocator = std::allocator<TWord>>
class BitSet;

template <typename TWord = BitSetDefaultWord, typename TAllocator = std::allocator<TWord>>
class InfiniteBitSet;

// --- Implementation

/// @brief Utility class for bit operations for a given word size.
template <typename TWord>
class BitSetMeta {
public:
    using Word = TWord;

    // TODO: Use Concepts
    static_assert(is_bit_set_word_v<Word>);

    using size_type = std::size_t;
    static constexpr size_type npos = std::numeric_limits<size_type>::max();

    using difference_type = std::ptrdiff_t;

    using word_size_type = std::size_t;
    static constexpr word_size_type word_npos = std::numeric_limits<word_size_type>::max();

    using offset_size_type = std::size_t;
    static constexpr offset_size_type offset_npos = std::numeric_limits<offset_size_type>::max();

    using value_type = bool;

    static constexpr offset_size_type word_bits = sizeof(Word) * char_bit;
    static constexpr Word empty_word = std::numeric_limits<Word>::min();
    static constexpr Word filled_word = std::numeric_limits<Word>::max();

    /// @brief The word index for the given bit index.
    static constexpr word_size_type wordIndex(size_type bit) { return bit / word_bits; }

    /// @brief The offset from the least significant bit inside the corresponding word of the given bit index.
    /// @remark The most significant bit of a word is set first to make comparison easier.
    static constexpr offset_size_type wordOffset(size_type bit) { return word_bits - 1 - bit % word_bits; }

    /// @brief How many words are required to store at least the given number of bits.
    static constexpr word_size_type wordCount(size_type bit_count) { return ceilDiv(bit_count, word_bits); }

    /// @brief Applies the given bit operation on the given word and returns it.
    static constexpr Word applyMask(Word word, Word mask, BitOperation bit_operation)
    {
        switch (bit_operation) {
        case BitOperation::Set:
            return static_cast<Word>(word | mask);
        case BitOperation::Clear:
            return static_cast<Word>(word & static_cast<Word>(~mask));
        case BitOperation::Mask:
            return static_cast<Word>(word & mask);
        case BitOperation::Flip:
            return static_cast<Word>(word ^ mask);
        }
        std::terminate();
    }

    /// @brief Tests if certain bits in the mask are set.
    static constexpr bool testMask(Word word, Word mask, BitTest bit_test)
    {
        switch (bit_test) {
        case BitTest::None:
            return applyMask(word, mask, BitOperation::Mask) == empty_word;
        case BitTest::Any:
            return applyMask(word, mask, BitOperation::Mask) != empty_word;
        case BitTest::All:
            return applyMask(word, mask, BitOperation::Mask) == mask;
        }
        std::terminate();
    }

    /// @brief Builds a mask with all ones or all zeros.
    static constexpr Word fillMask(value_type value) { return value ? filled_word : empty_word; }

    /// @brief Builds a mask with only the given bit set.
    static constexpr Word bitMask(offset_size_type bit_offset) { return static_cast<Word>(Word{1} << bit_offset); }

    /// @brief Builds a mask with only up to the given bit set.
    static constexpr Word wordMask(offset_size_type bit_offset) { return static_cast<Word>(filled_word << bit_offset); }

    /// @brief Builds a mask with only up to the given bit cleared.
    static constexpr Word padMask(offset_size_type bit_offset) { return static_cast<Word>(~wordMask(bit_offset)); }

    /// @brief Finds the offset to the first bit with the given value.
    /// @remark Returns word_bits if no such bit exists.
    static constexpr offset_size_type firstBit(Word word, value_type value)
    {
        return static_cast<offset_size_type>(value ? std::countl_zero(word) : std::countl_one(word));
    }

    /// @brief Finds the offset to the last bit with the given value.
    /// @remark Returns offset_npos if no such bit exists.
    static constexpr offset_size_type lastBit(Word word, value_type value)
    {
        return word_bits - static_cast<offset_size_type>(1) -
               static_cast<offset_size_type>(value ? std::countr_zero(word) : std::countr_one(word));
    }

    /// @brief Counts the number of set bits in the word.
    static constexpr size_type bitCount(Word word) { return static_cast<size_type>(std::popcount(word)); }
};

/// @brief Wrapper class for a bit set of unknown size.
template <typename TWord>
class BitSetRefUnsized {
public:
    using Word = TWord;

    using Meta = BitSetMeta<std::remove_const_t<Word>>;
    using size_type = typename Meta::size_type;
    using value_type = typename Meta::value_type;

    static constexpr auto word_bits = Meta::word_bits;
    static constexpr auto empty_word = Meta::empty_word;
    static constexpr auto filled_word = Meta::filled_word;

    /// @brief Initializes the wrapper with the given word pointer.
    constexpr BitSetRefUnsized(Word* words = nullptr)
        : words_(words)
    {}

    /// @brief Applies a bit operation on the given bit.
    constexpr void applyBit(size_type bit, BitOperation operation)
    {
        auto& word = words_[Meta::wordIndex(bit)];
        word = Meta::applyMask(word, Meta::bitMask(Meta::wordOffset(bit)), operation);
    }

    /// @brief Tests if the given bit is set.
    constexpr value_type testBit(size_type bit) const
    {
        return Meta::testMask(words_[Meta::wordIndex(bit)], Meta::bitMask(Meta::wordOffset(bit)), BitTest::All);
    }

    /// @brief A reference to the first word of the bit set.
    constexpr Word& frontWord() { return *words_; }

    /// @brief The first word of the bit set.
    constexpr Word frontWord() const { return *words_; }

protected:
    Word* words_;
};

/// @brief Wrapper class for a bit set of known size.
template <typename TWord>
class BitSetRefSized : public BitSetRefUnsized<TWord> {
public:
    using Word = TWord;

    using Meta = BitSetMeta<std::remove_const_t<Word>>;
    using size_type = typename Meta::size_type;
    using word_size_type = typename Meta::word_size_type;
    using offset_size_type = typename Meta::offset_size_type;
    using value_type = typename Meta::value_type;

    static constexpr auto word_bits = Meta::word_bits;
    static constexpr auto empty_word = Meta::empty_word;
    static constexpr auto filled_word = Meta::filled_word;

    /// @brief Initializes the wrapper as an empty bit set pointing to nullptr.
    constexpr BitSetRefSized() = default;

    /// @brief Initializes the wrapper with the given word pointer and bit count.
    constexpr BitSetRefSized(Word* words, size_type bit_count)
        : BitSetRefUnsized<Word>(words)
        , bit_count_(bit_count)
    {}

    /// @brief The total number of bits in the bit set.
    constexpr size_type size() const { return bit_count_; }

    /// @brief The number of words that are necessary for the current bit count.
    constexpr word_size_type wordCount() const { return Meta::wordCount(bit_count_); }

    /// @brief The bit offset to pad the last word.
    constexpr offset_size_type backWordPadOffset() const { return Meta::wordOffset(bit_count_ - 1); }

    /// @brief A reference to the last word of the bit set.
    constexpr Word& backWord() { return this->words_[wordCount() - 1]; }

    /// @brief The last word of the bit set.
    constexpr Word backWord() const { return this->words_[wordCount() - 1]; }

    /// @brief Trims the given back word.
    constexpr Word trimmedWord(Word back_word, value_type value = false) const
    {
        return Meta::applyMask(
            back_word, Meta::padMask(backWordPadOffset()), value ? BitOperation::Set : BitOperation::Clear);
    }

    /// @brief The back word with all padding bits cleared (or set).
    /// @remarks Must not be called if there is no padding.
    constexpr Word trimmedBackWord(value_type value = false) const { return trimmedWord(backWord(), value); }

    /// @brief Clears (or sets) all paddings bits.
    constexpr void trim(value_type value = false)
    {
        if (backWordPadOffset() != 0)
            backWord() = trimmedBackWord(value);
    }

    /// @brief Checks if the given back word is trimmed.
    constexpr bool isWordTrimmed(Word back_word, value_type value = false) const
    {
        return Meta::testMask(back_word, Meta::wordMask(backWordPadOffset()), value ? BitTest::All : BitTest::None);
    }

    /// @brief Checks if all padding bits are cleared (or set).
    constexpr bool isTrimmed(value_type value = false) const
    {
        return backWordPadOffset() == 0 || isWordTrimmed(backWord(), value);
    }

    /// @brief Find the first bit set to the given value.
    /// @remark Returns size() if no such bit exists.
    constexpr size_type firstBit(value_type value) const { return bit_count_ == 0 ? 0 : firstBitFromOffset(0, value); }

    /// @brief Find the first bit set to the given value starting after the specified bit.
    /// @remark Returns size() if no such bit exists.
    constexpr size_type nextBit(size_type from, value_type value) const
    {
        if (++from == bit_count_)
            return bit_count_;

        auto word_index = Meta::wordIndex(from);
        auto word_offset = Meta::wordOffset(from);
        if (word_offset == word_bits - 1)
            return firstBitFromOffset(word_index, value);

        auto masked_word = Meta::applyMask(
            this->words_[word_index], Meta::wordMask(word_offset + 1), value ? BitOperation::Clear : BitOperation::Set);

        if (!value && word_index == wordCount() - 1)
            masked_word = trimmedWord(masked_word, true);

        if (masked_word == Meta::fillMask(!value))
            return word_index == wordCount() - 1 ? bit_count_ : firstBitFromOffset(word_index + 1, value);

        return word_index * word_bits + Meta::firstBit(masked_word, value);
    }

    /// @brief Find the last bit set to the given value.
    /// @remark Returns npos if no such bit exists.
    constexpr size_type lastBit(value_type value) const
    {
        return bit_count_ == 0 ? Meta::npos : lastBitFromOffset(Meta::wordIndex(bit_count_ - 1), value);
    }

    /// @brief Find the last bit set to the given value starting before the specified bit.
    /// @remark Returns npos if no such bit exists.
    constexpr size_type prevBit(size_type from, value_type value = true) const
    {
        // intentional unsigned overflow
        if (--from == Meta::npos)
            return Meta::npos;

        auto word_index = Meta::wordIndex(from);
        auto word_offset = Meta::wordOffset(from);
        if (word_offset == 0)
            return lastBitFromOffset(word_index, value);

        auto masked_word = Meta::applyMask(
            this->words_[word_index], Meta::padMask(word_offset + 1), value ? BitOperation::Clear : BitOperation::Set);

        if (!value && word_index == wordCount() - 1)
            masked_word = trimmedWord(masked_word, true);

        if (masked_word == empty_word)
            return word_index == 0 ? Meta::npos : lastBitFromOffset(word_index - 1, value);

        return word_index * word_bits + Meta::lastBit(masked_word, value);
    }

private:
    /// @brief Find the last bit set to the given value starting at the start of the given word.
    /// @remark Returns npos if no such bit exists.
    constexpr size_type firstBitFromOffset(word_size_type start_word_index, value_type value) const
    {
        auto skip_mask = Meta::fillMask(!value);
        auto word_count = wordCount();

        word_size_type word_index;
        for (word_index = start_word_index; word_index < word_count - 1; word_index++) {
            if (this->words_[word_index] != skip_mask)
                return word_index * word_bits + Meta::firstBit(this->words_[word_index], value);
        }

        auto last_word = this->words_[word_index];

        if (!value)
            last_word = trimmedWord(last_word, true);

        if (last_word != skip_mask)
            return word_index * word_bits + Meta::firstBit(last_word, value);

        return bit_count_;
    }

    /// @brief Find the last bit set to the given value starting at the end of the given word index.
    /// @remark Returns npos if no such bit exists.
    constexpr size_type lastBitFromOffset(word_size_type start_word_index, value_type value) const
    {
        auto skip_mask = Meta::fillMask(!value);

        auto start_word = this->words_[start_word_index];

        if (!value && start_word_index == wordCount() - 1)
            start_word = trimmedWord(start_word, true);

        if (start_word != skip_mask)
            return start_word_index * word_bits + Meta::lastBit(start_word, value);

        for (word_size_type word_index = start_word_index - 1; word_index != Meta::word_npos; word_index--) {
            if (this->words_[word_index] != skip_mask)
                return word_index * word_bits + Meta::lastBit(this->words_[word_index], value);
        }

        return Meta::npos;
    }

    size_type bit_count_ = 0;
};

/// @brief A proxy to a single bit in a bit set.
template <typename TWord>
class BitProxy {
public:
    using Word = TWord;

    using Meta = BitSetMeta<std::remove_const_t<Word>>;
    using size_type = typename Meta::size_type;
    using value_type = typename Meta::value_type;

    /// @brief Does not initialize the proxy with any values.
    BitProxy() = default;

    /// @brief Initializes the proxy with the given word pointer and bit index.
    constexpr BitProxy(Word* words, size_type bit)
        : words_(words)
        , bit_(bit)
    {}

    /// @brief Allow for assignment between proxies.
    constexpr BitProxy& operator=(const BitProxy& other)
    {
        static_assert(!std::is_const_v<Word>);
        *this = value_type{other};
        return *this;
    }

    /// @brief Allow for implicit conversion from proxy to bool.
    constexpr operator value_type() const { return ref().testBit(bit_); }

    /// @brief Allow for bool assignment.
    constexpr BitProxy& operator=(value_type value)
    {
        static_assert(!std::is_const_v<Word>);
        ref().applyBit(bit_, value ? BitOperation::Set : BitOperation::Clear);
        return *this;
    }

    /// @brief Flips the bit.
    constexpr void flip()
    {
        static_assert(!std::is_const_v<Word>);
        ref().applyBit(bit_, BitOperation::Flip);
    }

    /// @brief Swaps the contents with the given proxy.
    template <typename TOther>
    constexpr void swap(TOther& other)
    {
        auto other_value = value_type{other};
        other = *this;
        *this = other_value;
    }

    /// @brief Swaps the contents of the two proxies.
    template <typename TRhs>
    friend constexpr void swap(BitProxy& lhs, TRhs& rhs)
    {
        lhs.swap(rhs);
    }

private:
    template <typename>
    friend class BitSetIterator;

    constexpr BitSetRefUnsized<Word> ref() { return {words_}; }
    constexpr BitSetRefUnsized<const Word> ref() const { return {words_}; }

    Word* words_;
    size_type bit_;
};

/// @brief A random access bit set iterator that treats the bit set as a collection of bool.
/// @remark BitProxy is used for references, similar to how std::vector<bool> deals with this.
template <typename TWord>
class BitSetIterator {
public:
    using Word = TWord;

    using Meta = BitSetMeta<std::remove_const_t<Word>>;

    using size_type = typename Meta::size_type;

    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename Meta::value_type;
    using difference_type = typename Meta::difference_type;
    using pointer = std::conditional_t<std::is_const_v<Word>, const BitProxy<Word>*, BitProxy<Word>*>;
    using reference = std::conditional_t<std::is_const_v<Word>, value_type, BitProxy<Word>>;

    /// @brief Does not initialize the iterator with any value.
    constexpr BitSetIterator() = default;

    /// @brief Initializes the iterator with the given word pointer and bit index.
    constexpr BitSetIterator(Word* words, size_type bit)
        : proxy_(words, bit)
    {}

    /// @brief Increments the bit index.
    constexpr BitSetIterator& operator++()
    {
        proxy_.bit_++;
        return *this;
    }

    /// @brief Increments the bit index.
    constexpr BitSetIterator operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    /// @brief Decrements the bit index.
    constexpr BitSetIterator& operator--()
    {
        proxy_.bit_--;
        return *this;
    }

    /// @brief Decrements the bit index.
    constexpr BitSetIterator operator--(int)
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    /// @brief Advances the bit index by the given number of bits.
    constexpr BitSetIterator& operator+=(difference_type diff)
    {
        proxy_.bit_ += diff;
        return *this;
    }

    /// @brief Moves the bit index back by the given number of bits.
    constexpr BitSetIterator& operator-=(difference_type diff)
    {
        proxy_.bit_ -= diff;
        return *this;
    }

    /// @brief Advances the bit index by the given number of bits.
    constexpr BitSetIterator operator+(difference_type diff) const { return {proxy_.words_, proxy_.bit_ + diff}; }

    /// @brief Advances the bit index by the given number of bits.
    friend constexpr BitSetIterator operator+(difference_type diff, BitSetIterator iter) { return iter + diff; }

    /// @brief Moves the bit index back by the given number of bits.
    constexpr BitSetIterator operator-(difference_type diff) const { return {proxy_.words_, proxy_.bit_ - diff}; }

    /// @brief Returns the difference between the two bit indices.
    constexpr difference_type operator-(BitSetIterator other) const { return proxy_.bit_ - other.proxy_.bit_; }

    /// @brief Returns a reference to the bit at the given position, relative to the current.
    constexpr reference operator[](difference_type diff) const { return *(*this + diff); }

    /// @brief Whether two iterators point to the same bit.
    constexpr bool operator==(const BitSetIterator& other) const { return proxy_.bit_ == other.proxy_.bit_; }

    /// @brief Compares the position of two iterators.
    constexpr std::strong_ordering operator<=>(const BitSetIterator& other) const
    {
        return proxy_.bit_ <=> other.proxy_.bit_;
    }

    /// @brief A reference to the bit at the current position.
    reference operator*() const { return proxy_; }

    /// @brief A pointer to a bit proxy at the current position.
    pointer operator->() const { return &proxy_; }

private:
    BitProxy<Word> proxy_;
};

/// @brief A bidirectional iterator that treats a bit set as a collection of bit indices that are set/cleared.
template <bool v_iterated_bits, bool v_reverse, typename TWord>
class BitIterator {
public:
    static constexpr auto iterated_bits = v_iterated_bits;
    static constexpr auto reverse = v_reverse;
    using Word = TWord;

    using Meta = BitSetMeta<std::remove_const_t<Word>>;
    using BitSetRef = BitSetRefSized<const Word>;

    using size_type = typename Meta::size_type;

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename Meta::size_type;
    using difference_type = typename Meta::difference_type;
    using pointer = const value_type*;
    using reference = value_type;

    constexpr BitIterator() = default;

    constexpr static BitIterator begin(BitSetRef ref)
    {
        if constexpr (reverse)
            return {ref, ref.lastBit(iterated_bits)};
        else
            return {ref, ref.firstBit(iterated_bits)};
    }

    constexpr static BitIterator end(BitSetRef ref)
    {
        if constexpr (reverse)
            return {ref, Meta::npos};
        else
            return {ref, ref.size()};
    }

    /// @brief Increments the bit index.
    constexpr BitIterator& operator++()
    {
        if constexpr (reverse)
            bit_ = ref_.prevBit(bit_, iterated_bits);
        else
            bit_ = ref_.nextBit(bit_, iterated_bits);
        return *this;
    }

    /// @brief Increments the bit index.
    constexpr BitIterator operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    /// @brief Decrements the bit index.
    constexpr BitIterator& operator--()
    {
        if constexpr (reverse)
            bit_ = ref_.nextBit(bit_, iterated_bits);
        else
            bit_ = ref_.prevBit(bit_, iterated_bits);
        return *this;
    }

    /// @brief Decrements the bit index.
    constexpr BitIterator operator--(int)
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    /// @brief Whether two iterators point to the same bit.
    constexpr bool operator==(const BitIterator& other) const { return bit_ == other.bit_; }

    /// @brief Compares the position of two iterators.
    constexpr std::strong_ordering operator<=>(const BitIterator& other) const { return bit_ <=> other.bit_; }

    /// @brief A reference to the bit at the current position.
    reference operator*() const { return bit_; }

    /// @brief A pointer to a bit proxy at the current position.
    pointer operator->() const { return &bit_; }

private:
    constexpr BitIterator(BitSetRef ref, value_type bit)
        : ref_(ref)
        , bit_(bit)
    {}

    BitSetRef ref_;
    value_type bit_;
};

/// @brief A wrapper around a bit set that treats it as a collection of bit indices that are set/cleared when iterated.
template <bool v_iterated_bits, typename TWord>
class Bits {
public:
    static constexpr auto iterated_bits = v_iterated_bits;
    using Word = TWord;

    using Meta = BitSetMeta<Word>;
    using BitSetRef = BitSetRefSized<const Word>;

    using iterator = BitIterator<iterated_bits, false, const Word>;
    using const_iterator = iterator;
    using reverse_iterator = BitIterator<iterated_bits, true, const Word>;
    using const_reverse_iterator = reverse_iterator;

    using size_type = typename Meta::size_type;
    using value_type = size_type;
    using reference = value_type;
    using const_reference = value_type;
    using pointer = const value_type*;
    using const_pointer = const value_type*;

    constexpr Bits(BitSetRef ref)
        : ref_(ref)
    {}

    constexpr iterator begin() { return iterator::begin(ref_); }
    constexpr const_iterator begin() const { return const_iterator::begin(ref_); }
    constexpr const_iterator cbegin() const { return const_iterator::begin(ref_); }

    constexpr iterator end() { return iterator::end(ref_); }
    constexpr const_iterator end() const { return const_iterator::end(ref_); }
    constexpr const_iterator cend() const { return const_iterator::end(ref_); }

    constexpr reverse_iterator rbegin() { return reverse_iterator::begin(ref_); }
    constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator::begin(ref_); }
    constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator::begin(ref_); }

    constexpr reverse_iterator rend() { return reverse_iterator::end(ref_); }
    constexpr const_reverse_iterator rend() const { return const_reverse_iterator::end(ref_); }
    constexpr const_reverse_iterator crend() const { return const_reverse_iterator::end(ref_); }

private:
    BitSetRef ref_;
};

/// @brief A dynamically sized bit set similar to std::vector<bool> but with additional bit related operations.
template <typename TWord, typename TAllocator>
class BitSet {
public:
    using Word = TWord;
    using Allocator = TAllocator;

    using Meta = BitSetMeta<Word>;

    using iterator = BitSetIterator<Word>;
    using const_iterator = BitSetIterator<const Word>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using value_type = typename Meta::value_type;
    using reference = BitProxy<Word>;
    using const_reference = value_type;
    using pointer = iterator;
    using const_pointer = const_iterator;
    using size_type = typename Meta::size_type;

    static constexpr size_type word_bits = sizeof(Word) * char_bit;

    /// @brief Initializes an empty bit set.
    constexpr BitSet() = default;

    /// @brief Initializes the bit set with the given number of bits.
    constexpr BitSet(size_type bit_count)
        : words_(Meta::wordCount(bit_count))
        , bit_count_(bit_count)
    {}

    /// @brief Initializes a bit set with the given values.
    constexpr BitSet(std::initializer_list<value_type> values)
    {
        reserve(values.size());
        for (auto value : values)
            push_back(value);
    }

    /// @brief Initializes a bit set only setting the given bits.
    constexpr BitSet(std::initializer_list<size_type> bits)
    {
        resize(bits.size());
        for (auto bit : bits)
            ; // TODO: set(bit);
    }

    // --- Element Access

    // Vector

    /// @brief Returns a reference to the bit at the given index.
    constexpr reference at(size_type bit)
    {
        validIndexOrThrow(bit);
        return *this[bit];
    }

    /// @brief Returns the value of the bit at the given index.
    constexpr const_reference at(size_type bit) const
    {
        validIndexOrThrow(bit);
        return *this[bit];
    }

    /// @brief Returns a reference to the bit at the given index.
    constexpr reference operator[](size_type bit) { return {wordData(), bit}; }

    /// @brief Returns the value of the bit at the given index.
    constexpr const_reference operator[](size_type bit) const { return test(bit); }

    /// @brief Returns a reference to the first bit.
    constexpr reference front() { return *this[0]; }

    /// @brief Returns the value of the first bit.
    constexpr const_reference front() const { return *this[0]; }

    /// @brief Returns a reference to the last bit.
    constexpr reference back() { return *this[bit_count_ - 1]; }

    /// @brief Returns the value of the last bit.
    constexpr const_reference back() const { return *this[bit_count_ - 1]; }

    // data() would have to return bool* which isn't possible.
    // Instead provide wordData() to allow for manual bit twiddling.
    // Careful not to break the invariant of padding bits staying zero!

    /// @brief Returns a pointer to the first word.
    /// @remark Padding bits are zero and must remain so.
    constexpr Word* wordData() { return words_.data(); }

    /// @brief Returns a const pointer to the first word.
    /// @remark Padding bits are zero.
    constexpr const Word* wordData() const { return words_.data(); }

    // BitSet

    // operator[]() identical to Vector

    /// @brief If the given bit is set.
    value_type test(size_type bit) const { return at(bit); }

    /// @brief If all bits are set.
    bool all() const
    {
        return bit_count_ == 0 ||
               std::all_of(words_.begin(), words_.end() - 1, [](Word word) { return word == Meta::filled_word; }) &&
                   refSized().trimmedBackWord(true) == Meta::filled_word;
    }

    /// @brief If any bit is set.
    bool any() const
    {
        return !std::ranges::all_of(words_, [](Word word) { return word == Meta::empty_word; });
    }

    /// @brief If no bit is set.
    bool none() const
    {
        return std::ranges::all_of(words_, [](Word word) { return word == Meta::empty_word; });
    }

    /// @brief Returns the number of set bits.
    /// @remark Has linear complexity (on word size), but is quite efficient thanks to popcount.
    size_type count() const
    {
        return std::transform_reduce(words_.begin(), words_.end(), size_type{0}, std::plus(), Meta::bitCount);
    }

    // --- Iterators

    // Acts like a collection of value_type by default.

    constexpr iterator begin() { return {wordData(), 0}; }
    constexpr const_iterator begin() const { return {wordData(), 0}; }
    constexpr const_iterator cbegin() const { return {wordData(), 0}; }

    constexpr iterator end() { return {wordData(), bit_count_}; }
    constexpr const_iterator end() const { return {wordData(), bit_count_}; }
    constexpr const_iterator cend() const { return {wordData(), bit_count_}; }

    constexpr reverse_iterator rbegin() { return {end()}; }
    constexpr const_reverse_iterator rbegin() const { return {end()}; }
    constexpr const_reverse_iterator crbegin() const { return {end()}; }

    constexpr reverse_iterator rend() { return {begin()}; }
    constexpr const_reverse_iterator rend() const { return {begin()}; }
    constexpr const_reverse_iterator crend() const { return {begin()}; }

    // But also provides adaptors to iterate over set/cleared indices.

    /// @brief Returns a wrapper that treats the bit set as a collection of set/cleared indices.
    template <value_type v_iterated_bits = true>
    Bits<v_iterated_bits, Word> bits() const
    {
        return refSized();
    }

    Bits<true, Word> setBits() const { return refSized(); }

    Bits<false, Word> clearedBits() const { return refSized(); }

    // --- Capacity

    // Vector

    constexpr bool empty() const { return bit_count_ == 0; }
    constexpr size_type size() const { return bit_count_; }

    constexpr size_type max_size() const
    {
        auto max_words = words_.max_size();
        constexpr auto max_bits = std::numeric_limits<size_type>::max();
        return max_bits / word_bits > max_words ? max_bits : max_words * word_bits;
    }

    constexpr void reserve(size_type bit_count) { words_.reserve(Meta::wordCount(bit_count)); }
    constexpr size_type capacity() const { return words_.capacity() * word_bits; }
    constexpr void shrink_to_fit() { words_.shrink_to_fit(); }

    // BitSet

    // size() from Vector

    // --- Modifiers

    constexpr void clear() { words_.clear(); }

    constexpr void insert(const_iterator pos, value_type value);
    constexpr void insert(const_iterator pos, size_type count, value_type value);
    template <typename TInputIter>
    constexpr void insert(const_iterator pos, TInputIter first, TInputIter last);
    constexpr void insert(const_iterator pos, std::initializer_list<value_type> values);

    constexpr void emplace(const_iterator pos, value_type value);

    constexpr iterator erase(const_iterator pos);
    constexpr iterator erase(const_iterator first, const_iterator last);

    constexpr void push_back(value_type value) { resize(bit_count_ + 1, value); }
    constexpr reference emplace_back(value_type value);
    constexpr void pop_back();

    constexpr void resize(size_type bit_count, value_type value = false)
    {
        if (value) {
            refSized().trim(true);
            words_.resize(Meta::wordCount(bit_count), Meta::fillMask(value));
            bit_count_ = bit_count;
            refSized().trim();
        }
        else {
            words_.resize(Meta::wordCount(bit_count), Meta::fillMask(value));
            bit_count_ = bit_count;
        }
    }

    constexpr void swap(BitSet& other)
    {
        words_.swap(other.words_);
        std::swap(bit_count_, other.bit_count_);
    }

    // --- Other

    friend constexpr bool operator==(const BitSet& lhs, const BitSet& rhs)
    {
        return lhs.bit_count_ == rhs.bit_count_ && lhs.words_ == rhs.words_;
    }

    friend constexpr std::strong_ordering operator<=>(const BitSet& lhs, const BitSet& rhs)
    {
        auto cmp = std::lexicographical_compare_three_way(
            lhs.words_.begin(), lhs.words_.end(), rhs.words_.begin(), rhs.words_.end());
        return cmp == 0 ? lhs.bit_count_ <=> rhs.bit_count_ : cmp;
    }

    friend constexpr void swap(BitSet& lhs, BitSet& rhs) { return lhs.swap(rhs); }

    constexpr bool isValidIndex(size_type bit) const { return bit < bit_count_; }

    constexpr void validIndexOrThrow(size_type bit) const
    {
        if (!isValidIndex(bit))
            throw std::out_of_range("bit index out of range");
    }

private:
    constexpr BitSetRefUnsized<Word> refUnsized() { return {words_.data()}; }
    constexpr BitSetRefUnsized<const Word> refUnsized() const { return {words_.data()}; }
    constexpr BitSetRefSized<Word> refSized() { return {words_.data(), bit_count_}; }
    constexpr BitSetRefSized<const Word> refSized() const { return {words_.data(), bit_count_}; }

    std::vector<TWord, TAllocator> words_;
    size_type bit_count_ = 0;
};

// TODO: InfiniteBitSet iterators might modify the last set bit and shrink it.
//      - Either only provide const iterators
//      - Or write custom iterators that directly operate on InfiniteBitSet and deal with it

// Automatically resizes itself whenever bits get set or cleared.
template <typename TWord, typename TAllocator>
class InfiniteBitSet {
public:
    using Word = TWord;
    using Allocator = TAllocator;

    using Meta = BitSetMeta<Word>;

private:
    using Underlying = BitSet<Word, Allocator>;

public:
    using value_type = typename Underlying::value_type;
    using size_type = typename Underlying::size_type;

    static constexpr auto word_bits = Underlying::word_bits;

    constexpr InfiniteBitSet() = default;

private:
    Underlying bit_set_;
};

} // namespace dang::utils
