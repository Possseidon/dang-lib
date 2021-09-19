#pragma once

#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <string_view>
#include <variant>
#include <vector>

#include "dang-utils/global.h"
#include "dang-utils/utils.h"

namespace dang::utils {

namespace ebnf {

struct ExpressionData;
using Expression = ExpressionData*;
using Expressions = std::vector<Expression>;

std::ostream& operator<<(std::ostream& stream, const ExpressionData& expression);

/// @brief Matches a fixed string.
struct Terminal {
    std::string_view text;

    friend std::ostream& operator<<(std::ostream& stream, const Terminal& terminal)
    {
        return stream << '"' << terminal.text << '"';
    }
};

/// @brief Matches using a custom matching function for e.g. literals.
struct CustomTerminal {
    using Matcher = std::function<std::size_t(std::string_view)>;

    Matcher matcher;

    friend std::ostream& operator<<(std::ostream& stream, const CustomTerminal&) { return stream << "<?>"; }
};

/// @brief Matches the concatenation of multiple expressions.
struct Concatenation {
    Expressions expressions;

    friend std::ostream& operator<<(std::ostream& stream, const Concatenation& concatenation)
    {
        if (concatenation.expressions.empty())
            return stream;
        stream << *concatenation.expressions.front();
        for (std::size_t i = 1; i < concatenation.expressions.size(); i++)
            stream << ", " << *concatenation.expressions[i];
        return stream;
    }
};

/// @brief Matches any one of the expressions.
struct Alternation {
    Expressions expressions;

    friend std::ostream& operator<<(std::ostream& stream, const Alternation& alternation)
    {
        if (alternation.expressions.empty())
            return stream;
        stream << *alternation.expressions.front();
        for (std::size_t i = 1; i < alternation.expressions.size(); i++)
            stream << " | " << *alternation.expressions[i];
        return stream;
    }
};

/// @brief Optionally matches the expression.
struct Option {
    Expression expression;

    friend std::ostream& operator<<(std::ostream& stream, const Option& option)
    {
        return stream << "[ " << *option.expression << " ]";
    }
};

/// @brief Matches any repetition of the expression.
struct Repetition {
    Expression expression;

    friend std::ostream& operator<<(std::ostream& stream, const Repetition& repetition)
    {
        return stream << "{ " << *repetition.expression << " }";
    }
};

/// @brief A named expression that gets resolved delayed.
struct Rule {
    std::string_view name;
    Expression expression = nullptr;

    friend std::ostream& operator<<(std::ostream& stream, const Rule& rule) { return stream << rule.name; }
};

struct ExpressionData {
    std::variant<Terminal, CustomTerminal, Concatenation, Alternation, Option, Repetition, Rule> variant;
};

std::ostream& operator<<(std::ostream& stream, const ExpressionData& expression)
{
    std::visit([&](const auto& expression) { stream << expression; }, expression.variant);
    return stream;
}

}; // namespace ebnf

class EBNF;

class EBNFBuilder {
public:
    using ExpressionData = ebnf::ExpressionData;
    using Expression = ebnf::Expression;
    using Expressions = ebnf::Expressions;

    using Terminal = ebnf::Terminal;
    using CustomTerminal = ebnf::CustomTerminal;
    using Concatenation = ebnf::Concatenation;
    using Alternation = ebnf::Alternation;
    using Option = ebnf::Option;
    using Repetition = ebnf::Repetition;
    using Rule = ebnf::Rule;

    EBNFBuilder() = default;
    EBNFBuilder(const EBNFBuilder&) = delete;
    EBNFBuilder(EBNFBuilder&&) = default;
    EBNFBuilder& operator=(const EBNFBuilder&) = delete;
    EBNFBuilder& operator=(EBNFBuilder&&) = default;

    Expression terminal(std::string_view text) { return make(Terminal{text}); }
    Expression terminal(CustomTerminal::Matcher matcher) { return make(CustomTerminal{std::move(matcher)}); }
    Expression concat(Expressions expressions) { return make(Concatenation{std::move(expressions)}); }
    Expression alternation(Expressions expressions) { return make(Alternation{std::move(expressions)}); }
    Expression option(Expression expression) { return make(Option{expression}); }
    Expression repeat(Expression expression) { return make(Repetition{expression}); }
    Expression rule(std::string_view name) { return make(Rule{name}); }

    // Can create new entries
    Expression& operator[](std::string_view name) { return rules_[name]; }
    // Throws std::out_of_range
    const Expression& get(std::string_view name) const { return rules_.at(name); }

    friend std::ostream& operator<<(std::ostream& stream, const EBNFBuilder& ebnf_builder)
    {
        for (auto [rule_name, expression] : ebnf_builder.rules_)
            stream << rule_name << " = " << *expression << " ;\n";
        return stream;
    }

    EBNF build() &&;

    // TODO: Parse EBNF using EBNF and AST

private:
    template <typename TExpression>
    Expression make(TExpression expression)
    {
        expressions_.push_back(ExpressionData{std::move(expression)});
        return &expressions_.back();
    }

    void resolveRuleExpressions()
    {
        for (const auto& expression : expressions_) {
            std::visit(Overloaded{[&](Rule& rule) { rule.expression = rules_[rule.name]; }, [](const auto&) {}},
                       expression.variant);
        }
    }

    // ExpressionData pointers must remain valid
    std::deque<ExpressionData> expressions_;
    std::map<std::string_view, Expression> rules_;
};

class EBNF {
public:
    friend class EBNFBuilder;

    using Rule = ebnf::Rule;

    Rule operator[](std::string_view name) const { return Rule{name, builder_.get(name)}; }

private:
    EBNF(EBNFBuilder builder)
        : builder_(std::move(builder))
    {}

    EBNFBuilder builder_;
};

class ParseTree {
public:
    struct Node {
        ebnf::Expression expression;
        std::vector<ebnf::Expression> child_expressions;
        std::list<Node> children;
    };

    ParseTree(std::string_view text, const ebnf::Expression& expression)
    {
        while (!heads_.empty()) {
            for (auto head : heads_)
                std::visit(MatchExpression{head}, head->expression->variant);
        }
    }

private:
    struct MatchExpression {
        Node* current;

        void operator()(const ebnf::Terminal& terminal) const
        {
            // if matches, add child
        }
        void operator()(const ebnf::CustomTerminal& terminal) const
        {
            // if matches, add child
        }
        void operator()(const ebnf::Concatenation& terminal) const
        {
            // if all match, add children
        }
        void operator()(const ebnf::Alternation& terminal) const
        {
            // add head for next option
        }
        void operator()(const ebnf::Option& terminal) const
        {
            // add head for with, then for without
        }
        void operator()(const ebnf::Repetition& terminal) const
        {
            // add head for each repetition
            // a = a [ a ]
        }
        void operator()(const ebnf::Rule& rule) const { std::visit(*this, rule.expression->variant); }
    };

    Node root_;
    std::vector<Node*> heads_ = {&root_};
};

EBNF EBNFBuilder::build() &&
{
    resolveRuleExpressions();
    return EBNF(std::move(*this));
}

} // namespace dang::utils
