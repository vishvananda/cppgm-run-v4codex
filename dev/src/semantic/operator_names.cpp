#include "semantic/analyzer.h"
#include <cstring>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
ETokenType Analyzer::operator_token(NodeId name) const
{
    if (!name || ast[ast[name].last].op != KW_OPERATOR) return TOK_INVALID;
    NodeId child = ast[ast[name].last].first;
    if (ast[child].kind == Kind::Parameters) return OP_LPAREN;
    if (ast[child].kind == Kind::Array) return OP_LSQUARE;
    return ast[child].op;
}
bool Analyzer::array_operator(NodeId name) const
{
    NodeId part = ast[name].last;
    ETokenType op = operator_token(name);
    return (op == KW_NEW || op == KW_DELETE) && ast[ast[part].last].kind == Kind::Array;
}
IdentifierId Analyzer::operator_name(ETokenType op, bool array)
{
    auto key = (std::uint64_t(op) << 1) | array;
    if (auto id = operator_names.get(key)) return id;
    const char* spelling = 0;
    switch (op) {
    case KW_NEW: spelling = array ? "operator new[]" : "operator new"; break;
    case KW_DELETE: spelling = array ? "operator delete[]" : "operator delete"; break;
    case OP_PLUS: spelling = "operator+"; break;
    case OP_MINUS: spelling = "operator-"; break;
    case OP_STAR: spelling = "operator*"; break;
    case OP_AMP: spelling = "operator&"; break;
    case OP_DIV: spelling = "operator/"; break;
    case OP_MOD: spelling = "operator%"; break;
    case OP_BOR: spelling = "operator|"; break;
    case OP_XOR: spelling = "operator^"; break;
    case OP_COMPL: spelling = "operator~"; break;
    case OP_ASS: spelling = "operator="; break;
    case OP_PLUSASS: spelling = "operator+="; break;
    case OP_MINUSASS: spelling = "operator-="; break;
    case OP_STARASS: spelling = "operator*="; break;
    case OP_DIVASS: spelling = "operator/="; break;
    case OP_MODASS: spelling = "operator%="; break;
    case OP_BANDASS: spelling = "operator&="; break;
    case OP_BORASS: spelling = "operator|="; break;
    case OP_XORASS: spelling = "operator^="; break;
    case OP_LSHIFT: spelling = "operator<<"; break;
    case OP_RSHIFT: spelling = "operator>>"; break;
    case OP_LSHIFTASS: spelling = "operator<<="; break;
    case OP_RSHIFTASS: spelling = "operator>>="; break;
    case OP_EQ: spelling = "operator=="; break;
    case OP_NE: spelling = "operator!="; break;
    case OP_LT: spelling = "operator<"; break;
    case OP_GT: spelling = "operator>"; break;
    case OP_LE: spelling = "operator<="; break;
    case OP_GE: spelling = "operator>="; break;
    case OP_LNOT: spelling = "operator!"; break;
    case OP_LAND: spelling = "operator&&"; break;
    case OP_LOR: spelling = "operator||"; break;
    case OP_INC: spelling = "operator++"; break;
    case OP_DEC: spelling = "operator--"; break;
    case OP_COMMA: spelling = "operator,"; break;
    case OP_ARROW: spelling = "operator->"; break;
    case OP_ARROWSTAR: spelling = "operator->*"; break;
    case OP_LPAREN: spelling = "operator()"; break;
    case OP_LSQUARE: spelling = "operator[]"; break;
    default: throw std::runtime_error("unsupported operator name");
    }
    IdentifierId id = ids.intern(TextView(spelling, std::strlen(spelling)));
    operator_names.put(key, id); return id;
}
void Analyzer::declare_operator(EntityId e, NodeId name)
{
    NodeId part = ast[name].last;
    if (ast[part].op == KW_OPERATOR && ast[ast[part].first].kind == Kind::Literal) {
        literal_functions.put(e, ast[ast[part].last].text); return;
    }
    ETokenType op = operator_token(name);
    if (op == TOK_INVALID) return;
    entities[e].key = op;
    Type function = types[entities[e].type];
    bool member = scopes[entities[e].owner].kind == ScopeKind::Class;
    if (op == KW_NEW || op == KW_DELETE) {
        entities[e].is_static |= member; entities[e].array_allocation = array_operator(name);
        if (op == KW_NEW && (!pointer(function.child) || !fundamental(types[function.child].child,FT_VOID) || !function.count ||
            !fundamental(types.parameters[function.offset],FT_UNSIGNED_LONG_INT))) throw std::runtime_error("invalid allocation signature");
        if (op == KW_DELETE && (!fundamental(function.child,FT_VOID) || !function.count ||
            types.parameters[function.offset] != types.compound(TypeKind::Pointer,types.fundamental(FT_VOID)))) throw std::runtime_error("invalid deallocation signature");
        return;
    }
    bool class_operand = member;
    for (unsigned i = 0; i < function.count; ++i)
        class_operand |= types[value_type(types.parameters[function.offset+i])].kind == TypeKind::Named;
    if (!class_operand) throw std::runtime_error("operator needs a class or enum operand");
    if (!member && (op == OP_ASS || op == OP_LPAREN || op == OP_LSQUARE || op == OP_ARROW))
        throw std::runtime_error("operator must be a member");
    if (member && entities[e].is_static) throw std::runtime_error("operator cannot be static");
}
} }
