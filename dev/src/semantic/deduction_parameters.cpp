#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Index Analyzer::deducible_parameters(TypeId function)
{
    // Declaration-time deducibility uses only the typed parameter graph. It
    // must not instantiate classes or trial-call a hypothetical function.
    Index result, seen;
    std::vector<ArgumentId> work;
    auto parameter_list = [&](Type type) {
        for (unsigned j = 0; j < type.count; ++j) {
            auto p = types.parameters[type.offset+j];
            if (types[p].kind == TypeKind::PackExpansion) {
                if (j+1 != type.count) continue;
                p = types[p].bound;
            }
            work.push_back(p);
        }
    };
    auto argument_list = [&](TypeArguments args) {
        // A nonfinal expansion makes a template argument list non-deduced.
        for (unsigned j = 0; j+1 < args.count; ++j) {
            auto arg = argument_types[args.offset+j];
            if (!value_argument(arg) && types[arg].kind == TypeKind::PackExpansion) return;
        }
        for (unsigned j = 0; j < args.count; ++j) work.push_back(argument_types[args.offset+j]);
    };
    parameter_list(types[function]);
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto arg = work[i];
        if (!arg || seen.get(arg)) continue;
        seen.put(arg,1);
        if (value_argument(arg)) {
            auto q = type_queries[argument_query(arg)];
            while (q.kind == QueryKind::Cast && q.op == TOK_INVALID) q = type_queries[query_edges[q.offset]];
            if (q.kind == QueryKind::TemplateValueParameter) { result.put(q.entity,1); work.push_back(q.type); }
            continue;
        }
        auto type = types[arg];
        switch (type.kind) {
        case TypeKind::Named:
            if (entities[type.entity].template_parameter) result.put(type.entity,1);
            else if (entities[type.entity].specialization) {
                auto primary = specialization_pattern(type.entity);
                if (entities[primary].template_parameter) result.put(primary,1);
                argument_list(specialization_arguments(type.entity));
            }
            break;
        case TypeKind::ArgumentPack: argument_list(argument_packs[type.bound]); break;
        case TypeKind::PackExpansion: work.push_back(type.bound); break;
        case TypeKind::DependentBitInt: case TypeKind::DependentName: case TypeKind::Decltype: case TypeKind::DependentVector: break;
        case TypeKind::DependentArray: case TypeKind::DependentExtVector:
            work.push_back(value_argument_id(type.bound)); work.push_back(type.child); break;
        case TypeKind::Function: parameter_list(type); work.push_back(type.child); break;
        case TypeKind::MemberPointer: work.push_back(type.member_owner()); work.push_back(type.child); break;
        default: if (type.child) work.push_back(type.child); break;
        }
    }
    return result;
}
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
