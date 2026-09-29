#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeQueryFact Analyzer::query_delete(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    auto operand = children[0].expression;
    auto pointer = delete_operand_type(operand), target = types[pointer].child;
    if (!size(target,false,true)) return incomplete_query(target);
    TypeQueryFact fact;
    fact.selected = default_destructor(target,q.context,false);
    auto leaf = target;
    while (types[leaf].kind == TypeKind::Array) leaf = types[leaf].child;
    fact.deallocation = select_deallocation(leaf,q.value & 1,q.value & 2,q.context,false);
    auto c = conversion_value(operand,pointer);
    if (!valid_fixed_conversion(operand,0,c,q.context))
        return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    fact.expression.type = types.fundamental(FT_VOID);
    fact.expression.conversions = conversions.size(); fact.expression.count = 1;
    conversions.push_back(c); return fact;
}
QueryId Analyzer::allocation_query(NodeId n, ScopeId s)
{
    TypeQuery q; std::vector<QueryId> children;
    if (ast[n].kind == Kind::Delete) {
        q.kind = QueryKind::Delete; q.context = s;
        q.value = bool(child(n,Kind::ArrayDelete)) | (bool(child(n,Kind::Global)) << 1);
        auto operand = expression_query(ast[n].last,s);
        if (!operand && template_type_probe) return 0;
        return intern_query(q,{operand});
    }
    q.kind = QueryKind::New; q.context = s;
    while (!template_object_context_index.get(q.context) &&
        (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block))
        q.context = scopes[q.context].parent;
    q.type = type_id(child(n,Kind::TypeId),s); q.value = child(n,Kind::Global) != 0;
    TypeQuery type; type.kind = QueryKind::TypeValue; type.type = q.type;
    while (types[type.type].kind == TypeKind::Array) type.type = types[type.type].child;
    std::vector<QueryId> args(1,intern_query(type,{}));
    auto init = child(n,Kind::Initializer);
    auto list = ast[init].first;
    q.op = init ? (ast[list].kind == Kind::BracedInit ? OP_LBRACE : OP_LPAREN) : TOK_INVALID;
    TypeQuery call; call.kind = QueryKind::Call; call.context = q.context; call.value = 1;
    if (ast[list].kind == Kind::BracedInit) {
        call.op = OP_LBRACE; args.push_back(expression_query(list,s));
    } else for (auto a = ast[list].first; a; a = ast[a].next) args.push_back(expression_query(a,s));
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
    bool array = types[allocated].kind == TypeKind::Array;
    auto leaf = allocated;
    while (types[leaf].kind == TypeKind::Array) leaf = types[leaf].child;
    auto name = operator_name(KW_NEW,array);
    EntityId family = 0;
    if (!q.value && class_value(leaf))
        family = imported(entities[types[leaf].entity].scope,name,Lookup::Ordinary,++walk);
    if (family == ~EntityId(0)) return TypeQueryFact::failed(TypeQueryFact::Failure::Ambiguous);
    if (!family) {
        if (children.size() == 1) global_allocation(KW_NEW,array);
        family = lookup(global,name);
    }
    std::vector<Expression> args(1); args[0].type = types.fundamental(FT_UNSIGNED_LONG_INT);
    for (unsigned j = 1; j < children.size(); ++j) args.push_back(children[j].expression);
    std::vector<Conversion> conversions;
    auto choice = select_call(family,args,0,0,ValueCategory::Prvalue,0,0,conversions);
    if (choice.failure != CallFailure::None || deleted_transfer(choice.entity))
        return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
    if (!accessible(choice.entity,q.context,entities[choice.entity].owner))
        return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    for (unsigned i = 0; i < args.size(); ++i)
        if (!valid_fixed_conversion(args[i],0,conversions[i],q.context))
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    auto result = types[entities[choice.entity].type].child;
    if (!pointer(result) || !fundamental(types[result].child,FT_VOID))
        throw std::runtime_error("allocation function must return void pointer");
    TypeQueryFact fact; fact.expression.type = types.compound(TypeKind::Pointer,array ? types[allocated].child : allocated);
    fact.selected = choice.entity;
    fact.expression.conversions = this->conversions.size(); fact.expression.count = conversions.size();
    this->conversions.insert(this->conversions.end(),conversions.begin(),conversions.end());
    return fact;
}
} }
