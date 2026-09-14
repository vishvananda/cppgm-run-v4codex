#include "semantic/analyzer.h"
#include <stdexcept>
#include <algorithm>
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::retain_definition_head(ScopeId scope, EntityId pattern, unsigned depth)
{
    TemplateDefinitionHead head; head.pattern = pattern; head.depth = depth;
    head.parameters = template_parameters.size();
    for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
        auto p = declarations[d].entity;
        if (!entities[p].template_parameter) continue;
        parameter_ordinals.put(p,++head.count); template_parameters.push_back(p);
    }
    auto id = template_definition_heads.size(); template_definition_heads.push_back(head);
    return id;
}
std::uint32_t Analyzer::definition_frame(const TemplateDefinition& def, EntityId owner, ScopeId environment)
{
    std::vector<EntityId> owners;
    for (auto e = owner; e;) {
        owners.push_back(e);
        auto enclosing = scopes[entities[e].owner].kind == ScopeKind::Class ?
            definition_owner(scopes[entities[e].owner].entity).specialization : 0;
        e = enclosing;
    }
    if (owners.size() != def.head_count) throw std::logic_error("definition owner/head depth mismatch");
    std::reverse(owners.begin(),owners.end());
    auto context = specializations[entities[owner].specialization].context;
    auto frame = template_type_contexts.get(context);
    if (!frame) throw std::logic_error("definition owner has no completed declaration frame");
    auto apply = [&](TemplateDefinitionHead head, bool names) {
        if (head.depth >= owners.size()) throw std::logic_error("definition head outside its owner chain");
        auto spec = entities[owners[head.depth]].specialization;
        auto selection = specializations[spec];
        auto arguments = selection.definition_arguments ? selection.definition_arguments : selection.arguments;
        auto pack = argument_packs[arguments];
        if (pack.count != head.count) throw std::logic_error("definition head argument count mismatch");
        if (names) for (unsigned j = 0; j < head.count; ++j)
            bind_argument(environment,template_parameters[head.parameters+j],argument_types[pack.offset+j]);
        frame = substitution_frame(spec,head.parameters,head.count,frame,arguments);
    };
    // Aliases in out-of-class nested declarations can refer to an earlier
    // source head. Compose those source identities over the established class
    // frames; each tuple is shared, and no visible binding map is copied.
    std::vector<std::uint32_t> source_heads;
    for (auto scope = facts[def.declarator].scope; scope; scope = scopes[scope].parent)
        if (auto id = definition_source_heads.get(scope))
            if (id < def.heads || id >= def.heads+def.head_count) source_heads.push_back(id);
    for (auto it = source_heads.rbegin(); it != source_heads.rend(); ++it) apply(template_definition_heads[*it],false);
    for (unsigned j = 0; j < def.head_count; ++j) apply(template_definition_heads[def.heads+j],true);
    return frame;
}
} }
