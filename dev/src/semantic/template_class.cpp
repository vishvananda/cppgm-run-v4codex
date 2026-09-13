#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::dependent_template_syntax(NodeId root, ScopeId s)
{
    std::vector<NodeId> work(1,root);
    Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        NodeId n = work[i];
        if (!n || seen.get(n)) continue;
        seen.put(n,1);
        auto node = ast[n];
        if (node.kind == Kind::Member) {
            // The name after . or -> belongs to the object's class. It is
            // never an unqualified reference to a surrounding type parameter.
            work.push_back(node.first);
            auto name = ast[ast[node.first].next].detail;
            for (auto part = ast[name].first; part; part = ast[part].next)
                if (auto args = child(part,Kind::TemplateArguments)) work.push_back(args);
            continue;
        }
        if (node.kind == Kind::Name && node.first) {
            auto e = lookup(node.op == OP_COLON2 ? global : s,ast[node.first].text);
            if (e && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias) && dependent_type(entities[e].type)) return true;
        }
        if (node.detail) work.push_back(node.detail);
        for (auto child = node.first; child; child = ast[child].next) work.push_back(child);
    }
    return false;
}
TypeId Analyzer::declare_class_template(NodeId n, ScopeId s)
{
    NodeId name = ast[n].detail;
    ScopeId owner = name_owner(name,s);
    if (owner == s) owner = scopes[s].parent;
    if (!encloses(scopes[s].parent,owner)) throw std::runtime_error("class template outside enclosing scope");
    auto id = terminal(name);
    EntityId e = local(owner,id,Lookup::Tag);
    if (e && !entities[e].template_info) throw std::runtime_error("conflicting class template declaration");
    if (!e) {
        e = make_entity(EntityKind::Type,owner,id,n);
        entities[e].key = ast[ast[n].first].op;
        entities[e].type = types.named(e);
        entities[e].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
        entities[e].scope = make_scope(ScopeKind::Class,s,id,e,false);
        bind(owner,id,e);
    }
    auto previous_index = entities[e].template_info;
    auto previous = previous_index ? templates[previous_index] : TemplateFunction();
    if (ast[n].kind == Kind::Class && previous.body) throw std::runtime_error("class template redefinition");
    template_facts(e,s);
    auto index = entities[e].template_info;
    if (previous.environment && previous.count != templates[index].count) throw std::runtime_error("different template parameter count");
    auto current = templates[index];
    // A later declaration adds defaults to the canonical head without replacing
    // the definition's parameter environment or reinterpreting its source names.
    if (previous.body) { index = previous_index; entities[e].template_info = index; }
    auto selected = templates[index];
    Index old_bindings, current_bindings, old_cache, current_cache;
    bool old_ready = false, current_ready = false;
    auto translate = [&](TypeId value, const TemplateFunction& head, Index& bindings, Index& cache, bool& ready) {
        if (head.offset == selected.offset) return value;
        if (!ready) {
            for (unsigned j = 0; j < head.count; ++j)
                bindings.put(template_parameters[head.offset+j],entities[template_parameters[selected.offset+j]].type);
            ready = true;
        }
        return substitute_type(value,bindings,cache);
    };
    for (unsigned j = 0; j < current.count; ++j) {
        auto parameter = template_parameters[current.offset+j];
        auto init = entities[parameter].initializer;
        auto old = previous.environment ? template_default_types.get(template_parameters[previous.offset+j]) : 0;
        if (old && init) throw std::runtime_error("duplicate template default argument");
        TypeId value = init ? translate(type_id(ast[init].first,s),current,current_bindings,current_cache,current_ready) :
            old ? translate(old,previous,old_bindings,old_cache,old_ready) : 0;
        if (value) template_default_types.put(template_parameters[selected.offset+j],value);
    }
    if (!previous.body) {
        templates[index].source = n;
        templates[index].body = ast[n].kind == Kind::Class ? n : previous.body;
    }
    if (ast[n].kind == Kind::Class) index_template_members(n,definition_root(e),s);
    bind(s,id,e); record(s,e,n,entities[e].type,EntityKind::Type);
    // Fixed bases are definition-time demands, independent of whether a
    // specialization of this derived template will ever be requested.
    auto bases_node = child(n,Kind::Bases);
    for (auto b = ast[bases_node].first; b; b = ast[b].next) {
        if (dependent_template_syntax(ast[child(b,Kind::BaseName)].detail,s)) continue;
        auto base = resolve(ast[child(b,Kind::BaseName)].detail,s,Lookup::Qualifier);
        if (base && entities[base].kind == EntityKind::Alias) base = types[entities[base].type].entity;
        if (base && dependent_type(entities[base].type)) continue;
        if (base && entities[base].class_info) complete_class(base);
        if (!base || !entities[base].class_info || !entities[base].complete || entities[base].key == KW_UNION ||
            class_facts[entities[base].class_info].final_class)
            throw std::runtime_error("invalid nondependent template base");
    }
    if (ast[n].kind == Kind::Class) bind_template_class(n,s,e);
    return entities[e].type;
}
bool Analyzer::template_defaults(EntityId pattern, std::vector<TypeId>& args)
{
    auto t = templates[entities[pattern].template_info];
    if (args.size() > t.count) return false;
    Index bindings, cache;
    for (unsigned j = 0; j < t.count; ++j) {
        auto p = template_parameters[t.offset+j];
        if (j == args.size()) {
            TypeId value = template_default_types.get(p);
            if (!value) {
                NodeId d = entities[p].initializer;
                if (!d) return false;
                value = type_id(ast[d].first,t.environment);
            }
            value = substitute_type(value,bindings,cache);
            if (!value) return false;
            args.push_back(value);
        }
        bindings.put(p,args[j]);
    }
    return true;
}
EntityId Analyzer::specialize_class(EntityId pattern, const std::vector<TypeId>& arguments)
{
    auto args = arguments;
    if (!template_defaults(pattern,args)) throw std::runtime_error("invalid class template arguments");
    auto pack = intern_arguments(args);
    if (auto old = specialization_index.get(key(pattern,pack))) return specializations[old].entity;
    auto source = entities[pattern];
    EntityId e = make_entity(EntityKind::Type,source.owner,source.name,source.source);
    entities[e].key = source.key; entities[e].type = types.named(e);
    entities[e].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
    Specialization spec; spec.pattern = pattern; spec.entity = e; spec.arguments = pack;
    spec.declaration = FactState::Success;
    auto index = specializations.size(); specializations.push_back(spec);
    specialization_index.put(key(pattern,pack),index); entities[e].specialization = index;
    ScopeId environment = specialization_environment(e);
    entities[e].scope = make_scope(ScopeKind::Class,environment,source.name,e,false);
    bind(environment,source.name,e); bind(entities[e].scope,source.name,e);
    return e;
}
ScopeId Analyzer::specialization_environment(EntityId e)
{
    auto index = entities[e].specialization;
    if (specializations[index].environment) return specializations[index].environment;
    auto spec = specializations[index];
    auto pattern = templates[entities[spec.pattern].template_info];
    auto environment = make_scope(ScopeKind::Template,entities[spec.pattern].owner);
    auto pack = argument_packs[spec.arguments];
    for (unsigned j = 0; j < pack.count; ++j) {
        auto parameter = template_parameters[pattern.offset+j];
        auto alias = make_entity(EntityKind::Alias,environment,entities[parameter].name,0);
        entities[alias].type = argument_types[pack.offset+j];
        bind(environment,entities[alias].name,alias);
    }
    specializations[index].environment = environment;
    return environment;
}
EntityId Analyzer::class_template_name(NodeId part, EntityId e, ScopeId s)
{
    if (!e || !entities[e].class_info) return e;
    auto list = child(part,Kind::TemplateArguments);
    if (!list) return e;
    EntityId pattern = entities[e].template_info ? e : entities[e].specialization ? specialization_pattern(e) : 0;
    if (!pattern) throw std::runtime_error("template-id names a nontemplate class");
    std::vector<TypeId> args;
    for (NodeId a = ast[list].first; a; a = ast[a].next) {
        if (ast[a].kind != Kind::TypeId) throw std::runtime_error("type template argument required");
        auto type = type_id(a,s);
        if (template_type_probe && !type) return 0;
        args.push_back(type);
    }
    return specialize_class(pattern,args);
}
void Analyzer::complete_class(EntityId e)
{
    if (!e || entities[e].complete) return;
    if (!entities[e].specialization) { instantiate_member_definition(e); return; }
    auto index = entities[e].specialization;
    if (specializations[index].body == FactState::Active || dependent_type(entities[e].type)) return;
    auto pattern = templates[entities[specializations[index].pattern].template_info];
    if (!pattern.body) return;
    if (specializations[index].body == FactState::Failure) throw std::runtime_error("failed class specialization");
    specializations[index].body = FactState::Active; ++template_completions;
    auto environment = specialization_environment(e);
    // An earlier forward declaration may have used different parameter names.
    auto pack = argument_packs[specializations[index].arguments];
    for (unsigned j = 0; j < pack.count; ++j) {
        auto parameter = template_parameters[pattern.offset+j];
        if (local(environment,entities[parameter].name)) continue;
        auto alias = make_entity(EntityKind::Alias,environment,entities[parameter].name,0);
        entities[alias].type = argument_types[pack.offset+j]; bind(environment,entities[alias].name,alias);
    }
    auto context = ast.new_context();
    auto source = ast.instantiate(pattern.body,context);
    specializations[index].context = context;
    attach_template_context(context,substitution_frame(index,pattern.offset,pattern.count));
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
    ScopeId saved = active_template_scope; active_template_scope = 0;
    facts[source].entity = e;
    class_type(source,environment,0,true);
    active_template_scope = saved;
    specializations[index].body = FactState::Success;
}
} }
