// TODO: #include <concepts>
#include <limits>
#include <type_traits>

#include "dang-utils/bitset.h"
#include "dang-utils/typelist.h"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

namespace dutils = dang::utils;

using WordSizes = dutils::TypeList<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>;
using ConstWordSizes = WordSizes::transform<std::add_const>;

using AllBitSetMetas = WordSizes::instantiate<dutils::BitSetMeta>;

using AllRegularBitSets = WordSizes::instantiate<dutils::BitSet>;
using AllInfiniteBitSets = WordSizes::instantiate<dutils::InfiniteBitSet>;

using AllBitSets = AllRegularBitSets::join<AllInfiniteBitSets>;

TEST_CASE("BitSetMeta provides an alias type for the given word size.", "[bitset]")
{
    STATIC_CHECK(std::is_same_v<dutils::BitSetMeta<std::uint8_t>::Word, std::uint8_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetMeta<std::uint16_t>::Word, std::uint16_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetMeta<std::uint32_t>::Word, std::uint32_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetMeta<std::uint64_t>::Word, std::uint64_t>);
}

TEMPLATE_LIST_TEST_CASE("BitSetMeta provides various type aliases for word offsets.", "[bitset]", AllBitSetMetas)
{
    STATIC_CHECK(std::is_same_v<typename TestType::size_type, std::size_t>);
    STATIC_CHECK(TestType::npos == std::numeric_limits<typename TestType::size_type>::max());
    STATIC_CHECK(std::is_same_v<typename TestType::difference_type, std::ptrdiff_t>);

    STATIC_CHECK(std::is_same_v<typename TestType::word_size_type, std::size_t>);
    STATIC_CHECK(TestType::word_npos == std::numeric_limits<typename TestType::word_size_type>::max());

    STATIC_CHECK(std::is_same_v<typename TestType::offset_size_type, std::size_t>);
    STATIC_CHECK(TestType::offset_npos == std::numeric_limits<typename TestType::offset_size_type>::max());
}

