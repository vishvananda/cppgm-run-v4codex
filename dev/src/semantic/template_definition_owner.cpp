#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::template_owner_shape(ScopeId scope, const std::vector<ArgumentId>& arguments)
{
    Index bindings, cache;
    std::vector<ArgumentId> head, shape;
    for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
        auto p = declarations[d].entity;
        if (!entities[p].template_parameter) continue;
        auto arg = canonical_argument(p,head.size(),bindings,cache);
        bindings.put(p,arg);
        head.push_back(entities[p].parameter_pack ? types.compound(TypeKind::PackExpansion,0,arg) : arg);
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
    if (!template_defaults(primary,arguments)) throw std::runtime_error("invalid definition owner arguments");
    auto shape = template_owner_shape(scope,arguments);
    auto head = templates[entities[primary].template_info];
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
