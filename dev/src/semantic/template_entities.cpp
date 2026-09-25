#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::declare_template_parameters(NodeId params, ScopeId ts, std::uint32_t source_head, std::uint32_t frame)
{
    auto source = templates[source_head]; unsigned ordinal = 0;
    auto bind_parameter = [&](EntityId e) {
        if (source_head && frame) {
            auto arg = parameter_argument(e);
            if (entities[e].parameter_pack) arg = make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)});
            frame = argument_frame(frame,template_parameters[source.offset+ordinal],arg);
        }
        ++ordinal;
    };
    for (NodeId p = ast[ast[params].first].first; p; p = ast[p].next) {
        if (definitions && ast[p].kind == Kind::NonTypeParameter) {
            auto specs = ast[p].first;
            auto d = child(p,Kind::Declarator);
            TypeId type = 0;
            if (source_head && frame) {
                // The retained head already owns this type. Substitute it
                // under the enclosing specialization and earlier parameters
                // of this new head, without projecting another syntax region.
                Index bindings, cache;
                type = substitute_type(entities[template_parameters[source.offset+ordinal]].type,bindings,cache,frame);
                if (!type) throw std::runtime_error("invalid member template parameter type");
            } else type = declarator(d,specifiers(specs,ts),ts);
            if (types[type].kind == TypeKind::Array || types[type].kind == TypeKind::Function) type = decay(type);
            if (types[type].kind == TypeKind::RRef || (!dependent_type(type) && !integral(type) && !pointer(type) &&
                types[type].kind != TypeKind::LRef && !fundamental(type,FT_NULLPTR_T)))
                throw std::runtime_error("invalid non-type template parameter type");
            auto e = make_entity(EntityKind::Parameter,ts,terminal(decl_name(d)),p);
            entities[e].template_parameter = true;
            bool pack = child(p,Kind::ParameterPack) != 0;
            for (auto decl = d; decl; decl = ast[child(decl,Kind::NestedDeclarator)].first)
                pack |= child(decl,Kind::ParameterPack) != 0;
            entities[e].parameter_pack = pack;
            entities[e].type = types.unqualified(type);
            entities[e].initializer = child(p,Kind::DefaultTemplateArgument);
            bind(ts,entities[e].name,e); record(ts,e,p,type,EntityKind::Parameter);
            bind_parameter(e);
            continue;
        }
        if (ast[p].kind != Kind::TypeParameter) continue;
        NodeId identifier = child(p, Kind::Identifier);
        if (!identifier && !definitions) continue;
        IdentifierId name = ast[identifier].text;
        EntityId e = make_entity(EntityKind::Type, ts, name, p);
        entities[e].key = child(p, Kind::TemplateTemplate) ? KW_TEMPLATE : KW_TYPENAME;
        entities[e].template_parameter = true;
            entities[e].parameter_pack = child(p,Kind::ParameterPack) != 0;
        entities[e].initializer = child(p,Kind::DefaultTemplateArgument);
        entities[e].type = types.named(e);
        if (entities[e].key == KW_TEMPLATE && definitions) {
            auto nested = make_scope(ScopeKind::Template,ts);
            auto nested_head = source_head && frame ? entities[template_parameters[source.offset+ordinal]].template_info : 0;
            declare_template_parameters(child(p,Kind::TemplateParameters),nested,nested_head,frame);
            template_facts(e,nested);
            entities[e].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
        }
        bind(ts, name, e); record(ts, e, p, entities[e].type, EntityKind::Type);
        bind_parameter(e);
    }
}
std::uint32_t Analyzer::template_head_shape(EntityId e)
{
    auto index = entities[e].template_info;
    if (auto shape = template_head_shapes.get(index)) return shape;
    Index bindings, cache;
    auto shape = template_head_shape(e,bindings,cache,0);
    template_head_shapes.put(index,shape); return shape;
}
std::uint32_t Analyzer::template_head_shape(EntityId e, Index& bindings, Index& cache, unsigned depth)
{
    auto head = templates[entities[e].template_info];
    std::vector<ArgumentId> shape;
    // This scratch map grows over the head graph. Nested heads retain their
    // enclosing bindings; depth distinguishes their own parameter ordinals.
    for (unsigned j = 0; j < head.count; ++j) {
        auto parameter = template_parameters[head.offset+j];
        auto arg = canonical_argument(parameter,j,bindings,cache,depth);
        bindings.put(parameter,arg);
        shape.push_back(entities[parameter].parameter_pack ? types.compound(TypeKind::PackExpansion,0,arg) : arg);
    }
    return intern_arguments(shape);
}
EntityId Analyzer::template_entity(EntityId e) const
{
    if (!e) return 0;
    if (entities[e].template_info && (entities[e].kind == EntityKind::Alias || entities[e].class_info)) return e;
    if (entities[e].kind == EntityKind::Alias && types[entities[e].type].kind == TypeKind::Named) {
        auto target = types[entities[e].type].entity;
        if (entities[target].template_info && (entities[target].class_info || entities[target].kind == EntityKind::Alias)) return target;
    }
    return 0;
}
bool Analyzer::template_compatible(EntityId parameter, EntityId argument, Index& bindings, unsigned depth)
{
    if (!template_entity(argument)) return false;
    auto p = templates[entities[parameter].template_info], a = templates[entities[argument].template_info];
    Index cache;
    unsigned i = 0, j = 0;
    for (; i < p.count; ++i) {
        auto pe = template_parameters[p.offset+i];
        bool pack = entities[pe].parameter_pack;
        do {
            if (j == a.count) return pack && i+1 == p.count;
            auto ae = template_parameters[a.offset+j];
            if (entities[pe].kind != entities[ae].kind ||
                (entities[pe].key == KW_TEMPLATE) != (entities[ae].key == KW_TEMPLATE)) return false;
            if (entities[pe].key == KW_TEMPLATE && !template_compatible(pe,ae,bindings,depth+1)) return false;
            if (entities[pe].kind != EntityKind::Type) {
                auto pt = substitute_type(entities[pe].type,bindings,cache);
                auto at = substitute_type(entities[ae].type,bindings,cache);
                if (!pt || !at || pt != at) return false;
            }
            if (entities[ae].parameter_pack && !pack) return false;
            auto canonical = canonical_argument(pe,i,bindings,cache,depth);
            bindings.put(pe,canonical); bindings.put(ae,canonical); ++j;
        } while (pack && j < a.count);
    }
    // Course fixtures also accept an omitted trailing defaulted parameter.
    for (; j < a.count; ++j) {
        auto ae = template_parameters[a.offset+j];
        if (!entities[ae].initializer && !template_default_types.get(ae) && !entities[ae].parameter_pack) return false;
    }
    return true;
}
TypeId Analyzer::apply_type_template(EntityId e, const std::vector<ArgumentId>& args)
{
    return entities[e].kind == EntityKind::Alias ? specialize_alias(e,args) : entities[specialize_class(e,args)].type;
}
TypeId Analyzer::specialize_alias(EntityId e, const std::vector<ArgumentId>& input)
{
    SubstitutionDependency dependency(incomplete_substitution);
    auto args = input;
    if (!template_defaults(e,args)) {
        if (template_type_probe) return 0;
        throw std::runtime_error("invalid alias template arguments");
    }
    auto k = key(e,intern_arguments(args));
    auto id = alias_specializations.get(k);
    if (!id) {
        id = alias_facts.size(); alias_facts.push_back(TemplateAliasFact());
        alias_specializations.put(k,id);
    }
    auto state = alias_facts[id].state;
    if (state == FactState::Success) { dependency.succeeded = true; return alias_facts[id].type; }
    if (state == FactState::Failure) {
        auto blocked = substitution_prerequisites[incomplete_aliases.get(id)];
        if (!blocked.query || blocked.revision == query_revisions.get(blocked.query)) {
            if (blocked.query) { incomplete_substitution = blocked.query; record_query_dependency(blocked.query); }
            if (template_type_probe) return 0;
            throw std::runtime_error("failed alias specialization");
        }
        substitution_prerequisites[incomplete_aliases.get(id)] = QueryPrerequisite();
    }
    if (state == FactState::Active) throw std::runtime_error("recursive alias specialization");
    alias_facts[id].state = FactState::Active;
    try {
    auto head = templates[entities[e].template_info];
    Index bindings, cache;
    auto parent = head.parent_frame ? head.parent_frame : template_lexical_frame(scopes[head.environment].parent);
    auto frame = substitution_frame(0,head.offset,head.count,parent,intern_arguments(args));
    auto type = substitute_type(entities[e].type,bindings,cache,frame);
    if (!type) {
        alias_facts[id].state = FactState::Failure;
        if (incomplete_substitution) retain_query_prerequisite(incomplete_aliases,id);
        if (template_type_probe) return 0;
        throw std::runtime_error("invalid alias substitution");
    }
    if (!substituted_type_access(entities[e].source,frame)) {
        alias_facts[id].state = FactState::Failure;
        if (template_type_probe) return 0;
        throw std::runtime_error("invalid alias type access");
    }
    // Alias transparency does not erase the obligation to form every argument
    // at substitution, even when the resulting type does not mention it.
    bool dependent = false;
    for (auto arg : args) dependent |= dependent_argument(arg);
    if (dependent) type = types.alias_application(e,type,intern_arguments(args));
    alias_facts[id].type = type; alias_facts[id].state = FactState::Success;
    dependency.succeeded = true;
    return type;
    } catch (...) { alias_facts[id].state = FactState::Failure; throw; }
}
} }
