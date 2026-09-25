#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::substitute_arguments(ArgumentId arg, const Index& bindings, Index& cache,
    std::uint32_t frame, std::vector<ArgumentId>& out)
{
    if (value_argument(arg) || types[arg].kind != TypeKind::PackExpansion) {
        out.push_back(substitute_argument(arg,bindings,cache,frame)); return;
    }
    auto recipe = types[arg];
    auto pattern = ArgumentId(recipe.bound);
    auto original_frame = frame;
    auto captures = argument_packs[recipe.entity];
    for (unsigned j = 0; j < captures.count; j += 2) {
        auto value = substitute_argument(argument_types[captures.offset+j+1],bindings,cache,original_frame);
        if (!value) { out.push_back(0); return; }
        frame = argument_frame(frame,argument_types[captures.offset+j],value);
    }
    auto params = expansion_parameters(pattern);
    auto count = expansion_count(params,bindings,frame);
    if (count == UnequalPacks) { out.push_back(0); return; }
    if (count >= 0 && frame) {
        for (int j = 0; j < count; ++j) {
            auto lane = expansion_frame(frame,params,j);
            auto value = substitute_argument(pattern,bindings,cache,lane);
            out.push_back(value && substitution_frames[lane].symbolic ? types.pack_expansion(value,0) : value);
        }
        return;
    }
    if (count == DeferredPacks) {
        // Capture only the packs consumed by this expansion. Shield their
        // scalar pattern uses while substituting all other dependent facts.
        // The capture tuple is immutable; it composes with the later frame.
        auto list = argument_packs[params]; std::vector<ArgumentId> retained;
        auto shield = frame;
        for (unsigned j = 0; j < list.count; ++j) {
            auto p = argument_types[list.offset+j];
            retained.push_back(p); retained.push_back(unexpanded_argument(frame,p));
            shield = argument_frame(shield,p,parameter_argument(p));
        }
        auto value = substitute_argument(pattern,bindings,cache,shield);
        out.push_back(value ? types.pack_expansion(value,intern_arguments(retained)) : 0);
        return;
    }
    auto value = substitute_argument(pattern,bindings,cache,frame);
    out.push_back(value ? types.pack_expansion(value,0) : 0);
}
} }
