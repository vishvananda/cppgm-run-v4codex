#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TemplateObjectContext Analyzer::template_object_context(ScopeId s) const
{
    while (s && scopes[s].kind != ScopeKind::Function) s = scopes[s].parent;
    return template_object_contexts[template_object_context_index.get(s)];
}
void Analyzer::bind_template_object_context(ScopeId function, NodeId parameters)
{
    auto e = scopes[function].entity;
    auto owner = entities[e].owner;
    if (scopes[owner].kind != ScopeKind::Class) return;
    TemplateObjectContext context; context.owner = scopes[owner].entity;
    context.cv = function_qualifiers(parameters).cv; context.available = !entities[e].is_static;
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
    expressions[n] = result; facts[n].type = result.type; facts[n].entity = field; facts[n].scope = s;
    while (scopes[s].kind != ScopeKind::Function) s = scopes[s].parent;
    auto source = ast.nodes.occurrences[n].source;
    template_field_sources.put(source,template_object_context_index.get(s));
    template_fixed_expressions.put(source,n); ++template_fixed_work;
    return true;
}
bool Analyzer::reuse_template_field(NodeId n, ScopeId s, Expression& result)
{
    auto path = template_field_sources.get(ast.nodes.occurrences[n].source);
    if (!path) return false;
    auto context = template_object_contexts[path];
    auto object_type = implicit_object_type(s);
    auto scope = s;
    while (scope && scopes[scope].kind != ScopeKind::Function) scope = scopes[scope].parent;
    auto function = scopes[scope].entity;
    auto cls = scopes[entities[function].owner].entity;
    if (!cls || template_class_patterns.get(cls) != context.owner)
        throw std::logic_error("missing concrete template object owner");
    if (!object_type && !unevaluated_depth) throw std::logic_error("template field use lacks its object");
    auto key = this->key(result.entity,object_type ? object_type : entities[cls].type);
    auto id = template_member_use_index.get(key);
    if (!id) {
        TemplateMemberUse use; use.entity = result.entity;
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
    auto use = template_member_uses[id];
    if (use.type != result.type || result.category != ValueCategory::Lvalue)
        throw std::logic_error("fixed template field value facts changed");
    result.entity = use.entity; result.object_use = use.object;
    if (ast[n].kind == Kind::Member) expression(ast[n].first,s);
    return true;
}
} }
