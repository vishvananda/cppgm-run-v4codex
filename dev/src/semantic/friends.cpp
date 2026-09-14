#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::friend_declaration(NodeId n, ScopeId s)
{
    NodeId specs = ast[n].first;
    if (!spec_has(specs, KW_FRIEND)) return false;
    ScopeId ns = s;
    while (scopes[ns].kind != ScopeKind::Namespace) ns = scopes[ns].parent;
    bool templated = s == active_template_scope;
    EntityId cls = scopes[templated ? scopes[s].parent : s].entity;
    NodeId friend_type = child(specs, Kind::ClassForward);
    if (friend_type) {
        NodeId name = ast[friend_type].detail;
        if (templated) {
            bool qualified = ast[name].first != ast[name].last || ast[name].op == OP_COLON2;
            auto owner = qualified ? name_owner(name,s) : ns;
            if (child(ast[name].last,Kind::TemplateArguments))
                throw std::runtime_error("friend declaration cannot declare a partial specialization");
            if (qualified && !local(owner,terminal(name),Lookup::Tag))
                throw std::runtime_error("qualified friend class template was not declared");
            auto type = declare_class_template(friend_type,s,owner);
            auto target = types[type].entity;
            friendships.put(key(cls,target),1);
            record(s,target,n,type,EntityKind::Type);
            return true;
        }
        if (pattern_scope(s) && dependent_template_syntax(name,s)) {
            bind_template_name(name,s); return true;
        }
        EntityId target = resolve(name, s, Lookup::Tag);
        if (!target) {
            if (ast[name].first != ast[name].last) throw std::runtime_error("undeclared qualified friend class");
            TypeId type = class_type(friend_type, ns);
            target = types[type].entity;
        }
        if (!entities[target].class_info) throw std::runtime_error("friend is not a class");
        friendships.put(key(cls, target), 1); return true;
    }
    TypeId base = specifiers(specs, s);
    NodeId list = child(n, Kind::InitDeclarators);
    if (ast[n].kind != Kind::Function && !ast[list].first) {
        // [class.friend]: a simple-type-specifier or typename-specifier grants
        // friendship to the resulting class; other types are ignored.
        if (!dependent_type(base) && types[base].kind == TypeKind::Named && entities[types[base].entity].class_info)
            friendships.put(key(cls,types[base].entity),1);
        return true;
    }
    auto add = [&](NodeId d, NodeId body) {
        bool source_pattern = definitions && !ast.nodes.occurrences[d].context && pattern_scope(s);
        TypeId type = types.signature(source_pattern ? bind_template_type(specs,d,s) : declarator(d, base, s));
        if (types[type].kind != TypeKind::Function) throw std::runtime_error("friend declaration is not a function");
        NodeId name = decl_name(d);
        bool qualified = ast[name].first != ast[name].last || ast[name].op == OP_COLON2;
        ScopeId owner = qualified ? name_owner(name, s) : ns;
        EntityId function = 0;
        bool fixed_reference = (qualified || child(ast[name].last,Kind::TemplateArguments)) &&
            !dependent_type(type) && !dependent_template_syntax(name,s);
        if (source_pattern && !templated && !fixed_reference) {
            // A non-template friend of a class template is a source pattern
            // for an ordinary namespace function. It is not a function
            // template over the enclosing class's parameters.
            function = make_entity(EntityKind::Function,owner,terminal(name),n);
            entities[function].type = type; entities[function].template_pattern = true;
            template_pattern_entities.put(function,2);
            template_declaration_sources.put(ast.nodes.occurrences[d].source,function);
            declaration_attributes(function,specs,n); declare_operator(function,name);
            friendships.put(key(cls,function),1);
            if (!qualified) hidden_friends.put(key(cls,terminal(name)),merge_lookup(hidden_friends.get(key(cls,terminal(name))),function));
            record(s,function,d,type,EntityKind::Function);
            if (body) {
                Body retained{body,d,s,function,n};
                if (template_source_deferred) template_source_deferred->push_back(retained);
                else bind_template_body(retained);
            }
            return;
        }
        if (!templated && child(ast[name].last,Kind::TemplateArguments)) {
            function = declare_function_specialization(name,s,type,owner);
        } else if (qualified) {
            if (templated) {
                auto family = template_families.get(key(owner,terminal(name)));
                auto shape = template_declaration_shape(type,s);
                if (family) function = template_signatures.get(key(family,shape));
            } else {
                EntityId found = lookup(owner, terminal(name), Lookup::Ordinary, true);
                if (function_binding(found)) for (EntityId candidate : candidates(found))
                    if (!entities[candidate].template_info && entities[candidate].type == type) function = candidate;
                if (!function) function = declare_function_specialization(name,s,type,owner);
            }
            if (!function) throw std::runtime_error("qualified friend must match a declared function");
        } else function = declare_function(owner, terminal(name), n, type, true);
        declaration_attributes(function,specs,n);
        declare_operator(function, name);
        friendships.put(key(cls, function), 1);
        if (!qualified) hidden_friends.put(key(cls, terminal(name)), merge_lookup(hidden_friends.get(key(cls, terminal(name))), function));
        function_defaults(function, d, s, n); exception_specification(function, d, s);
        record(s, function, d, type, EntityKind::Function);
        if (body) {
            if (!qualified) entities[function].emission |= Entity::HiddenFriend;
            entities[function].inline_function = true;
            Body retained{body,d,s,function,n};
            if (!templated && definitions && ast.nodes.occurrences[n].context) {
                if (friend_definition_index.get(function) || entities[function].definition)
                    throw std::runtime_error("friend function redefinition");
                friend_definitions.push_back(retained);
                friend_definition_index.put(function,friend_definitions.size());
                if (entities[function].emission & Entity::Used) demand_friend_body(function);
            } else schedule_body(retained);
        }
    };
    if (ast[n].kind == Kind::Function) { NodeId d = ast[specs].next; add(d, ast[d].next); }
    else for (NodeId item = ast[list].first; item; item = ast[item].next) add(ast[item].first, 0);
    return true;
}
void Analyzer::demand_friend_body(EntityId e)
{
    if (!friend_definition_index.get(e) || friend_definition_queued.get(e)) return;
    friend_definition_queued.put(e,1); friend_definition_demand.push_back(e);
}
void Analyzer::instantiate_friend_body(EntityId e)
{
    auto id = friend_definition_index.get(e);
    if (!id || entities[e].body_state == FactState::Success || entities[e].body_state == FactState::Active) return;
    auto body = friend_definitions[id-1]; function_body(body);
}
EntityId Analyzer::associated_lookup(IdentifierId name, const std::vector<NodeId>& args)
{
    std::vector<TypeId> work;
    for (NodeId n : args) if (expressions[n].type) work.push_back(expressions[n].type);
    return associated_type_lookup(name,std::move(work));
}
EntityId Analyzer::associated_type_lookup(IdentifierId name, std::vector<TypeId> work)
{
    Index seen_types, seen_scopes;
    std::vector<ScopeId> spaces;
    EntityId result = 0;
    for (std::size_t i = 0; i < work.size(); ++i) {
        TypeId id = work[i];
        if (seen_types.get(id)) continue;
        seen_types.put(id, 1); auto type = types[id];
        if (type.kind == TypeKind::Pointer || type.kind == TypeKind::LRef || type.kind == TypeKind::RRef || type.kind == TypeKind::Array) work.push_back(type.child);
        else if (type.kind == TypeKind::ArgumentPack) {
            auto pack = pack_arguments(id);
            for (unsigned j = 0; j < pack.count; ++j)
                if (!value_argument(argument_types[pack.offset+j])) work.push_back(argument_types[pack.offset+j]);
        } else if (type.kind == TypeKind::Function) {
            work.push_back(type.child);
            for (unsigned j = 0; j < type.count; ++j) work.push_back(types.parameters[type.offset+j]);
        } else if (type.kind == TypeKind::Named) {
            EntityId cls = type.entity;
            if (entities[cls].class_info) {
                if (definitions) complete_class(cls);
                if (entities[cls].specialization) {
                    auto pack = specialization_arguments(cls);
                    for (unsigned j = 0; j < pack.count; ++j) if (!value_argument(argument_types[pack.offset+j])) work.push_back(argument_types[pack.offset+j]);
                }
                result = merge_lookup(result, hidden_friends.get(key(cls, name)));
                for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) work.push_back(entities[bases[b].base].type);
            }
            ScopeId owner = entities[cls].owner;
            if (scopes[owner].kind == ScopeKind::Class) work.push_back(entities[scopes[owner].entity].type);
            while (owner && scopes[owner].kind != ScopeKind::Namespace) owner = scopes[owner].parent;
            if (owner) spaces.push_back(owner);
        }
    }
    for (std::size_t i = 0; i < spaces.size(); ++i) {
        ScopeId ns = spaces[i];
        if (seen_scopes.get(ns)) continue;
        seen_scopes.put(ns, 1);
        EntityId found = local(ns, name);
        if (function_binding(found)) result = merge_lookup(result, found);
        for (auto edge = scopes[ns].first_inline; edge; edge = edges[edge].inline_next) spaces.push_back(edges[edge].target);
        ScopeId parent = scopes[ns].parent;
        auto edge = edge_index.get(key(parent, ns));
        if (edge && edges[edge].inline_namespace) spaces.push_back(parent);
    }
    return result;
}
} }
