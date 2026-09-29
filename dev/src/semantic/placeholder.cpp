#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::placeholder_type()
{
    if (!placeholder_parameter) {
        placeholder_parameter = make_entity(EntityKind::Type,0,0,0);
        entities[placeholder_parameter].template_parameter = true;
        entities[placeholder_parameter].type = types.named(placeholder_parameter);
    }
    return entities[placeholder_parameter].type;
}
bool Analyzer::placeholder_type(TypeId t)
{
    if (!t || !placeholder_parameter) return false;
    if (placeholder_types.size() <= t) placeholder_types.resize(types.records.size());
    if (placeholder_types[t]) return placeholder_types[t] == 2;
    ++placeholder_type_work;
    auto type = types[t];
    bool found = (type.kind == TypeKind::Named && type.entity == placeholder_parameter) || placeholder_type(type.child);
    placeholder_types[t] = found ? 2 : 1;
    return found;
}
TypeId Analyzer::deduce_placeholder(TypeId pattern, Expression value, TypeId& deduced)
{
    auto p = types[pattern];
    auto actual = value.type;
    if (fundamental(actual,FT_VOID) && types.unqualified(pattern) != placeholder_type())
        throw std::runtime_error("void cannot deduce a pointer or reference placeholder");
    auto deduction = pattern;
    bool ref = p.kind == TypeKind::LRef || p.kind == TypeKind::RRef;
    if (ref) {
        deduction = p.child;
        if (p.kind == TypeKind::RRef && p.child == placeholder_type() && value.category == ValueCategory::Lvalue)
            actual = types.compound(TypeKind::LRef,actual);
    } else actual = types.unqualified(decay(actual));
    Index bindings, cache;
    if (!deduce_type(deduction,actual,bindings) || !bindings.get(placeholder_parameter))
        throw std::runtime_error("auto deduction failed");
    auto type = substitute_type(pattern,bindings,cache);
    deduced = bindings.get(placeholder_parameter);
    return type;
}
TypeId Analyzer::deduced_object_type(NodeId specs, NodeId d, NodeId init, ScopeId s, TypeId& deduced)
{
    auto source = init;
    while (source && (ast[source].kind == Kind::Initializer || ast[source].kind == Kind::ParenInitializer || ast[source].kind == Kind::ParenArguments)) {
        auto first = ast[source].first;
        if (ast[first].next) throw std::runtime_error("auto requires a single initializer");
        source = first;
    }
    if (!source || ast[source].kind == Kind::BracedInit) throw std::runtime_error("auto requires an expression initializer");
    auto name = decl_name(d), owner = name_owner(name,s,true);
    auto id = terminal(name), previous = local(owner,id);
    if (previous && entities[previous].kind != EntityKind::Type) throw std::runtime_error("duplicate auto declaration");
    // The declarator's scope starts before its initializer. Publish the
    // canonical entity with an explicitly pending type so a shadowed outer
    // name cannot supply a spurious successful self-deduction.
    auto object = make_entity(EntityKind::Variable,owner,id,d);
    placeholder_objects.put(object,d); bind(owner,id,object);
    auto value = expression(source,s);
    placeholder_type();
    unsigned cv = (spec_has(specs,KW_CONST) ? 1 : 0) | (spec_has(specs,KW_VOLATILE) ? 2 : 0);
    auto base = types.qualify(entities[placeholder_parameter].type,cv);
    auto saved = deducing_placeholder; deducing_placeholder = true;
    TypeId pattern;
    try { pattern = declarator(d,base,s); }
    catch (...) { deducing_placeholder = saved; throw; }
    deducing_placeholder = saved;
    auto type = deduce_placeholder(pattern,value,deduced);
    facts.edit(d).type = type;
    return type;
}
} }
