#include "lowering/procedural.h"
#include "abi/itanium/operations.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
namespace {
const char* query_operation(ETokenType op, bool unary)
{
    switch (op) {
    case OP_PLUS: return unary ? "ps" : "pl";
    case OP_MINUS: return unary ? "ng" : "mi";
    case OP_STAR: return unary ? "de" : "ml";
    case OP_AMP: return unary ? "ad" : "an";
    case OP_DIV: return "dv"; case OP_MOD: return "rm";
    case OP_COMPL: return "co"; case OP_LNOT: return "nt";
    case OP_BOR: return "or"; case OP_XOR: return "eo";
    case OP_LSHIFT: return "ls"; case OP_RSHIFT: return "rs";
    case OP_EQ: return "eq"; case OP_NE: return "ne";
    case OP_LT: return "lt"; case OP_GT: return "gt";
    case OP_LE: return "le"; case OP_GE: return "ge";
    case OP_LAND: return "aa"; case OP_LOR: return "oo";
    case OP_COMMA: return "cm"; case OP_ARROW: return "pt";
    case OP_DOT: return "dt";
    case OP_LSQUARE: return "ix";
    default: throw std::logic_error("missing type-query ABI operation");
    }
}
}
abi_mangle::Id Procedural::abi_query(semantic::QueryId id)
{
    using semantic::QueryKind; using abi_mangle::Kind;
    if (auto old = abi_queries.get(id)) return old;
    auto q = sem.type_query(id); abi_mangle::Id result = 0;
    auto child = [&](unsigned i) { return abi_query(sem.type_query_child(id,i)); };
    std::vector<abi_mangle::Id> args;
    auto pack = sem.query_arguments(q.arguments);
    for (unsigned j = 0; j < pack.count; ++j)
        args.push_back(abi_argument(sem.template_argument(pack.offset+j)));
    switch (q.kind) {
    case QueryKind::Value: result = abi.make(Kind::Value,abi_type(q.type),0,0,q.value); break;
    case QueryKind::TemplateValueParameter: result = abi.make(Kind::ExprParameter,0,0,0,sem.template_ordinal(q.entity)); break;
    case QueryKind::Parameter: result = abi.make(Kind::ExprFunctionParameter,0,0,0,q.value); break;
    case QueryKind::Name: {
        auto entity = sem.entities[q.entity];
        auto name = abi.name(q.name ? 0 : abi_scope(entity.owner),spelling(q.name ? q.name : entity.name));
        result = abi.make(Kind::UnresolvedName,name,q.arguments!=0,0,0,args); break;
    }
    case QueryKind::QualifiedValue: result = abi.make(Kind::Member,abi_type(q.type),abi.string(spelling(q.name))); break;
    case QueryKind::Parenthesized: result = child(0); break;
    case QueryKind::Conditional: result = abi.make(Kind::Conditional,child(0),child(1),child(2)); break;
    case QueryKind::Cast:
        result = q.op == KW_STATIC_CAST ? abi.make(Kind::Cast,abi_type(q.type),child(0),abi_mangle::operation("sc")) :
            abi.make(Kind::Conversion,abi_type(q.type),0,0,0,{child(0)}); break;
    case QueryKind::Unary: result = abi.make(Kind::Unary,child(0),abi_mangle::operation(query_operation(q.op,true))); break;
    case QueryKind::Binary: result = abi.make(Kind::Binary,child(0),child(1),abi_mangle::operation(query_operation(q.op,false))); break;
    case QueryKind::Member: {
        auto op = abi_mangle::operation(query_operation(q.op,false));
        if (!q.type) result = abi.make(Kind::ObjectMember,child(0),abi.string(spelling(q.name)),op,0,args);
        else {
            auto owner = abi_type(q.type); bool known = true;
            for (auto part = owner; part; part = abi[part].a)
                if (abi[part].kind != Kind::Name && abi[part].kind != Kind::Template) { known = false; break; }
            auto name = known ? abi.make(Kind::UnresolvedName,abi.name(owner,spelling(q.name)),q.arguments!=0,0,0,args) :
                abi.make(Kind::Member,owner,abi.string(spelling(q.name)),0,0,args);
            result = abi.make(Kind::Binary,child(0),name,op);
        }
        break;
    }
    case QueryKind::Call: {
        auto callee = sem.type_query(sem.type_query_child(id,0)); args.clear();
        for (unsigned i = 1; i < q.count; ++i) args.push_back(child(i));
        result = callee.kind == QueryKind::TypeValue ? abi.make(Kind::Conversion,abi_type(callee.type),0,0,0,args) :
            abi.make(Kind::Call,child(0),0,0,0,args); break;
    }
    case QueryKind::Expansion: result = abi.make(Kind::ExprPack,child(0)); break;
    case QueryKind::New: throw std::logic_error("new-expression ABI query is not yet represented");
    case QueryKind::SizeofPack: {
        if (q.entity) {
            bool parameter = sem.entities[q.entity].template_parameter;
            auto operand = abi.make(parameter ? Kind::ExprParameter : Kind::ExprFunctionParameter,0,0,0,
                parameter ? sem.template_ordinal(q.entity) : q.value);
            result = abi.make(Kind::SizeofPack,operand);
        } else {
            args.clear();
            auto captured = sem.pack_arguments(sem.template_argument(pack.offset));
            for (unsigned j = 0; j < captured.count; ++j) args.push_back(abi_argument(sem.template_argument(captured.offset+j)));
            result = abi.make(Kind::SizeofPack,0,0,0,0,args);
        }
        break;
    }
    case QueryKind::Sizeof:
        result = q.type ? abi.make(q.op == KW_ALIGNOF ? Kind::AlignofType : Kind::SizeofType,abi_type(q.type)) :
            abi.make(Kind::Unary,child(0),abi_mangle::operation(q.op == KW_ALIGNOF ? "az" : "sz")); break;
    case QueryKind::String: throw std::logic_error("string literal is not a type-dependent ABI expression");
    case QueryKind::TypeValue: throw std::logic_error("type-query type used as ABI expression");
    }
    abi_queries.put(id,result); return result;
}
} }
