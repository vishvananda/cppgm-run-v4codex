#include "preprocess/expression_value.h"

namespace cppgm {

static const std::uint64_t sign_bit = std::uint64_t(1) << 63;

bool promote_pp_literal(const PostToken& token, PPValue& value)
{
    if (token.kind != PostTokenKind::literal ||
        (token.literal != LiteralKind::integer && token.literal != LiteralKind::character) ||
        token.type > FT_BOOL) return false;
    value = PPValue();
    value.is_unsigned = (token.type >= FT_UNSIGNED_CHAR && token.type <= FT_UNSIGNED_LONG_LONG_INT) ||
                        token.type == FT_CHAR16_T || token.type == FT_CHAR32_T;
    unsigned width = fundamental_width(token.type);
    for (unsigned i = 0; i < width; ++i)
        value.bits |= std::uint64_t(static_cast<unsigned char>(token.scalar[i])) << (i * 8);
    if (!value.is_unsigned && width < 8 && (value.bits & (std::uint64_t(1) << (width * 8 - 1))))
        value.bits |= ~std::uint64_t(0) << (width * 8);
    return true;
}

PPValue pp_unary(ETokenType op, PPValue value)
{
    switch (op) {
    case OP_MINUS: value.bits = 0 - value.bits; break;
    case OP_COMPL: value.bits = ~value.bits; break;
    case OP_LNOT: value.bits = !value.bits; value.is_unsigned = false; break;
    default: break; // unary plus
    }
    return value;
}

PPValue pp_conditional(PPValue condition, PPValue yes, PPValue no)
{
    PPValue result = condition.bits ? yes : no;
    result.is_unsigned = yes.is_unsigned || no.is_unsigned;
    result.error = condition.error || result.error;
    return result;
}

PPValue pp_binary(ETokenType op, PPValue left, PPValue right)
{
    std::uint64_t a = left.bits, b = right.bits;
    bool uns = left.is_unsigned || right.is_unsigned;
    PPValue result(0, uns, left.error || right.error);
    // Checking the complete expression is unconditional. Only the selected
    // value's arithmetic error contributes to a short-circuit result.
    if (op == OP_LAND || op == OP_LOR) {
        bool use_right = op == OP_LAND ? a != 0 : a == 0;
        return PPValue(use_right ? b != 0 : a != 0, false,
                       left.error || (use_right && right.error));
    }
    bool less = uns ? a < b : (a ^ sign_bit) < (b ^ sign_bit);
    switch (op) {
    case OP_PLUS: result.bits = a + b; break;
    case OP_MINUS: result.bits = a - b; break;
    case OP_STAR: result.bits = a * b; break;
    case OP_DIV:
    case OP_MOD: {
        if (!b || (!uns && a == sign_bit && b == ~std::uint64_t(0))) {
            result.error = true;
            break;
        }
        bool negative_a = !uns && (a & sign_bit), negative_b = !uns && (b & sign_bit);
        std::uint64_t magnitude_a = negative_a ? 0 - a : a;
        std::uint64_t magnitude_b = negative_b ? 0 - b : b;
        result.bits = op == OP_DIV ? magnitude_a / magnitude_b : magnitude_a % magnitude_b;
        if (op == OP_DIV ? negative_a != negative_b : negative_a) result.bits = 0 - result.bits;
        break;
    }
    case OP_LSHIFT:
    case OP_RSHIFT:
        result.is_unsigned = left.is_unsigned;
        if (b >= 64) { result.error = true; break; }
        result.bits = op == OP_LSHIFT ? a << b : a >> b;
        if (op == OP_RSHIFT && !left.is_unsigned && (a & sign_bit) && b)
            result.bits |= ~std::uint64_t(0) << (64 - b);
        break;
    case OP_LT: result.bits = less; result.is_unsigned = false; break;
    case OP_GT: result.bits = !less && a != b; result.is_unsigned = false; break;
    case OP_LE: result.bits = less || a == b; result.is_unsigned = false; break;
    case OP_GE: result.bits = !less; result.is_unsigned = false; break;
    case OP_EQ: result.bits = a == b; result.is_unsigned = false; break;
    case OP_NE: result.bits = a != b; result.is_unsigned = false; break;
    case OP_AMP: result.bits = a & b; break;
    case OP_XOR: result.bits = a ^ b; break;
    case OP_BOR: result.bits = a | b; break;
    default: result.error = true; break;
    }
    return result;
}

} // namespace cppgm
