#include "preprocess/expression.h"
#include "preprocess/expression_value.h"
#include "posttoken/number.h"
#include "posttoken/literal.h"
#include <algorithm>

namespace cppgm {

PPExpressionEvaluator::PPExpressionEvaluator(IdentifierTable& identifiers, DefinedQuery defined,
                                           void* context, PPExpressionStats* stats, PostStats* literal_stats)
    : identifiers_(identifiers), defined_(defined), context_(context), stats_(stats), literal_stats_(literal_stats) {}

std::size_t PPExpressionEvaluator::storage_bytes() const
{
    return values_.capacity() * sizeof(PPValue) + operators_.capacity() * sizeof(Operator);
}

void PPExpressionEvaluator::value(PPValue result)
{
    if (stats_ && values_.size() == values_.capacity()) ++stats_->storage_growths;
    values_.push_back(result);
    if (stats_) stats_->max_values = std::max(stats_->max_values, values_.size());
    need_operand_ = false;
}

void PPExpressionEvaluator::operation(Operator op)
{
    if (stats_ && operators_.size() == operators_.capacity()) ++stats_->storage_growths;
    operators_.push_back(op);
    if (stats_) stats_->max_operators = std::max(stats_->max_operators, operators_.size());
}

void PPExpressionEvaluator::reduce()
{
    Operator op = operators_.back();
    operators_.pop_back();
    unsigned arity = op.unary ? 1 : op.kind == OP_COLON ? 3 : 2;
    if (values_.size() < arity) { valid_ = false; return; }
    std::size_t base = values_.size() - arity;
    PPValue result;
    if (op.unary) result = pp_unary(op.kind, values_[base]);
    else if (op.kind == OP_COLON)
        result = pp_conditional(values_[base], values_[base + 1], values_[base + 2]);
    else result = pp_binary(op.kind, values_[base], values_[base + 1]);
    values_[base] = result;
    values_.resize(base + 1);
    if (stats_) ++stats_->reductions;
}

void PPExpressionEvaluator::reduce_before(unsigned precedence)
{
    while (valid_ && !operators_.empty() && operators_.back().precedence >= precedence)
        reduce();
}

static unsigned binary_precedence(ETokenType op)
{
    switch (op) {
    case OP_LOR: return 2;
    case OP_LAND: return 3;
    case OP_BOR: return 4;
    case OP_XOR: return 5;
    case OP_AMP: return 6;
    case OP_EQ: case OP_NE: return 7;
    case OP_LT: case OP_GT: case OP_LE: case OP_GE: return 8;
    case OP_LSHIFT: case OP_RSHIFT: return 9;
    case OP_PLUS: case OP_MINUS: return 10;
    case OP_STAR: case OP_DIV: case OP_MOD: return 11;
    default: return 0;
    }
}

void PPExpressionEvaluator::punctuation(ETokenType kind)
{
    if (need_operand_) {
        if (kind == OP_LPAREN) operation(Operator(kind, 0));
        else if (kind == OP_PLUS || kind == OP_MINUS || kind == OP_LNOT || kind == OP_COMPL)
            operation(Operator(kind, 12, true));
        else valid_ = false;
        return;
    }
    if (kind == OP_RPAREN || kind == OP_COLON) {
        reduce_before(1);
        ETokenType barrier = kind == OP_RPAREN ? OP_LPAREN : OP_QMARK;
        if (operators_.empty() || operators_.back().kind != barrier) { valid_ = false; return; }
        operators_.pop_back();
        if (kind == OP_COLON) {
            operation(Operator(OP_COLON, 1));
            need_operand_ = true;
        }
    } else if (kind == OP_QMARK) {
        // Conditional expressions associate to the right. Their middle arm is
        // a full expression; '?' forms a barrier until its matching colon.
        reduce_before(2);
        operation(Operator(OP_QMARK, 0));
        need_operand_ = true;
    } else {
        unsigned precedence = binary_precedence(kind);
        if (!precedence) { valid_ = false; return; }
        reduce_before(precedence);
        operation(Operator(kind, precedence));
        need_operand_ = true;
    }
}

void PPExpressionEvaluator::defined_operand(const PPToken& token)
{
    if (defined_state_ == DefinedState::operand && token.kind == PPTokenKind::punctuation &&
        token.spelling.equals("(")) {
        defined_state_ = DefinedState::parenthesized_operand;
    } else if (defined_state_ == DefinedState::close) {
        valid_ = token.kind == PPTokenKind::punctuation && token.spelling.equals(")");
        defined_state_ = DefinedState::none;
    } else if (token.kind == PPTokenKind::identifier) {
        value(PPValue(defined_(token.identifier, context_)));
        defined_state_ = defined_state_ == DefinedState::parenthesized_operand ?
                         DefinedState::close : DefinedState::none;
    } else valid_ = false;
}

void PPExpressionEvaluator::push(const PPToken& token)
{
    if (token.kind == PPTokenKind::whitespace) return;
    empty_ = false;
    if (stats_) ++stats_->tokens;
    if (!valid_) return; // The caller still lexes the rest of the logical line.
    if (defined_state_ != DefinedState::none) { defined_operand(token); return; }
    if (token.kind == PPTokenKind::punctuation) {
        punctuation(classify_simple(token.spelling));
        return;
    }
    if (!need_operand_) { valid_ = false; return; }
    if (token.kind == PPTokenKind::identifier) {
        if (token.spelling.equals("defined")) defined_state_ = DefinedState::operand;
        else value(PPValue(token.spelling.equals("true")));
        return;
    }
    PostToken converted;
    converted.source = token;
    if (token.kind == PPTokenKind::number) {
        if (literal_stats_) literal_stats_->number_bytes += token.spelling.size;
        decode_number(converted, identifiers_, NumberDomain::integral);
    } else if (token.kind == PPTokenKind::character) {
        if (literal_stats_) literal_stats_->literal_bytes += token.spelling.size;
        decode_character(converted, identifiers_, literal_stats_);
    }
    PPValue result;
    valid_ = promote_pp_literal(converted, result);
    if (valid_) value(result);
}

PPExpressionResult PPExpressionEvaluator::finish()
{
    if (need_operand_ || defined_state_ != DefinedState::none) valid_ = false;
    reduce_before(1);
    valid_ = valid_ && operators_.empty() && values_.size() == 1;
    PPValue result = valid_ ? values_.back() : PPValue();
    PPExpressionResult output = { empty_, valid_ && !result.error, result };
    if (stats_ && !empty_) { ++stats_->lines; stats_->errors += !output.valid; }
    values_.clear(); operators_.clear();
    defined_state_ = DefinedState::none;
    empty_ = valid_ = need_operand_ = true;
    return output;
}

} // namespace cppgm
