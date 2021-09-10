#include <array>
#include <string>
#include <tuple>
#include <vector>

#include "dang-utils/parsing.h"

#include "catch2/catch.hpp"

namespace dutils = dang::utils;

// Good logging and error messages

// Lexer
// - Turns a stream of chars into a stream of tokens

// Parser
// - an optional lexer<TToken>
// - optional support for highlighting
// - optional support for suggestions

TEST_CASE("The basic lexer uses characters as tokens.")
{
    SECTION("It returns all characters one by one.")
    {
        auto lexer = dutils::BasicLexer<>("true");
        CHECK(lexer.next().value() == "t");
        CHECK(lexer.next().value() == "r");
        CHECK(lexer.next().value() == "u");
        CHECK(lexer.next().value() == "e");
        CHECK_FALSE(lexer.next());
        CHECK_FALSE(lexer.next());
    }
    SECTION("It can be used constexpr.")
    {
        constexpr auto tokens = [] {
            auto lexer = dutils::BasicLexer<>("const");
            return std::array{
                lexer.next(), lexer.next(), lexer.next(), lexer.next(), lexer.next(), lexer.next(), lexer.next()};
        }();
        STATIC_REQUIRE(tokens[0] == "c");
        STATIC_REQUIRE(tokens[1] == "o");
        STATIC_REQUIRE(tokens[2] == "n");
        STATIC_REQUIRE(tokens[3] == "s");
        STATIC_REQUIRE(tokens[4] == "t");
        STATIC_REQUIRE_FALSE(tokens[5]);
        STATIC_REQUIRE_FALSE(tokens[6]);
    }
}

TEST_CASE("The UTF-8 lexer uses UTF-8 code points as tokens.")
{
    // TODO: C++20 will need u8 literals.
    SECTION("It can lex ASCII.")
    {
        auto lexer = dutils::Utf8Lexer("true");
        CHECK_FALSE(lexer.hasBOM());
        CHECK(lexer.next().value() == "t");
        CHECK(lexer.next().value() == "r");
        CHECK(lexer.next().value() == "u");
        CHECK(lexer.next().value() == "e");
        CHECK_FALSE(lexer.next());
        CHECK_FALSE(lexer.next());
    }
    SECTION("It can lex Japanese code points.")
    {
        auto lexer = dutils::Utf8Lexer("ごきげんよう");
        CHECK_FALSE(lexer.hasBOM());
        CHECK(lexer.next().value() == "ご");
        CHECK(lexer.next().value() == "き");
        CHECK(lexer.next().value() == "げ");
        CHECK(lexer.next().value() == "ん");
        CHECK(lexer.next().value() == "よ");
        CHECK(lexer.next().value() == "う");
        CHECK_FALSE(lexer.next());
        CHECK_FALSE(lexer.next());
    }
    SECTION("It can lex a combination of ASCII and Japanese.")
    {
        auto lexer = dutils::Utf8Lexer("AあIいUうEえOお");
        CHECK_FALSE(lexer.hasBOM());
        CHECK(lexer.next().value() == "A");
        CHECK(lexer.next().value() == "あ");
        CHECK(lexer.next().value() == "I");
        CHECK(lexer.next().value() == "い");
        CHECK(lexer.next().value() == "U");
        CHECK(lexer.next().value() == "う");
        CHECK(lexer.next().value() == "E");
        CHECK(lexer.next().value() == "え");
        CHECK(lexer.next().value() == "O");
        CHECK(lexer.next().value() == "お");
        CHECK_FALSE(lexer.next());
        CHECK_FALSE(lexer.next());
    }
    SECTION("It skips a potential UTF-8 BOM.")
    {
        SECTION("Only BOM.")
        {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBF");
            CHECK(lexer.hasBOM());
            CHECK_FALSE(lexer.next());
            CHECK_FALSE(lexer.next());
        }
        SECTION("BOM with ASCII.")
        {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBFhi");
            CHECK(lexer.hasBOM());
            CHECK(lexer.next() == "h");
            CHECK(lexer.next() == "i");
            CHECK_FALSE(lexer.next());
            CHECK_FALSE(lexer.next());
        }
        SECTION("BOM with Japanese.")
        {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBFはい");
            CHECK(lexer.hasBOM());
            CHECK(lexer.next() == "は");
            CHECK(lexer.next() == "い");
            CHECK_FALSE(lexer.next());
            CHECK_FALSE(lexer.next());
        }
    }
    SECTION("It can be used constexpr.")
    {
        constexpr const auto& lexer_and_tokens = [] {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBFコンスト");
            return std::tuple{
                lexer, std::array{lexer.next(), lexer.next(), lexer.next(), lexer.next(), lexer.next(), lexer.next()}};
        }();
        constexpr const auto& lexer = std::get<0>(lexer_and_tokens);
        constexpr const auto& tokens = std::get<1>(lexer_and_tokens);
        STATIC_REQUIRE(lexer.hasBOM());
        STATIC_REQUIRE(tokens[0] == "コ");
        STATIC_REQUIRE(tokens[1] == "ン");
        STATIC_REQUIRE(tokens[2] == "ス");
        STATIC_REQUIRE(tokens[3] == "ト");
        STATIC_REQUIRE_FALSE(tokens[4]);
        STATIC_REQUIRE_FALSE(tokens[5]);
    }
}
