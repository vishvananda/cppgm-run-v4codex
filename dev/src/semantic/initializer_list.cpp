#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeId Analyzer::initializer_list_element(TypeId type) const
{
    if (types[type].kind == TypeKind::LRef || types[type].kind == TypeKind::RRef) type = types[type].child;
    if (!class_value(type)) return 0;
    auto spec = entities[types[type].entity].specialization;
    if (!spec || specializations[spec].pattern != initializer_list_template) return 0;
    auto args = argument_packs[specializations[spec].arguments];
    return args.count == 1 ? argument_types[args.offset] : 0;
}
bool Analyzer::complete_builtin_list(EntityId e)
{
    auto element = initializer_list_element(entities[e].type);
    if (!element || templates[entities[initializer_list_template].template_info].body) return false;
    // The course permits a forward declaration of the standard class. Its
    // library ABI is a const-element pointer followed by size_t. These are
    // semantic declarations, not fabricated syntax or a replacement user body.
    if (!entities[e].scope) entities[e].scope = make_scope(ScopeKind::Class,entities[e].owner,entities[e].name,e,false);
    auto scope = entities[e].scope;
    auto add = [&](const char* name, unsigned length, TypeId type) {
        auto id = ids.intern(TextView(name,length));
        auto field = make_entity(EntityKind::Variable,scope,id,0);
        entities[field].type = type; bind(scope,id,field); record(scope,field,0,type,EntityKind::Variable);
        return field;
    };
    InitializerListType fact; fact.element = element;
    fact.begin = add("__begin",7,types.compound(TypeKind::Pointer,types.qualify(element,1)));
    fact.size = add("__size",6,types.fundamental(FT_UNSIGNED_LONG_INT));
    initializer_list_type_index.put(e,initializer_list_types.size());
    initializer_list_types.push_back(fact);
    entities[e].complete = true;
    specializations[entities[e].specialization].body = FactState::Success;
    return true;
}
InitializerListType Analyzer::initializer_list_type(TypeId type)
{
    type = value_type(type);
    auto element = initializer_list_element(type);
    if (!element) return InitializerListType();
    auto entity = types[type].entity;
    if (auto id = initializer_list_type_index.get(entity)) return initializer_list_types[id];
    size(type);
    if (auto id = initializer_list_type_index.get(entity)) return initializer_list_types[id];
    InitializerListType fact; fact.element = element;
    for (auto d = scopes[entities[entity].scope].first_decl; d; d = declarations[d].next) {
        auto field = declarations[d].entity;
        if (!nonstatic_field(field)) continue;
        if (!fact.begin) fact.begin = field;
        else if (!fact.size) fact.size = field;
        else throw std::runtime_error("unsupported initializer_list representation");
    }
    if (!fact.begin || !fact.size || entities[fact.begin].type != types.compound(TypeKind::Pointer,types.qualify(element,1)) ||
        entities[fact.size].type != types.fundamental(FT_UNSIGNED_LONG_INT))
        throw std::runtime_error("invalid initializer_list representation");
    initializer_list_type_index.put(entity,initializer_list_types.size()); initializer_list_types.push_back(fact);
    return fact;
}
std::vector<Expression> Analyzer::list_elements(Expression list)
{
    std::vector<Expression> result;
    if (list.inputs == CallInputs::Query) {
        auto q = type_queries[list.arguments];
        for (unsigned i = 0; i < q.count; ++i) result.push_back(query_fact(query_edges[q.offset+i]).expression);
    } else for (auto n = ast[list.arguments].first; n; n = ast[n].next) result.push_back(expressions[n]);
    return result;
}
TypeId Analyzer::deduce_initializer_list(NodeId source, ScopeId scope)
{
    if (!initializer_list_template) throw std::runtime_error("list deduction requires std::initializer_list");
    expand_expression_list(source,scope);
    TypeId element = 0;
    for (auto n = ast[source].first; n; n = ast[n].next) {
        auto value = expression(n,scope);
        auto type = types.unqualified(decay(value.type));
        if (!type || (element && element != type)) throw std::runtime_error("conflicting list element deduction");
        element = type;
    }
    if (!element) throw std::runtime_error("empty list cannot deduce auto");
    auto e = specialize_class(initializer_list_template,{element});
    complete_class(e); return entities[e].type;
}
void Analyzer::retain_list_backing(EntityId e)
{
    if (!initializer_list_element(entities[e].type) || !entities[e].initializer) return;
    auto n = entities[e].initializer;
    auto init = class_initialization(n,entities[e].type);
    auto c = conversions[init.conversion];
    if (init.source) n = init.source;
    while (ast[n].kind == syntax::Kind::Initializer || ast[n].kind == syntax::Kind::Parenthesized) n = ast[n].first;
    if (!init.source) c = conversions[expressions[n].incoming];
    if (c.kind != Conversion::Kind::List && expressions[n].form == ExpressionForm::ListValue)
        c = conversions[expressions[n].conversions];
    if (c.kind != Conversion::Kind::List) return; // Copies do not extend the backing lifetime.
    auto object = list_objects[c.materialization];
    if (!object.backing) return;
    if (static_temporaries.get(object.backing) || reference_temporary(e) == object.backing || reference_choices(e)) return;
    bool persistent = entities[e].is_static || scopes[entities[e].owner].kind == ScopeKind::Namespace;
    std::vector<unsigned> pending{c.materialization};
    std::vector<EntityId> backing;
    while (!pending.empty()) {
        auto id = pending.back(); pending.pop_back();
        auto list = list_objects[id];
        if (!list.backing) continue;
        backing.push_back(list.backing);
        // Only an initializer-list element owns a nested backing array.
        // Constructor call arguments retain full-expression duration.
        if (!initializer_list_element(list_plans[list.plan].backing_element)) continue;
        for (unsigned i = list.call.argument_count; i; --i) {
            auto element = conversions[list.call.conversions+i-1];
            if (element.kind == Conversion::Kind::List) pending.push_back(element.materialization);
        }
    }
    if (persistent) for (auto array : backing) {
        entities[array].definition = n; entities[array].is_static = true;
        ReferenceStorage storage; storage.object = array; storage.reference = e;
        static_temporaries.put(array,reference_storage.size()); reference_storage.push_back(storage);
    } else if (backing.size() == 1) {
        reference_temporaries.put(e,object.backing);
        object_destructors.put(e,object_destructor(object.backing));
    } else for (auto array : backing) {
        auto next = reference_choices(e);
        conditional_references.put(e,reference_alternatives.size());
        reference_alternatives.push_back({array,next});
    }
}
} }
