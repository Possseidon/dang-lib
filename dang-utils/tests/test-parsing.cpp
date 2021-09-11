#include <array>
#include <string>
#include <tuple>
#include <vector>

#include "dang-utils/parsing.h"

#include "catch2/catch.hpp"

namespace dutils = dang::utils;
using Catch::Matchers::Message;

TEST_CASE("The basic lexer uses characters as tokens.", "[lexer]")
{
    SECTION("It returns all characters one by one.")
    {
        auto lexer = dutils::BasicLexer<>("true");
        CHECK(lexer.next().value().text == "t");
        CHECK(lexer.next().value().text == "r");
        CHECK(lexer.next().value().text == "u");
        CHECK(lexer.next().value().text == "e");
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
        STATIC_REQUIRE(tokens[0].value().text == "c");
        STATIC_REQUIRE(tokens[1].value().text == "o");
        STATIC_REQUIRE(tokens[2].value().text == "n");
        STATIC_REQUIRE(tokens[3].value().text == "s");
        STATIC_REQUIRE(tokens[4].value().text == "t");
        STATIC_REQUIRE_FALSE(tokens[5]);
        STATIC_REQUIRE_FALSE(tokens[6]);
    }
}

TEST_CASE("The UTF-8 lexer uses UTF-8 code points as tokens.", "[lexer]")
{
    // TODO: C++20 will need u8 literals.
    SECTION("It can lex ASCII.")
    {
        auto lexer = dutils::Utf8Lexer("true");
        CHECK_FALSE(lexer.hasBOM());
        CHECK(lexer.next().value().text == "t");
        CHECK(lexer.next().value().text == "r");
        CHECK(lexer.next().value().text == "u");
        CHECK(lexer.next().value().text == "e");
        CHECK_FALSE(lexer.next());
        CHECK_FALSE(lexer.next());
    }
    SECTION("It can lex Japanese code points.")
    {
        auto lexer = dutils::Utf8Lexer("ごきげんよう");
        CHECK_FALSE(lexer.hasBOM());
        CHECK(lexer.next().value().text == "ご");
        CHECK(lexer.next().value().text == "き");
        CHECK(lexer.next().value().text == "げ");
        CHECK(lexer.next().value().text == "ん");
        CHECK(lexer.next().value().text == "よ");
        CHECK(lexer.next().value().text == "う");
        CHECK_FALSE(lexer.next());
        CHECK_FALSE(lexer.next());
    }
    SECTION("It can lex a combination of ASCII and Japanese.")
    {
        auto lexer = dutils::Utf8Lexer("AあIいUうEえOお");
        CHECK_FALSE(lexer.hasBOM());
        CHECK(lexer.next().value().text == "A");
        CHECK(lexer.next().value().text == "あ");
        CHECK(lexer.next().value().text == "I");
        CHECK(lexer.next().value().text == "い");
        CHECK(lexer.next().value().text == "U");
        CHECK(lexer.next().value().text == "う");
        CHECK(lexer.next().value().text == "E");
        CHECK(lexer.next().value().text == "え");
        CHECK(lexer.next().value().text == "O");
        CHECK(lexer.next().value().text == "お");
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
        SECTION("BOM with one character.")
        {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBFX");
            CHECK(lexer.hasBOM());
            CHECK(lexer.next().value().text == "X");
            CHECK_FALSE(lexer.next());
            CHECK_FALSE(lexer.next());
        }
        SECTION("BOM with ASCII.")
        {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBFhi");
            CHECK(lexer.hasBOM());
            CHECK(lexer.next().value().text == "h");
            CHECK(lexer.next().value().text == "i");
            CHECK_FALSE(lexer.next());
            CHECK_FALSE(lexer.next());
        }
        SECTION("BOM with Japanese.")
        {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBFはい");
            CHECK(lexer.hasBOM());
            CHECK(lexer.next().value().text == "は");
            CHECK(lexer.next().value().text == "い");
            CHECK_FALSE(lexer.next());
            CHECK_FALSE(lexer.next());
        }
    }
    SECTION("It can be used constexpr.")
    {
        constexpr auto lexer_and_tokens = [] {
            auto lexer = dutils::Utf8Lexer("\xEF\xBB\xBFコンスト");
            return std::tuple{
                lexer, std::array{lexer.next(), lexer.next(), lexer.next(), lexer.next(), lexer.next(), lexer.next()}};
        }();
        constexpr auto lexer = std::get<0>(lexer_and_tokens);
        constexpr auto tokens = std::get<1>(lexer_and_tokens);
        STATIC_REQUIRE(lexer.hasBOM());
        STATIC_REQUIRE(tokens[0].value().text == "コ");
        STATIC_REQUIRE(tokens[1].value().text == "ン");
        STATIC_REQUIRE(tokens[2].value().text == "ス");
        STATIC_REQUIRE(tokens[3].value().text == "ト");
        STATIC_REQUIRE_FALSE(tokens[4]);
        STATIC_REQUIRE_FALSE(tokens[5]);
    }
    SECTION("It throws a LexerError for invalid UTF-8 code points.")
    {
        auto lex = [](auto text) { dutils::Utf8Lexer(text).next(); };

        CHECK_THROWS_MATCHES(lex("\x80"), dutils::LexerError, Message("Invalid initial UTF-8 code point."));
        CHECK_THROWS_MATCHES(lex("\xC0\x01"), dutils::LexerError, Message("Invalid UTF-8 code point."));
        CHECK_THROWS_MATCHES(lex("\xC0"), dutils::LexerError, Message("Incomplete UTF-8 code unit."));
        CHECK_THROWS_MATCHES(lex("\xE0\x80\x01"), dutils::LexerError, Message("Invalid UTF-8 code point."));
        CHECK_THROWS_MATCHES(lex("\xE0\x80"), dutils::LexerError, Message("Incomplete UTF-8 code unit."));
        CHECK_THROWS_MATCHES(lex("\xF0\x80\x80\x01"), dutils::LexerError, Message("Invalid UTF-8 code point."));
        CHECK_THROWS_MATCHES(lex("\xF0\x80\x80"), dutils::LexerError, Message("Incomplete UTF-8 code unit."));
    }
}
