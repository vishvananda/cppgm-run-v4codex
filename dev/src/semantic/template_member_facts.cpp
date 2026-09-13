#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::template_method_shape(NodeId parameters, ScopeId scope)
{
    if (auto cached = template_method_shapes.get(parameters)) return cached-1;
    std::vector<TypeId> params;
    auto parameter_scope = prototype_scope_needed(parameters) ? make_scope(ScopeKind::Block,scope,0,0,false) : scope;
    if (parameter_scope != scope) template_pattern_scopes.put(parameter_scope,1);
    bool known = true, variadic = false;
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (ast[p].kind == Kind::ParameterPack) { variadic = true; continue; }
        auto specs = ast[p].first, d = ast[specs].next;
        // Dependent signatures and bounds belong to substitution. A missing
        // shape is conservative evidence, never a guessed overload identity.
        if (bind_template_expression(specs,parameter_scope) | bind_template_expression(d,parameter_scope)) { known = false; break; }
        if (child(d,Kind::Array)) { known = false; break; }
        auto type = parameter(p,parameter_scope); params.push_back(type);
        auto name = terminal(decl_name(d));
        if (name && parameter_scope != scope) {
            auto e = pattern_declaration(EntityKind::Parameter,parameter_scope,name,p,false);
            entities[e].type = parameter_body_type(type); signature_parameters.put(e,params.size());
        }
    }
    if (params.size() == 1 && fundamental(params[0],FT_VOID) && !variadic) params.clear();
    TypeId shape = known ? types.signature(types.function(types.fundamental(FT_VOID),params,variadic)) : 0;
    template_method_shapes.put(parameters,shape+1);
    return shape;
}
TemplateObjectContext Analyzer::template_object_context(ScopeId s) const
{
    while (s && scopes[s].kind != ScopeKind::Function) s = scopes[s].parent;
    return template_object_contexts[template_object_context_index.get(s)];
}
void Analyzer::bind_template_object_context(ScopeId function, NodeId parameters)
{
    auto e = scopes[function].entity;
    auto owner = entities[e].owner;
    bool outside = scopes[owner].kind == ScopeKind::Template && scopes[scopes[owner].parent].kind == ScopeKind::Class;
    if (!outside && scopes[owner].kind != ScopeKind::Class) return;
    auto qualifiers = function_qualifiers(parameters);
    bool available = !entities[e].is_static;
    if (outside) {
        owner = scopes[owner].parent;
        if (entities[e].name) {
            auto parameters_of = [&](NodeId d) {
                NodeId parameters = 0;
                while (d) {
                    if (auto p = child(d,Kind::Parameters)) parameters = p;
                    auto nested = child(d,Kind::NestedDeclarator);
                    d = nested ? ast[nested].first : 0;
                }
                return parameters;
            };
            std::vector<EntityId> possible;
            unsigned kinds = 0;
            for (auto candidate : candidates(local(owner,entities[e].name))) {
                if (entities[candidate].owner != owner) continue;
                auto p = parameters_of(entities[candidate].source);
                auto q = function_qualifiers(p);
                if (!p || q.cv != qualifiers.cv || q.ref != qualifiers.ref) continue;
                possible.push_back(candidate); kinds |= entities[candidate].is_static ? 1 : 2;
            }
            if (kinds == 3) {
                auto shape = template_method_shape(parameters,entities[e].owner);
                if (shape) {
                    kinds = 0;
                    for (auto candidate : possible) {
                        auto other = template_method_shape(parameters_of(entities[candidate].source),owner);
                        if (!other || shape == other) kinds |= entities[candidate].is_static ? 1 : 2;
                    }
                }
            }
            // All possible declarations must agree. Static is not repeated in
            // the definition, and unresolved mixed signatures stay deferred.
            if (kinds != 1 && kinds != 2) return;
            available = kinds == 2;
        }
    }
    TemplateObjectContext context; context.owner = scopes[owner].entity;
    context.cv = qualifiers.cv; context.available = available;
    template_object_context_index.put(function,template_object_contexts.size());
    template_object_contexts.push_back(context);
}
bool Analyzer::check_template_field(NodeId n, ScopeId s, EntityId field, bool explicit_object)
{
    if (!field || ast.nodes.occurrences[n].context || !nonstatic_field(field) ||
        !entities[field].type || dependent_type(entities[field].type)) return false;
    // Unknown width can change integral promotion after substitution.
    if (field_fact(field).bit_field && !field_fact(field).width) return false;
    auto context = template_object_context(s);
    if (!context.owner) return false;
    auto owner = scopes[entities[field].owner].entity;
    if (context.owner != owner && !class_derives(context.owner,owner)) {
        if (!unevaluated_depth) throw std::runtime_error("member does not belong to the template object");
        return false;
    }
    if (!context.available && (!unevaluated_depth || explicit_object)) throw std::runtime_error("template member requires an object");
    check_access(field,s,entities[field].owner);
    auto result = member_value(field,context.available ? context.cv : 0,ValueCategory::Lvalue);
    result.ready = true;
    expressions.set(n,result); { auto& published = facts.edit(n); published.type = result.type; published.entity = field; published.scope = s; }
    while (scopes[s].kind != ScopeKind::Function) s = scopes[s].parent;
    auto source = ast.nodes.occurrences[n].source;
    template_field_sources.put(source,template_object_context_index.get(s));
    template_fixed_expressions.put(source,n); ++template_fixed_work;
    return true;
}
TemplateMemberUse Analyzer::template_field_use(EntityId field, ScopeId s, EntityId pattern)
{
    auto object_type = implicit_object_type(s);
    auto scope = s;
    while (scope && scopes[scope].kind != ScopeKind::Function) scope = scopes[scope].parent;
    auto function = scopes[scope].entity;
    auto cls = scopes[entities[function].owner].entity;
    if (!pattern && entities[field].template_pattern) pattern = scopes[entities[field].owner].entity;
    if (!cls || (pattern && template_class_patterns.get(cls) != pattern))
        throw std::logic_error("missing concrete template object owner");
    if (!object_type && !unevaluated_depth) throw std::logic_error("template field use lacks its object");
    auto key = this->key(field,object_type ? object_type : entities[cls].type);
    auto id = template_member_use_index.get(key);
    if (!id) {
        TemplateMemberUse use; use.entity = field;
        if (entities[use.entity].template_pattern) {
            auto declaration = ast.projected(entities[use.entity].source,template_class_contexts.get(cls));
            use.entity = facts[declaration].entity;
            if (!declaration || !use.entity || entities[use.entity].template_pattern)
                throw std::logic_error("missing concrete template field declaration");
        }
        if (object_type) {
            Expression receiver;
            record_object(receiver,0,object_type,base_steps(types[object_type].child,scopes[entities[use.entity].owner].entity));
            use.object = receiver.object_use;
        }
        auto value = member_value(use.entity,object_type ? types[types[object_type].child].cv : 0,ValueCategory::Lvalue);
        use.type = value.type;
        id = template_member_uses.size(); template_member_uses.push_back(use);
        template_member_use_index.put(key,id);
    }
    return template_member_uses[id];
}
bool Analyzer::reuse_template_field(NodeId n, ScopeId s, Expression& result)
{
    auto path = template_field_sources.get(ast.nodes.occurrences[n].source);
    if (!path) return false;
    auto use = template_field_use(result.entity,s,template_object_contexts[path].owner);
    if (use.type != result.type || result.category != ValueCategory::Lvalue)
        throw std::logic_error("fixed template field value facts changed");
    result.entity = use.entity; result.object_use = use.object;
    if (ast[n].kind == Kind::Member) expression(ast[n].first,s);
    return true;
}
} }
