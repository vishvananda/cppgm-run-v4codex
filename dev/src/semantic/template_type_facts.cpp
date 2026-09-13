#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::substitution_frame(std::uint32_t specialization,
    std::uint32_t parameters, std::uint32_t count, std::uint32_t parent)
{
    // The flat index partitions by specialization/head. The short chain keeps
    // parent identity in equality, including different enclosing overlays.
    auto k = key(specialization,parameters+1);
    auto first = substitution_frame_index.get(k);
    for (auto id = first; id; id = substitution_frames[id].next) {
        auto f = substitution_frames[id];
        if (f.parent == parent && f.count == count) return id;
    }
    TemplateSubstitutionFrame frame;
    frame.specialization = specialization; frame.parameters = parameters;
    frame.count = count; frame.parent = parent; frame.next = first;
    auto id = substitution_frames.size(); substitution_frames.push_back(frame);
    substitution_frame_index.put(k,id); return id;
}
TypeId Analyzer::substitution_argument(std::uint32_t id, EntityId parameter) const
{
    auto ordinal = parameter_ordinals.get(parameter);
    if (!ordinal) return 0;
    for (; id; id = substitution_frames[id].parent) {
        auto frame = substitution_frames[id];
        if (ordinal > frame.count || template_parameters[frame.parameters+ordinal-1] != parameter) continue;
        auto pack = argument_packs[specializations[frame.specialization].arguments];
        // Partial explicit function arguments leave the remaining parameters
        // symbolic until deduction establishes a different specialization.
        return ordinal <= pack.count ? argument_types[pack.offset+ordinal-1] : entities[parameter].type;
    }
    return 0;
}
void Analyzer::attach_template_context(std::uint32_t context, std::uint32_t frame)
{
    auto prior = substitution_frame_contexts.get(frame);
    if (prior && prior != context) throw std::logic_error("substitution frame has two declaration contexts");
    template_type_contexts.put(context,frame); substitution_frame_contexts.put(frame,context);
}
EntityId Analyzer::substitution_entity(std::uint32_t frame, EntityId source) const
{
    auto node = entities[source].source;
    for (; frame; frame = substitution_frames[frame].parent) {
        auto spec = specializations[substitution_frames[frame].specialization];
        auto context = substitution_frame_contexts.get(frame);
        if (!context) context = spec.context;
        auto occurrence = ast.projected(node,context);
        if (occurrence && ast.nodes.occurrences[occurrence].context) {
            auto entity = facts[occurrence].entity;
            if (entity && !entities[entity].template_pattern) return entity;
        }
    }
    return 0;
}
bool Analyzer::pattern_scope(ScopeId scope) const
{
    for (; scope; scope = scopes[scope].parent) if (template_pattern_scopes.get(scope)) return true;
    return false;
}
EntityId Analyzer::substitution_binding(std::uint32_t frame, EntityId source)
{
    if (!source || !entities[source].template_pattern) return source;
    auto k = key(frame,source);
    if (auto known = substitution_binding_cache.get(k)) return known;
    auto entity = entities[source];
    auto result = entity.kind == EntityKind::Overload ?
        merge_lookup(substitution_binding(frame,entity.first),substitution_binding(frame,entity.second)) :
        substitution_entity(frame,source);
    if (!result) throw std::logic_error("missing substituted query declaration");
    substitution_binding_cache.put(k,result); return result;
}
ScopeId Analyzer::substitution_scope(std::uint32_t frame, ScopeId source) const
{
    auto spec = specializations[substitution_frames[frame].specialization];
    for (auto scope = source; scope; scope = scopes[scope].parent) {
        auto e = scopes[scope].entity;
        if (!e) continue;
        if (e == spec.pattern && entities[spec.entity].scope) return entities[spec.entity].scope;
        if (!entities[e].template_pattern) continue;
        auto concrete = substitution_entity(frame,e);
        if (!concrete) continue;
        if (entities[concrete].scope) return entities[concrete].scope;
        // Before a member body exists, its declaring class still supplies the
        // member signature's access context.
        if (scopes[entities[concrete].owner].kind == ScopeKind::Class) return entities[concrete].owner;
    }
    if (pattern_scope(source)) throw std::logic_error("missing substituted query context");
    return source;
}
TypeId Analyzer::bind_template_type(NodeId specs, NodeId d, ScopeId scope)
{
    // A zero result means another semantic owner must first supply a local
    // declaration identity, value-dependent bound, or expression-query form.
    // It is not a guessed concrete type or a caught semantic error.
    struct Probe {
        bool& mode; unsigned& depth; bool saved;
        Probe(bool& m, unsigned& d) : mode(m), depth(d), saved(m) { mode = true; ++depth; }
        ~Probe() { mode = saved; --depth; }
    } probe(template_type_probe,unevaluated_depth);
    auto occurrence = ast.nodes.occurrences[specs];
    auto source = occurrence.source;
    auto known = !occurrence.context ? template_type_sources.get(source) : 0;
    TypeId base = known ? known-1 : specifiers(specs,scope);
    if (!known && !occurrence.context) { template_type_sources.put(source,base+1); ++template_type_work; }
    TypeId type = base ? declarator(d,base,scope,0,true) : 0;
    if (!occurrence.context && d && type && types[type].kind != TypeKind::Function)
        template_type_sources.put(ast.nodes.occurrences[d].source,type+1);
    return type;
}
TypeId Analyzer::bind_template_special_type(NodeId d, ScopeId scope)
{
    struct Probe {
        bool& mode; unsigned& depth; bool saved;
        Probe(bool& m, unsigned& d) : mode(m), depth(d), saved(m) { mode = true; ++depth; }
        ~Probe() { mode = saved; --depth; }
    } probe(template_type_probe,unevaluated_depth);
    auto name = ast[decl_name(d)].last;
    auto result = ast[name].op == KW_OPERATOR && ast[name].detail ?
        type_id(ast[name].detail,scope) : types.fundamental(FT_VOID);
    return result ? declarator(d,result,scope,0,true) : 0;
}
TypeId Analyzer::reuse_template_type(NodeId node, ScopeId scope)
{
    auto occurrence = ast.nodes.occurrences[node];
    if (!occurrence.context) return 0;
    auto known = template_type_sources.get(occurrence.source);
    if (known <= 1) return 0;
    auto type = known-1;
    if (dependent_type(type)) {
        auto frame = template_type_contexts.get(occurrence.context);
        if (!frame) return 0;
        Index bindings, cache;
        type = substitute_type(type,bindings,cache,frame);
        if (!type) throw std::runtime_error("invalid substituted declaration type");
    }
    ++template_type_uses;
    facts[node].type = type; facts[node].scope = scope;
    return type;
}
} }
