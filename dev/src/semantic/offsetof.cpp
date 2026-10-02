#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
QueryId Analyzer::offsetof_query(NodeId n, ScopeId s)
{
    auto first = ast.first(n);
    TypeQuery root; root.kind = QueryKind::Offsetof; root.type = type_id(first,s);
    auto prior = intern_query(root,{});
    std::vector<QueryId> children; children.reserve(2);
    for (auto part = ast.next(first); part; part = ast.next(part)) {
        TypeQuery step; step.kind = QueryKind::Offsetof; step.context = s;
        children.clear(); children.push_back(prior);
        if (ast.kind(part) == syntax::Kind::Identifier) {
            step.op = OP_DOT; step.name = ast.text(part);
        } else {
            step.op = OP_LSQUARE; children.push_back(expression_query(ast.first(part),s));
        }
        prior = intern_query(step,children);
        offsetof_path_queries.put(part,prior);
    }
    return prior;
}
TypeQueryFact Analyzer::query_offsetof(QueryId id, const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    TypeQueryFact result; result.expression.type = types.fundamental(FT_UNSIGNED_LONG_INT);
    auto type = q.type ? q.type : children[0].declared_type;
    OffsetofStep step;
    if (q.op == OP_DOT) {
        if (!class_value(type)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        auto cls = types[type].entity; complete_class(cls); size(type);
        auto field = imported(entities[cls].scope,q.name,Lookup::Ordinary,++walk);
        if (!field || field == ~EntityId(0) || !nonstatic_field(field) || field_fact(field).bit_field)
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        if (!accessible(field,q.context,entities[cls].scope,type))
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        auto owner = scopes[entities[field].owner].entity;
        auto storage = injected_storage(field);
        while (storage && nonstatic_field(storage)) {
            owner = scopes[entities[storage].owner].entity; storage = injected_storage(storage);
        }
        if (owner != cls) {
            auto path = base_steps(type,owner);
            if (!path || base_adjustments[path].virtual_row)
                return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
            step.offset = base_adjustments[path].total;
        }
        step.offset += field_projection(field,entities[owner].type).offset;
        type = entities[field].type;
        if (types[type].kind == TypeKind::LRef || types[type].kind == TypeKind::RRef)
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    } else if (q.op == OP_LSQUARE) {
        if (types[type].kind != TypeKind::Array || (!integral(children[1].expression.type) || scoped_enum(children[1].expression.type)))
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        type = types[type].child; step.stride = size(type);
    } else if (!class_value(type)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    result.declared_type = type;
    offsetof_layout_index.put(id,offsetof_layouts.size()); offsetof_layouts.push_back(step);
    return result;
}
} }
