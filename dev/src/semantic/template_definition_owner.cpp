#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::template_owner_shape(ScopeId scope, const std::vector<ArgumentId>& arguments)
{
    Index bindings, cache;
    std::vector<ArgumentId> head, shape;
    std::vector<ScopeId> scopes_to_bind;
    for (auto at = scope; at; at = scopes[at].parent)
        if (scopes[at].kind == ScopeKind::Template) scopes_to_bind.push_back(at);
    unsigned depth = 0;
    for (auto it = scopes_to_bind.rbegin(); it != scopes_to_bind.rend(); ++it) {
        auto current = *it; unsigned ordinal = 0;
        if (auto id = definition_source_heads.get(current)) {
            auto source = template_definition_heads[id]; depth = source.depth;
            for (unsigned j = 0; j < source.count; ++j) {
                auto p = template_parameters[source.parameters+j];
                bindings.put(p,canonical_argument(p,j,bindings,cache,depth));
            }
            ++depth; continue;
        }
        for (auto d = scopes[current].first_decl; d; d = declarations[d].next) {
            auto p = declarations[d].entity;
            if (!entities[p].template_parameter) continue;
            auto arg = canonical_argument(p,ordinal++,bindings,cache,depth);
            bindings.put(p,arg);
            if (current == scope)
                head.push_back(entities[p].parameter_pack ? types.compound(TypeKind::PackExpansion,0,arg) : arg);
        }
        if (ordinal) ++depth;
    }
    for (auto arg : arguments) {
        auto value = substitute_argument(arg,bindings,cache);
        if (!value) throw std::runtime_error("invalid template owner argument");
        shape.push_back(value);
    }
    return intern_arguments({intern_arguments(head),intern_arguments(shape)});
}
EntityId Analyzer::template_definition_pattern(EntityId primary, NodeId part, ScopeId scope)
{
    std::vector<ArgumentId> arguments, generic;
    auto list = child(part,syntax::Kind::TemplateArguments);
    for (auto a = ast[list].first; a; a = ast[a].next)
        append_template_argument(a,scope,template_argument_node(a,scope),arguments);
    auto shape = template_owner_shape(scope,arguments);
    auto components = argument_packs[shape];
    auto header = argument_types[components.offset];
    auto normalized = argument_packs[argument_types[components.offset+1]];
    arguments.assign(argument_types.begin()+normalized.offset,argument_types.begin()+normalized.offset+normalized.count);
    auto head = templates[entities[primary].template_info];
    Index bindings, cache;
    template_signature_bindings(head.environment,primary,bindings);
    // Definition matching operates on canonical source heads. The primary's
    // enclosing parameters and a renamed definition head have equivalent
    // positional identity, even when a non-type parameter's type depends on
    // an enclosing type parameter. Concrete specialization defaults cannot
    // substitute those two distinct source environments for one another.
    for (unsigned j = 0; j < head.count; ++j) {
        auto parameter = template_parameters[head.offset+j];
        if (entities[parameter].parameter_pack) {
            std::vector<ArgumentId> values;
            if (j < arguments.size() && argument_pack(arguments[j])) {
                auto pack = pack_arguments(arguments[j]);
                values.assign(argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count);
            } else if (j < arguments.size()) values.assign(arguments.begin()+j,arguments.end());
            if (entities[parameter].kind != EntityKind::Type) {
                auto target = substitute_type(entities[parameter].type,bindings,cache);
                for (auto& value : values) {
                    if (!value_argument(value) && types[value].kind == TypeKind::PackExpansion) continue;
                    value = target ? convert_argument(value,target) : 0;
                    if (!value) throw std::runtime_error("invalid definition owner pack argument type");
                }
            }
            arguments.resize(j+1); arguments[j] = make_argument_pack(values);
        } else if (j == arguments.size()) {
            auto value = template_default_types.get(parameter);
            if (!value && entities[parameter].initializer)
                value = template_argument_node(ast[entities[parameter].initializer].first,head.environment);
            if (!value) throw std::runtime_error("missing definition owner argument");
            arguments.push_back(substitute_argument(value,bindings,cache));
        }
        if (entities[parameter].kind != EntityKind::Type && !entities[parameter].parameter_pack) {
            auto target = substitute_type(entities[parameter].type,bindings,cache);
            arguments[j] = target ? convert_argument(arguments[j],target) : 0;
            if (!arguments[j]) throw std::runtime_error("invalid definition owner argument type");
        }
        bindings.put(parameter,arguments[j]);
    }
    if (arguments.size() != head.count) throw std::runtime_error("excess definition owner arguments");
    shape = intern_arguments({header,intern_arguments(arguments)});
    for (unsigned j = 0; j < head.count; ++j) {
        auto p = template_parameters[head.offset+j];
        auto arg = parameter_argument(p);
        generic.push_back(entities[p].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
    }
    if (shape == template_owner_shape(head.environment,generic)) return primary;
    if (auto partial = class_partial_signatures.get(key(primary,shape))) return partial;
    throw std::runtime_error("member definition does not name a declared template owner");
}
} }
