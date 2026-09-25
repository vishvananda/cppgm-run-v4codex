#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
QueryId Analyzer::new_query(NodeId n, ScopeId s)
{
    TypeQuery q; std::vector<QueryId> children;
    q.kind = QueryKind::New; q.context = s;
    while (!template_object_context_index.get(q.context) &&
        (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block))
        q.context = scopes[q.context].parent;
    q.type = type_id(child(n,Kind::TypeId),s); q.value = child(n,Kind::Global) != 0;
    TypeQuery type; type.kind = QueryKind::TypeValue; type.type = q.type;
    std::vector<QueryId> args(1,intern_query(type,{}));
    auto init = child(n,Kind::Initializer);
    auto list = ast[init].first;
    for (auto a = ast[list].first; a; a = ast[a].next) args.push_back(expression_query(a,s));
    TypeQuery call; call.kind = QueryKind::Call; call.context = q.context;
    children.push_back(intern_query(call,args));
    auto placement = ast[child(n,Kind::Placement)].first;
    for (auto a = ast[placement].first; a; a = ast[a].next) children.push_back(expression_query(a,s));
    if (template_type_probe) for (auto child : children) if (!child) return 0;
    return intern_query(q,children);
}
TypeQueryFact Analyzer::query_new(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    // This unevaluated owner checks allocation and construction declarations;
    // it creates no runtime object, initializer occurrence or body demand.
    auto allocated = q.type;
    if (!size(allocated,false,true) || types[allocated].kind == TypeKind::LRef || types[allocated].kind == TypeKind::RRef)
        return incomplete_query(allocated);
    auto name = operator_name(KW_NEW);
    EntityId family = 0;
    if (!q.value && class_value(allocated))
        family = lookup(entities[types[allocated].entity].scope,name,Lookup::Ordinary,true);
    if (!family) {
        if (children.size() == 1) global_allocation(KW_NEW,false);
        family = lookup(global,name);
    }
    std::vector<Expression> args(1); args[0].type = types.fundamental(FT_UNSIGNED_LONG_INT);
    for (unsigned j = 1; j < children.size(); ++j) args.push_back(children[j].expression);
    std::vector<Conversion> conversions;
    auto choice = select_call(family,args,0,0,ValueCategory::Prvalue,0,0,conversions);
    if (choice.failure != CallFailure::None || deleted_transfer(choice.entity))
        return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
    check_access(choice.entity,q.context,entities[choice.entity].owner);
    auto result = types[entities[choice.entity].type].child;
    if (!pointer(result) || !fundamental(types[result].child,FT_VOID))
        throw std::runtime_error("allocation function must return void pointer");
    TypeQueryFact fact; fact.expression.type = types.compound(TypeKind::Pointer,allocated);
    fact.selected = choice.entity;
    fact.expression.conversions = this->conversions.size(); fact.expression.count = conversions.size();
    this->conversions.insert(this->conversions.end(),conversions.begin(),conversions.end());
    return fact;
}
} }
