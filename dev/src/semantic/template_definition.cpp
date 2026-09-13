#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
std::uint32_t Analyzer::definition_root(EntityId pattern)
{
    auto id = definition_roots.get(pattern);
    if (!id) { id = ++definition_path_count; definition_roots.put(pattern,id); }
    return id;
}
std::uint32_t Analyzer::definition_path(std::uint32_t parent, IdentifierId name)
{
    auto k = key(parent,name), id = std::uint64_t(definition_paths.get(k));
    if (!id) { id = ++definition_path_count; definition_paths.put(k,id); }
    return id;
}
TemplateDefinitionOwner Analyzer::definition_owner(EntityId cls)
{
    if (!cls || !entities[cls].class_info) return TemplateDefinitionOwner();
    if (auto id = definition_owner_index.get(cls)) return definition_owners[id];
    TemplateDefinitionOwner result;
    if (entities[cls].specialization) {
        result.specialization = cls; result.path = definition_root(specialization_pattern(cls));
    } else if (scopes[entities[cls].owner].kind == ScopeKind::Class) {
        result = definition_owner(scopes[entities[cls].owner].entity);
        if (result.specialization) result.path = definition_path(result.path,entities[cls].name);
    }
    if (!result.specialization) { definition_owner_index.put(cls,1); return result; }
    definition_owner_index.put(cls,definition_owners.size()); definition_owners.push_back(result);
    return result;
}
bool Analyzer::retain_template_definition(NodeId n, ScopeId s)
{
    NodeId d = child(n,Kind::Declarator), item = 0;
    if (ast[n].kind == Kind::Function) d = ast[ast[n].first].next;
    if (ast[n].kind == Kind::SimpleDeclaration) { item = ast[child(n,Kind::InitDeclarators)].first; d = ast[item].first; }
    if (item && ast[item].next) throw std::runtime_error("template declaration has multiple declarators");
    NodeId name = d ? decl_name(d) : ast[n].detail;
    if (!name || ast[name].first == ast[name].last) return false;
    ScopeId owner = ast[name].op == OP_COLON2 ? global : scopes[s].parent;
    bool qualified = ast[name].op == OP_COLON2;
    EntityId primary = 0;
    NodeId primary_part = 0;
    std::uint32_t path = 0;
    IdentifierId previous = 0;
    for (auto p = ast[name].first; p && p != ast[name].last; p = ast[p].next) {
        if (primary) {
            if (ast[p].text != previous) path = definition_path(path,ast[p].text);
            previous = ast[p].text; continue;
        }
        auto e = lookup(owner,ast[p].text,Lookup::Qualifier,qualified);
        if (e && entities[e].class_info && entities[e].template_info && child(p,Kind::TemplateArguments)) {
            primary = e; primary_part = p; path = definition_root(e); previous = ast[p].text;
        } else {
            owner = target(e); qualified = true;
            if (!owner) return false;
        }
    }
    if (!primary) return false;
    if (!encloses(scopes[s].parent,entities[primary].owner)) throw std::runtime_error("template member outside enclosing namespace");
    TemplateDefinition def; def.source = n; def.parameters = template_parameters.size();
    for (auto p = scopes[s].first_decl; p; p = declarations[p].next) {
        auto e = declarations[p].entity;
        if (entities[e].template_parameter) {
            parameter_ordinals.put(e,def.count+1); template_parameters.push_back(e); ++def.count;
        }
    }
    if (def.count != templates[entities[primary].template_info].count) throw std::runtime_error("member template head does not match owner");
    unsigned argument = 0;
    for (auto a = ast[child(primary_part,Kind::TemplateArguments)].first; a; a = ast[a].next) {
        if (argument >= def.count || ast[a].kind != Kind::TypeId ||
            types.signature(type_id(a,s)) != entities[template_parameters[def.parameters+argument]].type)
            throw std::runtime_error("member definition does not name its primary template");
        ++argument;
    }
    if (argument != def.count) throw std::runtime_error("incomplete member definition owner arguments");
    do {
        def.declarator = d; def.initializer = item ? ast[d].next : 0;
        auto member = terminal(name);
        if (ast[ast[name].last].op == OP_COMPL) {
            auto text = ids.spelling(ast[ast[ast[name].last].first].text);
            std::string spelling = "~" + std::string(text.data,text.size);
            member = ids.intern(TextView(spelling.data(),spelling.size()));
        }
        auto k = key(path,member);
        if (d && !template_prototype_index.get(k)) throw std::runtime_error("out-of-class member was not declared");
        if (d) check_template_member_exception(d,path,member,s);
        else if (ast[n].kind == Kind::Class) index_template_members(n,definition_path(path,member),s);
        def.next = definition_index.get(k);
        definition_index.put(k,template_definitions.size()); template_definitions.push_back(def);
        item = ast[item].next;
        if (item) { d = ast[item].first; name = decl_name(d); }
    } while (item);
    // Definition-time lookup sees this head's parameters over the owning
    // pattern class. The overlay contains only the declared parameters.
    ScopeId binding_owner = entities[primary].scope;
    IdentifierId previous_name = ast[primary_part].text;
    for (auto p = ast[primary_part].next; p && p != ast[name].last; p = ast[p].next) {
        if (ast[p].text == previous_name) continue;
        auto nested = lookup(binding_owner,ast[p].text,Lookup::Qualifier,true);
        binding_owner = target(nested); previous_name = ast[p].text;
        if (!binding_owner) throw std::runtime_error("unknown nested definition owner");
    }
    auto environment = make_scope(ScopeKind::Template,binding_owner,0,0,false);
    for (unsigned j = 0; j < def.count; ++j) {
        auto parameter = template_parameters[def.parameters+j];
        bind(environment,entities[parameter].name,parameter);
    }
    if (ast[n].kind == Kind::Class) {
        auto nested = local(binding_owner,terminal(name),Lookup::Qualifier);
        bind_template_class(n,environment,nested);
    } else bind_template_declaration(n,environment);
    return true;
}
bool Analyzer::instantiate_member_definition(EntityId e)
{
    if (!e || scopes[entities[e].owner].kind != ScopeKind::Class) return false;
    auto owner = definition_owner(scopes[entities[e].owner].entity);
    if (!owner.specialization || dependent_type(entities[owner.specialization].type)) return false;
    bool found = false;
    for (auto id = definition_index.get(key(owner.path,entities[e].name)); id; id = template_definitions[id].next) {
        auto k = key(owner.specialization,id);
        auto state = definition_applications.get(k);
        if (state == unsigned(FactState::Failure)) throw std::runtime_error("failed template member definition");
        if (state) { found = true; continue; }
        definition_applications.put(k,unsigned(FactState::Active));
        ++template_definition_work;
        auto def = template_definitions[id];
        auto pack = specialization_arguments(owner.specialization);
        ScopeId environment = make_scope(ScopeKind::Template,entities[e].owner);
        for (unsigned j = 0; j < def.count; ++j) {
            auto p = template_parameters[def.parameters+j];
            auto alias = make_entity(EntityKind::Alias,environment,entities[p].name,0);
            entities[alias].type = argument_types[pack.offset+j]; bind(environment,entities[alias].name,alias);
        }
        auto context = ast.new_context();
        auto source = ast.instantiate(def.source,context);
        auto specialization = entities[owner.specialization].specialization;
        auto head = templates[entities[specializations[specialization].pattern].template_info];
        auto parent = substitution_frame(specialization,head.offset,head.count);
        attach_template_context(context,substitution_frame(specialization,def.parameters,def.count,parent));
        facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
        auto saved_template = active_template_scope, saved_environment = member_definition_environment;
        active_template_scope = 0; member_definition_environment = environment;
        try {
            if (ast[source].kind == Kind::SimpleDeclaration) {
                auto specs = ast[source].first, d = ast.projected(def.declarator,context);
                auto type = declarator(d,specifiers(specs,environment),environment);
                declare_object(d,ast.projected(def.initializer,context),type,specs,environment,source);
            } else declaration(source,environment);
            definition_applications.put(k,unsigned(FactState::Success)); found = true;
        } catch (...) {
            definition_applications.put(k,unsigned(FactState::Failure));
            active_template_scope = saved_template; member_definition_environment = saved_environment;
            throw;
        }
        active_template_scope = saved_template; member_definition_environment = saved_environment;
    }
    return found;
}
void Analyzer::demand_template_storage(EntityId e)
{
    if (!e || entities[e].kind != EntityKind::Variable || !entities[e].is_static ||
        scopes[entities[e].owner].kind != ScopeKind::Class || storage_requested.get(e)) return;
    if (!definition_owner(scopes[entities[e].owner].entity).specialization) return;
    storage_requested.put(e,1); storage_demand.push_back(e);
    instantiate_member_definition(e);
}
} }
