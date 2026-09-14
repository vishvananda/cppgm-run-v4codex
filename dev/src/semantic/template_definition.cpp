#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
// Application state belongs to specialization/source. A selected-list result
// belongs to member/source-head and can also record a completed absent result.
enum class DefinitionState : unsigned char { NotStarted, Active, Applied, Failed, CompleteTail, AbsentTail };
}
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
    if (!cls || !entities[cls].class_info || entities[cls].explicit_specialization) return TemplateDefinitionOwner();
    if (auto id = definition_owner_index.get(cls)) return definition_owners[id];
    TemplateDefinitionOwner result;
    if (entities[cls].specialization) {
        auto spec = specializations[entities[cls].specialization];
        result.specialization = cls; result.path = definition_root(spec.definition_pattern ? spec.definition_pattern : spec.pattern);
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
            primary = template_definition_pattern(e,p,s); primary_part = p;
            path = definition_root(primary); previous = ast[p].text;
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
    std::uint64_t definition_bucket = 0;
    std::uint32_t retained = 0; IdentifierId definition_name = 0;
    do {
        def.declarator = d; def.initializer = item ? ast[d].next : 0;
        auto member = terminal(name);
        if (ast[ast[name].last].op == OP_COMPL) {
            auto text = ids.spelling(ast[ast[ast[name].last].first].text);
            std::string spelling = "~" + std::string(text.data,text.size);
            member = ids.intern(TextView(spelling.data(),spelling.size()));
        }
        auto k = key(path,member);
        definition_bucket = k; definition_name = member;
        if (d && !template_prototype_index.get(k)) throw std::runtime_error("out-of-class member was not declared");
        if (!d && ast[n].kind == Kind::Class) index_template_members(n,definition_path(path,member),s);
        def.next = definition_index.get(k);
        retained = template_definitions.size();
        definition_index.put(k,retained); template_definitions.push_back(def);
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
    definition_source_parameters.put(environment,def.parameters+1);
    for (unsigned j = 0; j < def.count; ++j) {
        auto parameter = template_parameters[def.parameters+j];
        bind(environment,entities[parameter].name,parameter);
    }
    std::uint32_t prototype = 0;
    if (ast[n].kind == Kind::Class) {
        auto nested = local(binding_owner,terminal(name),Lookup::Qualifier);
        bind_template_class(n,environment,nested);
    } else {
        bind_template_declaration(n,environment,0,false);
        if (d) prototype = check_template_member_definition(d,path,definition_name,s,primary);
    }
    if (prototype) {
        auto special = child(def.initializer ? def.initializer : child(n,Kind::Initializer),Kind::SpecialInitializer);
        if (ast[n].kind != Kind::Function && ast[n].kind != Kind::SpecialDefinition && !special)
            throw std::runtime_error("out-of-class member must be a definition");
        if (special && ast[special].op == KW_DELETE)
            throw std::runtime_error("deleted definition must be the first declaration");
        if (template_prototypes[prototype].definitions)
            throw std::runtime_error("duplicate out-of-class template member definition");
        template_definitions[retained].selected_next = template_prototypes[prototype].definitions;
        template_prototypes[prototype].definitions = retained;
    } else {
        template_definitions[retained].selected_next = unmatched_definitions.get(definition_bucket);
        unmatched_definitions.put(definition_bucket,retained);
    }
    template_definitions[retained].checked = true;
    return true;
}
bool Analyzer::instantiate_member_definition(EntityId e)
{
    if (!e || entities[e].explicit_specialization || instantiation_suppressed(e) || scopes[entities[e].owner].kind != ScopeKind::Class) return false;
    auto owner = definition_owner(scopes[entities[e].owner].entity);
    if (!owner.specialization || dependent_type(entities[owner.specialization].type)) return false;
    ++definition_requests;
    auto bucket = key(owner.path,entities[e].name);
    auto head = definition_index.get(bucket);
    if (!head) return false;
    // Source checking can re-enter this owner before prototype selection is
    // published. Such a request uses the existing declaration path and cannot
    // cache a final selected-list result until source checking completes.
    auto traversal = key(e,head);
    if (auto known = definition_traversals.get(traversal)) {
        ++definition_hits; return known == unsigned(DefinitionState::CompleteTail);
    }
    auto member = entities[e].member_info;
    auto prototype = member ? members[member].prototype : 0;
    bool source_ready = template_definitions[head].checked;
    bool selected = source_ready && prototype && template_prototypes[prototype].signature;
    auto current = selected ? template_prototypes[prototype].definitions : head;
    auto unmatched = selected ? unmatched_definitions.get(bucket) : 0;
    bool found = current || unmatched, complete = source_ready;
    while (current || unmatched) {
        // Preserve source order within the two immutable lists, without
        // visiting definitions matched to a different member declaration.
        auto id = current > unmatched ? current : unmatched;
        bool matched = selected && id == current;
        auto next = template_definitions[id].selected_next;
        if (id == current) current = selected ? next : template_definitions[id].next;
        else unmatched = next;
        auto k = key(owner.specialization,id);
        auto state = DefinitionState(definition_applications.get(k));
        ++definition_edges;
        if (state == DefinitionState::Failed)
            throw FailedSemanticFact(SemanticFact::MemberDefinition,owner.specialization,template_definitions[id].source);
        if (state != DefinitionState::NotStarted) {
            // Recursive demand can consume established declarations, but must
            // not publish completion for a still-active definition application.
            complete &= state == DefinitionState::Applied; continue;
        }
        definition_applications.put(k,unsigned(DefinitionState::Active));
        ++template_definition_work;
        auto saved_template = active_template_scope, saved_environment = member_definition_environment;
        auto saved_depth = class_depth;
        auto saved_bodies = bodies.size();
        auto saved_defaults = declaration_defaults.size();
        try {
        auto def = template_definitions[id];
        auto selection = specializations[entities[owner.specialization].specialization];
        auto pack = argument_packs[selection.definition_arguments ? selection.definition_arguments : selection.arguments];
        ScopeId environment = make_scope(ScopeKind::Template,entities[e].owner);
        for (unsigned j = 0; j < def.count; ++j) {
            auto p = template_parameters[def.parameters+j];
            bind_argument(environment,p,argument_types[pack.offset+j]);
        }
        auto context = ast.new_context();
        auto source = ast.instantiate(def.source,context);
        auto specialization = entities[owner.specialization].specialization;
        auto head = templates[entities[selection.definition_pattern ? selection.definition_pattern : selection.pattern].template_info];
        auto parent = substitution_frame(specialization,head.offset,head.count);
        // A nested class definition can introduce aliases under another head.
        // Preserve those source parameter identities in parent-linked frames.
        std::vector<std::uint32_t> parents;
        for (auto scope = facts[def.declarator].scope; scope; scope = scopes[scope].parent) {
            auto parameters = definition_source_parameters.get(scope);
            if (parameters && parameters-1 != def.parameters && parameters-1 != head.offset)
                parents.push_back(parameters-1);
        }
        auto source_frame = [&](std::uint32_t parameters, std::uint32_t parent) {
            if (selection.definition_pattern == selection.pattern || !selection.definition_pattern)
                return substitution_frame(specialization,parameters,def.count,parent);
            // A selected tuple plus a retained parameter slice gives O(1)
            // ordinal lookup even for a wide partial-owner head.
            return substitution_frame(specialization,parameters,def.count,parent,selection.definition_arguments);
        };
        for (auto p = parents.rbegin(); p != parents.rend(); ++p) parent = source_frame(*p,parent);
        auto frame = source_frame(def.parameters,parent);
        attach_template_context(context,frame);
        facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
        active_template_scope = 0; member_definition_environment = environment;
            if (matched && entities[e].kind == EntityKind::Function) {
                ++definition_direct_work;
                // The checked source signature selected this concrete member.
                // Apply body/defaulted facts through the existing lifetime and
                // exception owners without reconstructing its declaration.
                auto d = ast.projected(def.declarator,context);
                instantiate_parameters(def.declarator,context,frame,environment);
                auto kind = ast[source].kind;
                bool special_member = kind == Kind::SpecialDefinition || kind == Kind::SpecialMember;
                auto specs = special_member ? 0 : ast[source].first;
                auto init = ast.projected(def.initializer,context);
                if (!init) init = child(source,Kind::Initializer);
                declaration_attributes(e,specs,source);
                function_defaults(e,d,environment,source);
                exception_specification(e,d,environment);
                auto special = child(init,Kind::SpecialInitializer);
                if (special) {
                    members[entities[e].member_info].deleted = ast[special].op == KW_DELETE;
                    classify_transfer(e,special,environment);
                }
                virtual_declaration(e,d,init,specs,source,environment);
                record(entities[e].owner,e,d,entities[e].type,EntityKind::Function);
                auto body = kind == Kind::Function ? ast[d].next : child(source,Kind::Compound);
                if (!body) body = child(source,Kind::FunctionTry);
                if (body) schedule_body({body,d,entities[e].owner,e,source});
            } else if (ast[source].kind == Kind::SimpleDeclaration) {
                auto specs = ast[source].first, d = ast.projected(def.declarator,context);
                auto type = declarator(d,specifiers(specs,environment),environment);
                declare_object(d,ast.projected(def.initializer,context),type,specs,environment,source);
            } else declaration(source,environment);
            definition_applications.put(k,unsigned(DefinitionState::Applied));
        } catch (...) {
            definition_applications.put(k,unsigned(DefinitionState::Failed));
            active_template_scope = saved_template; member_definition_environment = saved_environment;
            class_depth = saved_depth; bodies.resize(saved_bodies); declaration_defaults.resize(saved_defaults);
            if (entities[e].class_info) entities[e].complete = false;
            throw;
        }
        active_template_scope = saved_template; member_definition_environment = saved_environment;
    }
    if (complete) definition_traversals.put(traversal,unsigned(found ? DefinitionState::CompleteTail : DefinitionState::AbsentTail));
    return found;
}
void Analyzer::demand_template_storage(EntityId e)
{
    if (e && entities[e].kind == EntityKind::Variable && entities[e].is_static && scopes[entities[e].owner].kind == ScopeKind::Class)
        record_default_dependency(DefaultDependencyKind::Storage,e);
    if (unevaluated_depth) return;
    if (!e || entities[e].kind != EntityKind::Variable || !entities[e].is_static ||
        scopes[entities[e].owner].kind != ScopeKind::Class || storage_requested.get(e)) return;
    if (!definition_owner(scopes[entities[e].owner].entity).specialization) return;
    storage_requested.put(e,1); storage_demand.push_back(e);
    instantiate_member_definition(e);
    // A shared constant initializer can be evaluated while its class is still
    // unevaluated. Actual storage demand owns the required relocation targets.
    demand_constant_relocations(constant_entity_value(e));
}
void Analyzer::demand_class_constant_storage(TypeId type)
{
    while (types[type].kind == TypeKind::Array) type = types[type].child;
    if (!class_value(type)) return;
    auto cls = types[type].entity;
    if (!definition_owner(cls).specialization || class_constant_storage.get(cls)) return;
    complete_class(cls);
    if (!entities[cls].complete) return;
    class_constant_storage.put(cls,1);
    // PA15's concrete namespace object output retains visible definitions of
    // this class's static constants. Storage is a separate demand from layout
    // and from dormant member functions. Visit only this owner's declarations
    // once, and use the definition index for each qualifying member.
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        auto e = declarations[d].entity;
        if (entities[e].owner == entities[cls].scope && entities[e].kind == EntityKind::Variable &&
            entities[e].is_static && entities[e].constant.valid)
            demand_template_storage(e);
    }
}
} }
