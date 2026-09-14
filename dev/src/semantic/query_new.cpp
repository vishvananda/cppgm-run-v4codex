#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeQueryFact Analyzer::query_new(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    // This unevaluated owner checks allocation and construction declarations;
    // it creates no runtime object, initializer occurrence or body demand.
    auto allocated = q.type;
    size(allocated);
    if (types[allocated].kind == TypeKind::LRef || types[allocated].kind == TypeKind::RRef)
        throw std::runtime_error("new cannot allocate a reference");
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
    if (choice.failure != CallFailure::None) throw std::runtime_error("invalid allocation in new-expression query");
    check_access(choice.entity,q.context,entities[choice.entity].owner);
    auto result = types[entities[choice.entity].type].child;
    if (!pointer(result) || !fundamental(types[result].child,FT_VOID))
        throw std::runtime_error("allocation function must return void pointer");
    TypeQueryFact fact; fact.expression.type = types.compound(TypeKind::Pointer,allocated);
    fact.selected = choice.entity; return fact;
}
} }
