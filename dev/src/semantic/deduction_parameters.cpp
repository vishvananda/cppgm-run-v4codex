#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::deduction_parameters(Type function, std::uint32_t prefix, std::vector<DeductionParameter>& out)
{
    unsigned first = 0;
    for (; first+1 < function.count; ++first)
        if (types[types.parameters[function.offset+first]].kind == TypeKind::PackExpansion) break;
    if (first+1 >= function.count) return true;
    // [temp.deduct.type]/5: a nonfinal function parameter pack supplies no
    // deductions. Only its explicitly supplied prefix occupies argument lanes.
    // Retain source ordinals for defaults; these are candidate-local views,
    // not substituted declarations or invented semantic types.
    for (unsigned i = 0; i < function.count; ++i) {
        auto type = types.parameters[function.offset+i];
        if (i+1 == function.count || types[type].kind != TypeKind::PackExpansion) {
            out.push_back({type,i,false}); continue;
        }
        Index empty;
        auto count = prefix ? expansion_count(expansion_parameters(types[type].bound),empty,prefix) : UnboundPack;
        if (count == UnboundPack) count = 0;
        if (count < 0) return false;
        for (int j = 0; j < count; ++j) out.push_back({types[type].bound,i,true});
    }
    return true;
}
std::uint32_t Analyzer::specialization_defaults(EntityId pattern, std::uint32_t frame)
{
    auto defaults = entities[pattern].defaults;
    if (!defaults) return 0;
    auto f = types[entities[pattern].type];
    bool packs = false, present = false;
    for (unsigned i = 0; i < f.count; ++i) {
        packs |= types[types.parameters[f.offset+i]].kind == TypeKind::PackExpansion;
        present |= default_arguments[defaults+i] != 0;
    }
    if (!packs) return defaults;
    if (!present) return 0;
    // Defaults retain their source identity but their parameter positions must
    // follow the concrete expansion. One immutable slice belongs to each
    // completed declaration, independently of default-expression demand.
    std::vector<NodeId> arguments;
    Index empty;
    for (unsigned i = 0; i < f.count; ++i) {
        auto type = types[types.parameters[f.offset+i]];
        if (type.kind != TypeKind::PackExpansion) arguments.push_back(default_arguments[defaults+i]);
        else {
            auto count = expansion_count(expansion_parameters(type.bound),empty,frame);
            if (count < 0) throw std::logic_error("completed function has an unbound parameter pack");
            arguments.insert(arguments.end(),count,0);
        }
    }
    auto result = default_arguments.size();
    default_arguments.insert(default_arguments.end(),arguments.begin(),arguments.end());
    return result;
}
} }
