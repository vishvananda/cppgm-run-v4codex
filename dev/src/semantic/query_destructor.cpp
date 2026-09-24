#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeId Analyzer::destructor_target(NodeId name, TypeId object, ScopeId scope)
{
    auto part = ast[name].last, id = ast[part].first;
    if (ast[id].kind == syntax::Kind::TypeId) return type_id(id,scope);
    EntityId found = 0;
    if (class_value(object) || pattern_class_type(object))
        found = lookup(entities[types[object].entity].scope,ast[id].text,Lookup::Ordinary,true);
    if (!found) found = lookup(scope,ast[id].text);
    if (!found || (entities[found].kind != EntityKind::Type && entities[found].kind != EntityKind::Alias)) return 0;
    if (child(part,syntax::Kind::TemplateArguments)) found = class_template_name(part,found,scope);
    return found ? entities[found].type : 0;
}
TypeQueryFact Analyzer::query_destructor(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    TypeQueryFact result; auto object = children[0].expression; auto type = object.type;
    if (!q.value) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    if (q.op == OP_ARROW) {
        result.arrow = prepare_arrow(object,q.context,0,false);
        if (result.arrow) type = arrow_chains[result.arrow].type;
        if (!pointer(type)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        type = types[type].child;
    }
    if (!q.type || (types.unqualified(q.type) != types.unqualified(type) && !derived_from(type,q.type)))
        return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    if (children.size() > 1 && types.unqualified(children[1].expression.type) != types.unqualified(q.type))
        return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    auto& x = result.expression;
    if (!class_value(type)) {
        if (!arithmetic(type) && !pointer(type) && !fundamental(type,FT_NULLPTR_T) &&
            !(types[type].kind == TypeKind::Named && entities[types[type].entity].underlying))
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        x.type = types.function(types.fundamental(FT_VOID),{},false);
        x.form = ExpressionForm::PseudoDestructor; return result;
    }
    complete_class(types[type].entity);
    if (!entities[types[type].entity].complete) return incomplete_query(type);
    auto e = destructor_declaration(q.type);
    if (!e) return incomplete_query(type);
    if (members[entities[e].member_info].deleted) return TypeQueryFact::failed(TypeQueryFact::Failure::Deleted);
    if (!accessible(e,q.context,entities[e].owner,type)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    if (!default_destructor_valid(e)) return TypeQueryFact::failed(TypeQueryFact::Failure::Deleted);
    x = member_value(e,types[type].cv,object.category); x.form = ExpressionForm::Overload; x.type = 0;
    record_object(x,0,0,0); object_uses[x.object_use].naming_scope = entities[e].owner;
    return result;
}
} }