TEMPLATE_LIST_TEST_CASE("BitSetMeta provides bit set operation utilities.", "[bitset]", AllBitSetMetas)
{
    using Word = typename TestType::Word;

    constexpr auto words_bits = [](auto words, auto bits) { return TestType::word_bits * words + bits; };

    SECTION("Type aliases and constants.")
    {
        STATIC_CHECK(std::is_same_v<typename TestType::value_type, bool>);

        STATIC_CHECK(TestType::word_bits == sizeof(Word) * dutils::char_bit);
        STATIC_CHECK(TestType::empty_word == std::numeric_limits<Word>::min());
        STATIC_CHECK(TestType::filled_word == std::numeric_limits<Word>::max());
    }
    SECTION("Getting the word for a given bit index.")
    {
        STATIC_CHECK(TestType::wordIndex(words_bits(0, +0)) == 0);
        STATIC_CHECK(TestType::wordIndex(words_bits(0, +1)) == 0);
        STATIC_CHECK(TestType::wordIndex(words_bits(1, -1)) == 0);
        STATIC_CHECK(TestType::wordIndex(words_bits(1, +0)) == 1);
        STATIC_CHECK(TestType::wordIndex(words_bits(1, +1)) == 1);
        STATIC_CHECK(TestType::wordIndex(words_bits(2, -1)) == 1);
        STATIC_CHECK(TestType::wordIndex(words_bits(2, +0)) == 2);
    }
    SECTION("Getting the offset inside a word for a given bit index.")
    {
        STATIC_CHECK(TestType::wordOffset(words_bits(0, +0)) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::wordOffset(words_bits(0, +1)) == TestType::word_bits - 2);
        STATIC_CHECK(TestType::wordOffset(words_bits(1, -1)) == 0);
        STATIC_CHECK(TestType::wordOffset(words_bits(1, +0)) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::wordOffset(words_bits(1, +1)) == TestType::word_bits - 2);
        STATIC_CHECK(TestType::wordOffset(words_bits(2, -1)) == 0);
        STATIC_CHECK(TestType::wordOffset(words_bits(2, +0)) == TestType::word_bits - 1);
    }
    SECTION("Getting the minimum number of required words for a given bit count.")
    {
        STATIC_CHECK(TestType::wordCount(words_bits(0, +0)) == 0);
        STATIC_CHECK(TestType::wordCount(words_bits(0, +1)) == 1);
        STATIC_CHECK(TestType::wordCount(words_bits(1, -1)) == 1);
        STATIC_CHECK(TestType::wordCount(words_bits(1, +0)) == 1);
        STATIC_CHECK(TestType::wordCount(words_bits(1, +1)) == 2);
        STATIC_CHECK(TestType::wordCount(words_bits(2, -1)) == 2);
        STATIC_CHECK(TestType::wordCount(words_bits(2, +0)) == 2);
    }
    SECTION("Applying a bit mask on a word.")
    {
        STATIC_CHECK(TestType::applyMask(0b1100, 0b1010, dutils::BitOperation::Set) == 0b1110);
        STATIC_CHECK(TestType::applyMask(0b1100, 0b1010, dutils::BitOperation::Clear) == 0b0100);
        STATIC_CHECK(TestType::applyMask(0b1100, 0b1010, dutils::BitOperation::Mask) == 0b1000);
        STATIC_CHECK(TestType::applyMask(0b1100, 0b1010, dutils::BitOperation::Flip) == 0b0110);
    }
    SECTION("Testing a bit mask on a word.")
    {
        STATIC_CHECK(TestType::testMask(0b00, 0b11, dutils::BitTest::None));
        STATIC_CHECK_FALSE(TestType::testMask(0b01, 0b11, dutils::BitTest::None));
        STATIC_CHECK_FALSE(TestType::testMask(0b11, 0b11, dutils::BitTest::None));

        STATIC_CHECK_FALSE(TestType::testMask(0b00, 0b11, dutils::BitTest::Any));
        STATIC_CHECK(TestType::testMask(0b01, 0b11, dutils::BitTest::Any));
        STATIC_CHECK(TestType::testMask(0b11, 0b11, dutils::BitTest::Any));

        STATIC_CHECK_FALSE(TestType::testMask(0b00, 0b11, dutils::BitTest::All));
        STATIC_CHECK_FALSE(TestType::testMask(0b01, 0b11, dutils::BitTest::All));
        STATIC_CHECK(TestType::testMask(0b11, 0b11, dutils::BitTest::All));
    }
    SECTION("Turning a boolean into a filled mask.")
    {
        STATIC_CHECK(TestType::fillMask(false) == TestType::empty_word);
        STATIC_CHECK(TestType::fillMask(true) == TestType::filled_word);
    }
    SECTION("Generating a single-bit bit mask for a given bit offset.")
    {
        STATIC_CHECK(TestType::bitMask(0) == 0b1);
        STATIC_CHECK(TestType::bitMask(1) == 0b10);
        STATIC_CHECK(TestType::bitMask(TestType::word_bits - 2) == Word{1} << (TestType::word_bits - 2));
        STATIC_CHECK(TestType::bitMask(TestType::word_bits - 1) == Word{1} << (TestType::word_bits - 1));
    }
    SECTION("Generating a mask up to the given bit.")
    {
        STATIC_CHECK(TestType::wordMask(0) == TestType::filled_word);
        STATIC_CHECK(TestType::wordMask(1) == static_cast<Word>(TestType::filled_word << 1));
        STATIC_CHECK(TestType::wordMask(TestType::word_bits - 2) ==
                     static_cast<Word>(TestType::filled_word << (TestType::word_bits - 2)));
        STATIC_CHECK(TestType::wordMask(TestType::word_bits - 1) ==
                     static_cast<Word>(TestType::filled_word << (TestType::word_bits - 1)));
    }
    SECTION("Generating a negative mask for padding up to the given bit.")
    {
        STATIC_CHECK(TestType::padMask(0) == TestType::empty_word);
        STATIC_CHECK(TestType::padMask(1) == 1);
        STATIC_CHECK(TestType::padMask(TestType::word_bits - 2) == static_cast<Word>(TestType::filled_word >> 2));
        STATIC_CHECK(TestType::padMask(TestType::word_bits - 1) == static_cast<Word>(TestType::filled_word >> 1));
    }
    SECTION("Finding the first set/cleared bit in a word.")
    {
        STATIC_CHECK(TestType::firstBit(TestType::filled_word, true) == 0);
        STATIC_CHECK(TestType::firstBit(1, true) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::firstBit(Word{1} << (TestType::word_bits - 1), true) == 0);
        STATIC_CHECK(TestType::firstBit(TestType::empty_word, true) == TestType::word_bits);

        STATIC_CHECK(TestType::firstBit(TestType::empty_word, false) == 0);
        STATIC_CHECK(TestType::firstBit(~Word{1}, false) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::firstBit(~(Word{1} << (TestType::word_bits - 1)), false) == 0);
        STATIC_CHECK(TestType::firstBit(TestType::filled_word, false) == TestType::word_bits);
    }
    SECTION("Finding the last set/cleared bit in a word.")
    {
        STATIC_CHECK(TestType::lastBit(TestType::filled_word, true) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::lastBit(1, true) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::lastBit(Word{1} << (TestType::word_bits - 1), true) == 0);
        STATIC_CHECK(TestType::lastBit(TestType::empty_word, true) == TestType::offset_npos);

        STATIC_CHECK(TestType::lastBit(TestType::empty_word, false) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::lastBit(~Word{1}, false) == TestType::word_bits - 1);
        STATIC_CHECK(TestType::lastBit(~(Word{1} << (TestType::word_bits - 1)), false) == 0);
        STATIC_CHECK(TestType::lastBit(TestType::filled_word, false) == TestType::offset_npos);
    }
    SECTION("Counting set bits in a word.")
    {
        STATIC_CHECK(TestType::bitCount(TestType::filled_word) == TestType::word_bits);
        STATIC_CHECK(TestType::bitCount(0b1) == 1);
        STATIC_CHECK(TestType::bitCount(0b11) == 2);
        STATIC_CHECK(TestType::bitCount(0b10011) == 3);
        STATIC_CHECK(TestType::bitCount(0b11011) == 4);
        STATIC_CHECK(TestType::bitCount(Word{1} << (TestType::word_bits - 1)) == 1);
        STATIC_CHECK(TestType::bitCount(TestType::empty_word) == 0);
    }
}

