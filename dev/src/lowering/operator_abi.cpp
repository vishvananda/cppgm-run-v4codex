#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
abi_mangle::AbiTerminalKind Procedural::operator_terminal(EntityId id) const
{
    using namespace abi_mangle;
    const auto& e = sem.entities[id];
    unsigned arity = sem.types[e.type].count + (e.member_info && !e.is_static);
    switch (e.key) {
    case KW_NEW: return e.array_allocation ? ABI_TERMINAL_NEW_ARRAY : ABI_TERMINAL_NEW;
    case KW_DELETE: return e.array_allocation ? ABI_TERMINAL_DELETE_ARRAY : ABI_TERMINAL_DELETE;
    case OP_PLUS: return ABI_TERMINAL_PLUS;
    case OP_MINUS: return ABI_TERMINAL_MINUS;
    case OP_STAR: return arity == 1 ? ABI_TERMINAL_DEREFERENCE : ABI_TERMINAL_MULTIPLY;
    case OP_AMP: return arity == 1 ? ABI_TERMINAL_ADDRESS_OF : ABI_TERMINAL_BIT_AND;
    case OP_DIV: return ABI_TERMINAL_DIVIDE;
    case OP_MOD: return ABI_TERMINAL_REMAINDER;
    case OP_BOR: return ABI_TERMINAL_BIT_OR;
    case OP_XOR: return ABI_TERMINAL_BIT_XOR;
    case OP_COMPL: return ABI_TERMINAL_COMPLEMENT;
    case OP_ASS: return ABI_TERMINAL_ASSIGN;
    case OP_PLUSASS: return ABI_TERMINAL_PLUS_ASSIGN;
    case OP_MINUSASS: return ABI_TERMINAL_MINUS_ASSIGN;
    case OP_STARASS: return ABI_TERMINAL_MULTIPLY_ASSIGN;
    case OP_DIVASS: return ABI_TERMINAL_DIVIDE_ASSIGN;
    case OP_MODASS: return ABI_TERMINAL_REMAINDER_ASSIGN;
    case OP_BANDASS: return ABI_TERMINAL_AND_ASSIGN;
    case OP_BORASS: return ABI_TERMINAL_OR_ASSIGN;
    case OP_XORASS: return ABI_TERMINAL_XOR_ASSIGN;
    case OP_LSHIFT: return ABI_TERMINAL_LEFT_SHIFT;
    case OP_RSHIFT: return ABI_TERMINAL_RIGHT_SHIFT;
    case OP_LSHIFTASS: return ABI_TERMINAL_LEFT_SHIFT_ASSIGN;
    case OP_RSHIFTASS: return ABI_TERMINAL_RIGHT_SHIFT_ASSIGN;
    case OP_EQ: return ABI_TERMINAL_EQUAL;
    case OP_NE: return ABI_TERMINAL_NOT_EQUAL;
    case OP_LT: return ABI_TERMINAL_LESS;
    case OP_GT: return ABI_TERMINAL_GREATER;
    case OP_LE: return ABI_TERMINAL_LESS_EQUAL;
    case OP_GE: return ABI_TERMINAL_GREATER_EQUAL;
    case OP_LNOT: return ABI_TERMINAL_LOGICAL_NOT;
    case OP_LAND: return ABI_TERMINAL_LOGICAL_AND;
    case OP_LOR: return ABI_TERMINAL_LOGICAL_OR;
    case OP_INC: return ABI_TERMINAL_INCREMENT;
    case OP_DEC: return ABI_TERMINAL_DECREMENT;
    case OP_COMMA: return ABI_TERMINAL_COMMA;
    case OP_ARROW: return ABI_TERMINAL_ARROW;
    case OP_ARROWSTAR: return ABI_TERMINAL_MEMBER_POINTER;
    case OP_LPAREN: return ABI_TERMINAL_CALL;
    case OP_LSQUARE: return ABI_TERMINAL_INDEX;
    default: return ABI_TERMINAL_NONE;
    }
}
} }
