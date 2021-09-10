#pragma once

#include <optional>
#include <stdexcept>
#include <string_view>

namespace dang::utils {

class LexerError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

// The Lexer concept:
// - using Token = ...;
// - using TextView = ...;
// - Lexer(TextView)
// - std::optional<Token> next()

/// @brief Tokenizes a series of characters one by one.
template <typename TChar = char>
class BasicLexer {
public:
    using TextView = std::basic_string_view<TChar>;
    using Token = TextView;

    constexpr BasicLexer(TextView text_view)
        : text_view_(text_view)
    {}

    constexpr std::optional<Token> next()
    {
        if (text_view_.empty())
            return std::nullopt;
        auto token = text_view_.substr(0, 1);
        text_view_.remove_prefix(1);
        return token;
    }

private:
    TextView text_view_;
};

/// @brief Tokenizes a series of UTF-8 code units one by one.
class Utf8Lexer {
public:
    using TextView = std::basic_string_view<char>; // TODO: C++20 char8_t
    using Token = TextView;

    constexpr Utf8Lexer(TextView text_view)
        : text_view_(text_view)
        , has_bom_(scanBOM())
    {}

    constexpr bool hasBOM() const { return has_bom_; }

    constexpr std::optional<Token> next()
    {
        if (text_view_.empty())
            return std::nullopt;

        std::size_t code_unit_length = [&] {
            auto code_point = text_view_.front();
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

        for (std::size_t i = 1; i < code_unit_length; i++) {
            if ((text_view_[i] & 0b1100'0000) != 0b1000'0000)
                throw LexerError("Invalid UTF-8 code point.");
        }

        auto result = text_view_.substr(0, code_unit_length);
        text_view_.remove_prefix(code_unit_length);
        return result;
    }

private:
    constexpr bool scanBOM()
    {
        if (text_view_[0] != '\xEF')
            return false;
        if (text_view_.size() < 3 || text_view_[1] != '\xBB' || text_view_[2] != '\xBF')
            throw LexerError("Invalid UTF-8 BOM.");
        text_view_.remove_prefix(3);
        return true;
    }

    TextView text_view_;
    bool has_bom_;
};

} // namespace dang::utils