TEST_CASE("(Infinite)BitSet word size defaults to std::size_t, but any unsigned integer type can be used.", "[bitset]")
{
    STATIC_CHECK(std::is_same_v<dutils::BitSet<>::Word, std::size_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSet<std::uint8_t>::Word, std::uint8_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSet<std::uint16_t>::Word, std::uint16_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSet<std::uint32_t>::Word, std::uint32_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSet<std::uint64_t>::Word, std::uint64_t>);

    STATIC_CHECK(std::is_same_v<dutils::InfiniteBitSet<>::Word, std::size_t>);
    STATIC_CHECK(std::is_same_v<dutils::InfiniteBitSet<std::uint8_t>::Word, std::uint8_t>);
    STATIC_CHECK(std::is_same_v<dutils::InfiniteBitSet<std::uint16_t>::Word, std::uint16_t>);
    STATIC_CHECK(std::is_same_v<dutils::InfiniteBitSet<std::uint32_t>::Word, std::uint32_t>);
    STATIC_CHECK(std::is_same_v<dutils::InfiniteBitSet<std::uint64_t>::Word, std::uint64_t>);
}

TEMPLATE_LIST_TEST_CASE("(Infinite)BitSets provide their used allocator as a type alias.", "[bitset]", AllBitSets)
{
    STATIC_CHECK(std::is_same_v<typename TestType::Allocator, std::allocator<typename TestType::Word>>);
}

TEMPLATE_LIST_TEST_CASE("(Infinite)BitSets provide their corresponding BitSetMeta type as a type alias.",
                        "[bitset]",
                        AllBitSets)
{
    STATIC_CHECK(std::is_same_v<typename TestType::Meta, dutils::BitSetMeta<typename TestType::Word>>);
}

TEMPLATE_LIST_TEST_CASE("(Infinite)BitSets provide container type aliases.",
                        "[bitset]",
                        AllRegularBitSets) // TODO: AllBitSets
{
    [[maybe_unused]] typename TestType::iterator iterator;
    [[maybe_unused]] typename TestType::const_iterator const_iterator;
    [[maybe_unused]] typename TestType::reverse_iterator reverse_iterator;
    [[maybe_unused]] typename TestType::const_reverse_iterator const_reverse_iterator;
    // TODO: For now, just check if the types exist and can be default constructed (as is required by iterators).
    //       Use this instead once concepts work on the CI:
    // STATIC_CHECK(std::random_access_iterator<typename TestType::iterator>);
    // STATIC_CHECK(std::random_access_iterator<typename TestType::const_iterator>);
    // STATIC_CHECK(std::random_access_iterator<typename TestType::reverse_iterator>);
    // STATIC_CHECK(std::random_access_iterator<typename TestType::const_reverse_iterator>);

    // STATIC_CHECK(std::is_same_v<typename TestType::value_type>);
}
