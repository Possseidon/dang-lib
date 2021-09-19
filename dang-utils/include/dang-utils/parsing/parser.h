#pragma once

#include <optional>
#include <stdexcept>
#include <string>

#include "dang-utils/global.h"
#include "dang-utils/parsing/lexer.h"

namespace dang::utils {

template <typename TLexer>
class ParserError : public std::runtime_error {
public:
    using Lexer = TLexer;

    ParserError(Lexer lexer, const std::string& message)
        : runtime_error(message)
        , lexer_(lexer)
    {}

private:
    Lexer lexer_;
};

// The Processor concept:

/// @brief Converts a stream of tokens from a lexer into a concrete type.
template <typename TProcessor>
struct Parser {
    using Processor = TProcessor;
    using Lexer = typename Processor::Lexer;
    using Result = typename Processor::Result;
    using Error = ParserError<Lexer>;

    static std::optional<Result> optional(Lexer& lexer) { return Processor::parse(lexer); }

    static Result require(Lexer& lexer)
    {
        if (auto result = optional(lexer))
            return std::move(*result);
        throw Error(lexer, "Expected " + std::string(Processor::name) + ".");
    }
};

} // namespace dang::utils
