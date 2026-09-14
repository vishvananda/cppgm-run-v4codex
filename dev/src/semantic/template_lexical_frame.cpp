#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::template_lexical_frame(ScopeId scope)
{
    // Source heads have immutable parameter sets. Sharing the enclosing frame
    // lets an alias be applied while its outer class remains dependent.
    if (!scope) return 0;
    if (auto known = template_lexical_frames.get(scope)) return known-1;
    auto parent = template_lexical_frame(scopes[scope].parent);
    auto result = parent;
    if (scopes[scope].kind == ScopeKind::Template) {
        auto offset = template_parameters.size();
        std::vector<ArgumentId> arguments;
        for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
            auto p = declarations[d].entity;
            if (!entities[p].template_parameter) continue;
            parameter_ordinals.put(p,arguments.size()+1);
            template_parameters.push_back(p);
            auto arg = parameter_argument(p);
            arguments.push_back(entities[p].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
        }
        if (!arguments.empty()) result = substitution_frame(0,offset,arguments.size(),parent,intern_arguments(arguments));
    }
    template_lexical_frames.put(scope,result+1);
    return result;
}
} }
