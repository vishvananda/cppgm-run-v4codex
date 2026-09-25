#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::deduce_conversion(EntityId pattern, TypeId target)
{
    // [temp.deduct.conv]: the destination supplies A, and only the declared
    // conversion type supplies P. No body or token replay participates.
    auto head = templates[entities[pattern].template_info];
    TypeId p = types.alias_target(value_type(types.alias_target(types[entities[pattern].type].child)));
    bool reference = types[target].kind == TypeKind::LRef || types[target].kind == TypeKind::RRef;
    TypeId a = reference ? types[target].child : types.unqualified(target);
    if (!reference) p = decay(p);
    Index bindings;
    if (!deduce_type(p,a,bindings,DeductionKind::ClassPattern)) {
        // The qualification alternatives are considered only after exact
        // deduction fails. Strip corresponding pointer levels, then deduce
        // from unqualified terminal types. The original substituted result
        // still has to pass the ordinary exact-rank conversion check.
        bindings = Index();
        if (!reference && !pointer(p)) return 0;
        TypeId pp = p, aa = a;
        while (pointer(pp) && pointer(aa)) { pp = types[pp].child; aa = types[aa].child; }
        if (pointer(pp) != pointer(aa) ||
            !deduce_type(types.unqualified(pp),types.unqualified(aa),bindings,DeductionKind::ClassPattern)) return 0;
    }
    std::vector<ArgumentId> arguments;
    for (unsigned i = 0; i < head.count; ++i) {
        auto parameter = template_parameters[head.offset+i];
        auto argument = bindings.get(parameter);
        if (!argument && entities[parameter].parameter_pack) argument = make_argument_pack({});
        arguments.push_back(argument);
    }
    return deduced_specialization(pattern,arguments);
}
} }
