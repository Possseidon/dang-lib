#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace dang::utils {

class LexerError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

// The Lexer concept:
// - using Char = ...;
// - using Token = ...;
// - Lexer(std::basic_string_view<Char>)
// - std::optional<Token> next()

template <typename TChar = char>
struct LexerToken {
    using Char = TChar;
    using TextView = std::basic_string_view<Char>;

    TextView text;
};

/// @brief Tokenizes a series of characters one by one.
template <typename TChar = char>
class BasicLexer {
public:
    using Char = TChar;
    using Token = LexerToken<Char>;

    using TextView = std::basic_string_view<Char>;

    constexpr explicit BasicLexer(TextView text)
        : text_(text)
    {}

    constexpr TextView textView() const { return text_; }

    constexpr std::optional<Token> next()
    {
        if (text_.empty())
            return std::nullopt;
        auto token = text_.substr(0, 1);
        text_.remove_prefix(1);
        return Token{token};
    }

private:
    TextView text_;
};

/// @brief Tokenizes a series of UTF-8 code units one by one.
class Utf8Lexer {
public:
    using Char = char; // TODO: C++20 char8_t
    using Token = LexerToken<Char>;

    using TextView = std::basic_string_view<Char>;

    constexpr explicit Utf8Lexer(TextView text)
        : text_(text)
        , has_bom_(scanBOM())
    {}

    constexpr bool hasBOM() const { return has_bom_; }

    constexpr TextView textView() const { return text_; }

    constexpr std::optional<Token> next()
    {
        if (text_.empty())
            return std::nullopt;

        std::size_t code_unit_length = [&] {
            auto code_point = text_.front();
            if ((code_point & 0b1000'0000) == 0b0000'0000)
                return 1;
            if ((code_point & 0b1110'0000) == 0b1100'0000)
                return 2;
            if ((code_point & 0b1111'0000) == 0b1110'0000)
                return 3;
            if ((code_point & 0b1111'1000) == 0b1111'0000)
                return 4;
            throw LexerError("Invalid initial UTF-8 code point.");
        }();

        if (text_.size() < code_unit_length)
            throw LexerError("Incomplete UTF-8 code unit.");

        for (std::size_t i = 1; i < code_unit_length; i++) {
            if ((text_[i] & 0b1100'0000) != 0b1000'0000)
                throw LexerError("Invalid UTF-8 code point.");
        }

        auto result = text_.substr(0, code_unit_length);
        text_.remove_prefix(code_unit_length);
        return Token{result};
    }

private:
    constexpr bool scanBOM()
    {
        if (text_[0] != '\xEF')
            return false;
        if (text_.size() < 3 || text_[1] != '\xBB' || text_[2] != '\xBF')
            throw LexerError("Invalid UTF-8 BOM.");
        text_.remove_prefix(3);
        return true;
    }

    TextView text_;
    bool has_bom_;
};

} // namespace dang::utils
