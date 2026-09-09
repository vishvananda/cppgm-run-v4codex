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
IdentifierId Analyzer::operator_name(ETokenType op)
{
    if (auto id = operator_names.get(op)) return id;
    const char* spelling = 0;
    switch (op) {
    case KW_NEW: spelling = "operator new"; break;
    case KW_DELETE: spelling = "operator delete"; break;
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
    operator_names.put(op, id); return id;
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
    if (op == KW_NEW || op == KW_DELETE) { entities[e].is_static |= member; return; }
    bool class_operand = member;
    for (unsigned i = 0; i < function.count; ++i)
        class_operand |= types[value_type(types.parameters[function.offset+i])].kind == TypeKind::Named;
    if (!class_operand) throw std::runtime_error("operator needs a class or enum operand");
    if (!member && (op == OP_ASS || op == OP_LPAREN || op == OP_LSQUARE || op == OP_ARROW))
        throw std::runtime_error("operator must be a member");
    if (member && entities[e].is_static) throw std::runtime_error("operator cannot be static");
}
} }
