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

namespace lex {

/// @brief Reads a single char from the char lexer if it matches.
template <auto v_char, typename TCharLexer = BasicLexer<decltype(v_char)>>
struct Char : LexerToken<typename TCharLexer::Char> {
    using Base = LexerToken<typename TCharLexer::Char>;
    using CharLexer = TCharLexer;

    static_assert(std::is_same_v<decltype(v_char), typename Base::Char>);

    static constexpr std::optional<Char> match(CharLexer& char_lexer)
    {
        auto next_char = char_lexer.next();
        if (!next_char || next_char->text.size() != 1 || next_char->text.front() != v_char)
            return std::nullopt;
        return Char{next_char->text};
    }
};

/// @brief Reads chars as long as the predicate holds true.
/// @remark Allows both TextView and plain chars in the predicate.
template <auto v_predicate, typename TCharLexer = BasicLexer<char>, typename = void>
struct TakeWhile;

template <auto v_predicate, typename TCharLexer>
struct TakeWhile<v_predicate,
                 TCharLexer,
                 std::enable_if_t<std::is_invocable_r_v<bool, decltype(v_predicate), typename TCharLexer::TextView>>>
    : LexerToken<typename TCharLexer::Char> {
    using Base = LexerToken<typename TCharLexer::Char>;
    using CharLexer = TCharLexer;

    static constexpr std::optional<TakeWhile> match(CharLexer& char_lexer)
    {
        auto original_text = char_lexer.textView();
        std::size_t count = 0;
        while (true) {
            auto previous_lexer = char_lexer;
            auto next_char = char_lexer.next();
            if (!next_char || !v_predicate(next_char->text)) {
                char_lexer = previous_lexer;
                break;
            }
            count += next_char->text.size();
        }
        if (count == 0)
            return std::nullopt;
        return TakeWhile{original_text.substr(0, count)};
    }
};

namespace detail {

/// @brief Turns a bool(char) predicate into a bool(TextView) predicate.
template <auto v_predicate, typename TChar>
constexpr bool singleCharValidator(std::basic_string_view<TChar> c)
{
    return c.size() == 1 && v_predicate(c.front());
}

} // namespace detail

template <auto v_predicate, typename TCharLexer>
struct TakeWhile<v_predicate,
                 TCharLexer,
                 std::enable_if_t<std::is_invocable_r_v<bool, decltype(v_predicate), typename TCharLexer::Char>>>
    : TakeWhile<detail::singleCharValidator<v_predicate, typename TCharLexer::Char>, TCharLexer> {
    using Base = TakeWhile<detail::singleCharValidator<v_predicate, typename TCharLexer::Char>, TCharLexer>;
    using CharLexer = TCharLexer;

    static constexpr std::optional<TakeWhile> match(CharLexer& char_lexer)
    {
        auto result = Base::match(char_lexer);
        if (result)
            return TakeWhile{result->text};
        return std::nullopt;
    }
};

/// @brief Reads a single char from the char lexer.
template <typename TCharLexer = BasicLexer<char>>
struct Any : LexerToken<typename TCharLexer::Char> {
    using Base = LexerToken<typename TCharLexer::Char>;
    using CharLexer = TCharLexer;

    static constexpr std::optional<Any> match(CharLexer& char_lexer)
    {
        auto next_char = char_lexer.next();
        if (!next_char)
            return std::nullopt;
        return Any{next_char->text};
    }
};

} // namespace lex

/// @brief A lexer that can processes the given variant of tokens.
template <typename TTokenVariant, typename TCharLexer = BasicLexer<char>>
class AutoLexer;

template <typename... TTokens, typename TCharLexer>
class AutoLexer<std::variant<TTokens...>, TCharLexer> {
public:
    using CharLexer = TCharLexer;
    using Char = typename TCharLexer::Char;
    using Token = std::variant<TTokens...>;

    using TextView = typename CharLexer::TextView;

    constexpr explicit AutoLexer(TextView text)
        : char_lexer_(text)
    {}

    constexpr CharLexer charLexer() const { return char_lexer_; }

    constexpr std::optional<Token> next() { return tryTokens<TTokens...>(); }

private:
    template <typename TFirst, typename... TRest>
    constexpr std::optional<Token> tryTokens()
    {
        auto old_lexer = char_lexer_;
        auto maybe_token = TFirst::match(char_lexer_);
        if (!maybe_token) {
            char_lexer_ = old_lexer;
            if constexpr (sizeof...(TRest) > 0)
                return tryTokens<TRest...>();
            else {
                if (char_lexer_.next())
                    throw LexerError("Invalid lexer token.");
                return std::nullopt;
            }
        }
        return *maybe_token;
    }

    CharLexer char_lexer_;
};

} // namespace dang::utils
