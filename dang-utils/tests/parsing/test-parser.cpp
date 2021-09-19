#include <string_view>
#include <variant>

#include "dang-utils/parsing/lexer.h"
#include "dang-utils/parsing/parser.h"

#include "catch2/catch.hpp"

namespace dutils = dang::utils;

using dutils::lex::Any;
using dutils::lex::Char;
using dutils::lex::TakeWhile;

constexpr bool isWhitespace(char c) { return c == ' '; };
constexpr bool isAlpha(char c) { return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'; };

using WhitespaceToken = TakeWhile<isWhitespace>;
using CommaToken = Char<','>;
using StringToken = TakeWhile<isAlpha>;
using InvalidToken = Any<>;

using StringListToken = std::variant<WhitespaceToken, CommaToken, StringToken>;
using StringListLexer = dutils::AutoLexer<StringListToken>;

using StringList = std::vector<std::string>;

struct StringListProcessor {
    using Lexer = StringListLexer;
    using Result = StringList;
    static constexpr std::string_view name = "StringList";

    Lexer& lexer;

    std::optional<Result> operator()(StringToken string_token) const
    {
        auto maybe_string_token = std::get_if<StringToken>(lexer.next());
        if (!maybe_string_token)
            return std::nullopt;
        auto string_token = *maybe_string_token;

        Result result{string_token.text};
        while (true) {
            auto maybe_comma_token = std::get_if<CommaToken>(lexer.next())
        }
        return result;
    }
};

using StringListParser = dutils::Parser<StringListProcessor>;

TEST_CASE("Parser", "[parser]")
{
    auto lexer = dutils::BasicLexer<>("hello, world");

    SECTION("require") { auto x = StringListParser::require(lexer); }
    SECTION("optional") { auto y = StringListParser::optional(lexer); }
}
