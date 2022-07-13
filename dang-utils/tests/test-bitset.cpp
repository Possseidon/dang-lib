#include <array>
// TODO: #include <concepts>
#include <limits>
#include <type_traits>

#include "dang-utils/bitset.h"
#include "dang-utils/typelist.h"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

namespace dutils = dang::utils;

using MutableWordSizes = dutils::TypeList<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>;
using ConstWordSizes = MutableWordSizes::transform<std::add_const>;
using AllWordSizes = MutableWordSizes::join<ConstWordSizes>;

using AllBitSetMetas = MutableWordSizes::instantiate<dutils::BitSetMeta>;

using MutableUnsizedBitSetRefs = MutableWordSizes::instantiate<dutils::BitSetRefUnsized>;
using AllUnsizedBitSetRefs = AllWordSizes::instantiate<dutils::BitSetRefUnsized>;

using MutableSizedBitSetRefs = MutableWordSizes::instantiate<dutils::BitSetRefSized>;
using AllSizedBitSetRefs = AllWordSizes::instantiate<dutils::BitSetRefSized>;

using AllRegularBitSets = MutableWordSizes::instantiate<dutils::BitSet>;
using AllInfiniteBitSets = MutableWordSizes::instantiate<dutils::InfiniteBitSet>;

using AllBitSets = AllRegularBitSets::join<AllInfiniteBitSets>;

// --- is_bit_set_word

TEST_CASE("is_bit_set_word checks if a type can be used as a bit set word.", "[bitset]")
{
    STATIC_CHECK(dutils::is_bit_set_word<std::uint8_t>::value);
    STATIC_CHECK(dutils::is_bit_set_word_v<std::uint8_t>);
    STATIC_CHECK(dutils::is_bit_set_word<std::uint16_t>::value);
    STATIC_CHECK(dutils::is_bit_set_word_v<std::uint16_t>);
    STATIC_CHECK(dutils::is_bit_set_word<std::uint32_t>::value);
    STATIC_CHECK(dutils::is_bit_set_word_v<std::uint32_t>);
    STATIC_CHECK(dutils::is_bit_set_word<std::uint64_t>::value);
    STATIC_CHECK(dutils::is_bit_set_word_v<std::uint64_t>);

    STATIC_CHECK_FALSE(dutils::is_bit_set_word<const std::uint8_t>::value);
    STATIC_CHECK_FALSE(dutils::is_bit_set_word_v<const std::uint8_t>);
    STATIC_CHECK_FALSE(dutils::is_bit_set_word<volatile std::uint8_t>::value);
    STATIC_CHECK_FALSE(dutils::is_bit_set_word_v<volatile std::uint8_t>);

    STATIC_CHECK_FALSE(dutils::is_bit_set_word<int>::value);
    STATIC_CHECK_FALSE(dutils::is_bit_set_word_v<int>);
    STATIC_CHECK_FALSE(dutils::is_bit_set_word<bool>::value);
    STATIC_CHECK_FALSE(dutils::is_bit_set_word_v<bool>);
}

// --- BitSetMeta

TEST_CASE("BitSetMeta provides a type alias for the given word size.", "[bitset]")
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

// --- BitSetRefUnsized

TEST_CASE("BitSetRefUnsized provides a type alias for the given word size.", "[bitset]")
{
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefUnsized<std::uint8_t>::Word, std::uint8_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefUnsized<std::uint16_t>::Word, std::uint16_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefUnsized<std::uint32_t>::Word, std::uint32_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefUnsized<std::uint64_t>::Word, std::uint64_t>);
}

TEMPLATE_LIST_TEST_CASE("BitSetRefUnsized forwards various type aliases from BitSetMeta.",
                        "[bitset]",
                        AllUnsizedBitSetRefs)
{
    using Meta = dutils::BitSetMeta<std::remove_const_t<typename TestType::Word>>;

    STATIC_CHECK(std::is_same_v<typename TestType::Meta, Meta>);
    STATIC_CHECK(std::is_same_v<typename TestType::size_type, typename Meta::size_type>);
    STATIC_CHECK(std::is_same_v<typename TestType::value_type, typename Meta::value_type>);

    STATIC_CHECK(TestType::word_bits == Meta::word_bits);
    STATIC_CHECK(TestType::empty_word == Meta::empty_word);
    STATIC_CHECK(TestType::filled_word == Meta::filled_word);
}

TEMPLATE_LIST_TEST_CASE("BitSetRefUnsized can be constructed.", "[bitset]", AllUnsizedBitSetRefs)
{
    auto word = typename TestType::Word{0};

    SECTION("Using the default constructor.") { [[maybe_unused]] auto ref = TestType(); }
    SECTION("Using a word pointer.") { [[maybe_unused]] auto ref = TestType(&word); }

    SUCCEED();
}

