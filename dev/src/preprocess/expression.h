#pragma once

#include "posttoken/token.h"
#include <vector>

namespace cppgm {

// Target intmax/uintmax value. Bits avoid host signed overflow; arithmetic
// errors are facts, selected only by the enclosing &&, || or ?: expression.
struct PPValue {
    std::uint64_t bits;
    bool is_unsigned, error;
    PPValue(std::uint64_t bits = 0, bool is_unsigned = false, bool error = false)
        : bits(bits), is_unsigned(is_unsigned), error(error) {}
};

struct PPExpressionResult {
    bool empty, valid;
    PPValue value;
};

struct PPExpressionStats {
    std::size_t tokens = 0, lines = 0, errors = 0, reductions = 0;
    std::size_t max_values = 0, max_operators = 0, storage_growths = 0;
};

typedef bool (*DefinedQuery)(IdentifierId identifier, void* context);

// Consumes borrowed PP tokens immediately, retaining only typed values and
// pending operations. finish() ends one logical line and resets syntax state.
// The owner supplies TU identifiers and macro lookup; neither is cached here.
// Each token/operation is visited O(1) times. Geometric scratch is bounded by
// pending nesting/precedence, reused between lines and released on destruction.
class PPExpressionEvaluator {
public:
    PPExpressionEvaluator(IdentifierTable& identifiers, DefinedQuery defined,
                          void* context, PPExpressionStats* stats = 0,
                          PostStats* literal_stats = 0);
    void push(const PPToken& token);
    PPExpressionResult finish();
    std::size_t storage_bytes() const;

private:
    // '?' is an unmatched barrier; ':' is a complete ternary operation.
    struct Operator {
        ETokenType kind;
        unsigned char precedence;
        bool unary;
        Operator(ETokenType kind, unsigned char precedence, bool unary = false)
            : kind(kind), precedence(precedence), unary(unary) {}
    };
    enum class DefinedState { none, operand, parenthesized_operand, close };
    IdentifierTable& identifiers_;
    DefinedQuery defined_;
    void* context_;
    PPExpressionStats* stats_;
    PostStats* literal_stats_;
    std::vector<PPValue> values_;
    std::vector<Operator> operators_;
    DefinedState defined_state_ = DefinedState::none;
    bool empty_ = true, valid_ = true, need_operand_ = true;

    void value(PPValue value);
    void operation(Operator op);
    void reduce();
    void reduce_before(unsigned precedence);
    void punctuation(ETokenType kind);
    void defined_operand(const PPToken& token);
};

} // namespace cppgm
