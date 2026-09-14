#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Expression Analyzer::numeric_literal_call(NodeId n, ScopeId s)
{
    auto lit = ast.literals[ast[n].literal];
    auto cooked = lit.kind == LiteralKind::integer ? types.fundamental(FT_UNSIGNED_LONG_LONG_INT) :
        lit.kind == LiteralKind::floating ? types.fundamental(FT_LONG_DOUBLE) : types.fundamental(lit.type);
    auto raw_type = types.compound(TypeKind::Pointer,types.qualify(types.fundamental(FT_CHAR),1));
    EntityId scalar = 0, raw = 0, pattern = 0;
    auto select = [](EntityId& selected, EntityId e) {
        if (selected && selected != e) throw std::runtime_error("ambiguous literal operator");
        selected = e;
    };
    for (auto e : candidates(lookup(s,literal_name(lit.suffix)))) {
        if (entities[e].kind != EntityKind::Function) continue;
        ++candidate_work;
        auto f = types[entities[e].type];
        if (entities[e].template_info) {
            auto head = templates[entities[e].template_info];
            if (head.count != 1 || f.count || f.variadic) continue;
            auto p = template_parameters[head.offset];
            if (entities[p].kind == EntityKind::Parameter && entities[p].parameter_pack && fundamental(entities[p].type,FT_CHAR))
                select(pattern,e);
        } else if (f.count == 1 && !f.variadic) {
            auto parameter = types.parameters[f.offset];
            if (parameter == cooked) select(scalar,e);
            else if (parameter == raw_type) select(raw,e);
        }
    }
    EntityId selected = scalar; auto kind = LiteralCallKind::Scalar;
    if (scalar && !lit.cooked_valid) throw std::runtime_error("cooked literal value out of range");
    if (!scalar && lit.kind != LiteralKind::character) {
        if (raw && pattern) throw std::runtime_error("both raw and template literal operators are visible");
        if (raw) { selected = raw; kind = LiteralCallKind::Raw; }
        else if (pattern) {
            std::vector<ArgumentId> args;
            auto spelling = ids.spelling(lit.prefix);
            for (unsigned i = 0; i < spelling.size; ++i) {
                TypeQuery q; q.type = types.fundamental(FT_CHAR); q.value = static_cast<unsigned char>(spelling.data[i]);
                args.push_back(value_argument_id(intern_query(q,{})));
            }
            selected = specialize(pattern,{make_argument_pack(args)}); kind = LiteralCallKind::Pack;
        }
    }
    if (!selected) throw std::runtime_error("no literal operator for this category");
    demand_specialization(selected);
    auto returned = types[entities[selected].type].child;
    auto& f = facts.edit(n); f.entity = selected; f.type = returned;
    literal_call_kinds.put(n,unsigned(kind)+1);
    Expression result; result.type = value_type(returned); result.form = ExpressionForm::LiteralCall;
    if (kind == LiteralCallKind::Scalar) {
        Expression argument; argument.type = types.fundamental(lit.type);
        auto conversion = standard_conversion(argument,cooked);
        if (!conversion.valid()) throw std::logic_error("invalid cooked literal conversion");
        result.conversions = conversions.size(); result.count = 1; conversions.push_back(conversion);
    }
    result.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    return result;
}
} }