TEMPLATE_LIST_TEST_CASE("BitSetRefUnsized provides mutating bit set operation utilities.",
                        "[bitset]",
                        MutableUnsizedBitSetRefs)
{
    constexpr auto bit = TestType::word_bits * 2 - 1;

    SECTION("Setting a single bit.")
    {
        auto bit_set = std::array{
            TestType::empty_word,
            TestType::empty_word,
            TestType::empty_word,
        };
        auto ref = TestType(bit_set.data());

        SECTION("By setting it.") { ref.applyBit(bit, dutils::BitOperation::Set); }
        SECTION("By flipping it.") { ref.applyBit(bit, dutils::BitOperation::Flip); }

        CHECK(bit_set[0] == TestType::empty_word);
        CHECK(bit_set[1] == 1);
        CHECK(bit_set[2] == TestType::empty_word);
    }
    SECTION("Clearing a single bit.")
    {
        auto bit_set = std::array{
            TestType::filled_word,
            TestType::filled_word,
            TestType::filled_word,
        };
        auto ref = TestType(bit_set.data());

        SECTION("By clearing it.") { ref.applyBit(bit, dutils::BitOperation::Clear); }
        SECTION("By flipping it.") { ref.applyBit(bit, dutils::BitOperation::Flip); }

        CHECK(bit_set[0] == TestType::filled_word);
        CHECK(bit_set[1] == TestType::filled_word - 1);
        CHECK(bit_set[2] == TestType::filled_word);
    }
    SECTION("Masking an entire word.")
    {
        auto bit_set = std::array{
            TestType::filled_word,
            TestType::filled_word,
            TestType::filled_word,
        };
        auto ref = TestType(bit_set.data());

        ref.applyBit(bit, dutils::BitOperation::Mask);

        CHECK(bit_set[0] == TestType::filled_word);
        CHECK(bit_set[1] == 1);
        CHECK(bit_set[2] == TestType::filled_word);
    }
    SECTION("Modifying the front word.")
    {
        auto bit_set = std::array{
            TestType::empty_word,
            TestType::empty_word,
        };
        auto ref = TestType(bit_set.data());

        ref.frontWord() = 69;

        CHECK(bit_set[0] == 69);
        CHECK(bit_set[1] == TestType::empty_word);
    }
}

TEMPLATE_LIST_TEST_CASE("BitSetRefUnsized provides const bit set operation utilities.",
                        "[bitset]",
                        AllUnsizedBitSetRefs)
{
    SECTION("Testing if single bits are set.")
    {
        auto make_bit_set = [] { return std::array<typename TestType::Word, 2>{1}; };

        STATIC_CHECK_FALSE(TestType(make_bit_set().data()).testBit(TestType::word_bits - 2));
        STATIC_CHECK(TestType(make_bit_set().data()).testBit(TestType::word_bits - 1));
        STATIC_CHECK_FALSE(TestType(make_bit_set().data()).testBit(TestType::word_bits));
    }
    SECTION("Reading the first word.")
    {
        auto make_bit_set = [] { return std::array<typename TestType::Word, 2>{69}; };

        STATIC_CHECK(TestType(make_bit_set().data()).frontWord() == 69);
    }
}

// --- BitSetRefSized

TEST_CASE("BitSetRefSized provides a type alias for the given word size.", "[bitset]")
{
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefSized<std::uint8_t>::Word, std::uint8_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefSized<std::uint16_t>::Word, std::uint16_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefSized<std::uint32_t>::Word, std::uint32_t>);
    STATIC_CHECK(std::is_same_v<dutils::BitSetRefSized<std::uint64_t>::Word, std::uint64_t>);
}

TEMPLATE_LIST_TEST_CASE("BitSetRefSized extends the functionality of BitSetRefUnsized by deriving from it.",
                        "[bitset]",
                        AllSizedBitSetRefs)
{
    using BitSetRefUnsized = dutils::BitSetRefUnsized<typename TestType::Word>;
    STATIC_CHECK(std::is_base_of_v<BitSetRefUnsized, TestType>);
    STATIC_CHECK(std::is_convertible_v<TestType*, BitSetRefUnsized*>);
}

TEMPLATE_LIST_TEST_CASE("BitSetRefSized forwards various type aliases from BitSetMeta.", "[bitset]", AllSizedBitSetRefs)
{
    using Meta = dutils::BitSetMeta<std::remove_const_t<typename TestType::Word>>;

    STATIC_CHECK(std::is_same_v<typename TestType::Meta, Meta>);
    STATIC_CHECK(std::is_same_v<typename TestType::size_type, typename Meta::size_type>);
    STATIC_CHECK(std::is_same_v<typename TestType::word_size_type, typename Meta::word_size_type>);
    STATIC_CHECK(std::is_same_v<typename TestType::offset_size_type, typename Meta::offset_size_type>);
    STATIC_CHECK(std::is_same_v<typename TestType::value_type, typename Meta::value_type>);

    STATIC_CHECK(TestType::word_bits == Meta::word_bits);
    STATIC_CHECK(TestType::empty_word == Meta::empty_word);
    STATIC_CHECK(TestType::filled_word == Meta::filled_word);
}

TEMPLATE_LIST_TEST_CASE("BitSetRefSized can be constructed.", "[bitset]", AllSizedBitSetRefs)
{
    auto word = typename TestType::Word{0};

    SECTION("Using the default constructor.") { [[maybe_unused]] auto ref = TestType(); }
    SECTION("Using a word pointer and size.") { [[maybe_unused]] auto ref = TestType(&word, 1); }

    SUCCEED();
}

TEMPLATE_LIST_TEST_CASE("BitSetRefSized provides mutating bit set operation utilities.",
                        "[bitset]",
                        MutableSizedBitSetRefs)
{}

// ----------------------------------------------
// -------- TODO: Below here needs work. --------
// ----------------------------------------------

// --- (Infinite)BitSet

TEST_CASE("(Infinite)BitSet word size defaults to BitSetDefaultWord (std::size_t), but any unsigned integer type can "
          "be used.",
          "[bitset]")
{
    STATIC_CHECK(std::is_same_v<dutils::BitSet<>::Word, dutils::BitSetDefaultWord>);
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
