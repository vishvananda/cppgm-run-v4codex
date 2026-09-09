#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
IdentifierId Analyzer::literal_name(IdentifierId suffix)
{
    if (auto known = literal_names.get(suffix)) return known;
    TextView text = ids.spelling(suffix);
    std::string name = "operator\"\"" + std::string(text.data, text.size);
    auto id = ids.intern(TextView(name.data(), name.size())); literal_names.put(suffix, id); return id;
}
Expression Analyzer::literal_call(NodeId n, ScopeId s)
{
    auto lit = ast.literals[ast[n].literal];
    if (lit.kind != LiteralKind::string) throw std::runtime_error("unsupported literal operator category");
    TypeId pointer_type = types.compound(TypeKind::Pointer, types.qualify(types.fundamental(lit.type), 1));
    EntityId selected = 0;
    for (EntityId e : candidates(lookup(s, literal_name(lit.suffix)))) {
        if (!e || entities[e].kind != EntityKind::Function) continue;
        ++candidate_work;
        Type f = types[entities[e].type];
        if (f.count != 2 || types.parameters[f.offset] != pointer_type ||
            !fundamental(types.parameters[f.offset+1], FT_UNSIGNED_LONG_INT)) continue;
        if (selected) throw std::runtime_error("ambiguous string literal operator");
        selected = e;
    }
    if (!selected) throw std::runtime_error("no string literal operator");
    TypeId returned = types[entities[selected].type].child;
    facts[n].entity = selected; facts[n].type = returned;
    Expression result; result.type = value_type(returned); result.form = ExpressionForm::LiteralCall;
    result.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    return result;
}
} }
