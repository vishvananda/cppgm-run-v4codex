#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
EntityId Analyzer::new_deallocation(const PlacementNew& use, bool force_global, ScopeId scope)
{
    if (!use.call.argument_count) return select_deallocation(use.leaf,use.array,force_global,scope,false,false);
    EntityId family = 0;
    auto name = operator_name(KW_DELETE,use.array);
    if (!force_global && class_value(use.leaf))
        family = lookup(entities[types[use.leaf].entity].scope,name,Lookup::Ordinary,true);
    if (!family) family = lookup(global,name);
    auto allocation = types[entities[use.allocation].type];
    auto pointer = types.compound(TypeKind::Pointer,types.fundamental(FT_VOID));
    EntityId selected = 0;
    for (auto e : candidates(family)) {
        ++candidate_work;
        auto f = types[entities[e].type];
        if (entities[e].template_info || f.kind != TypeKind::Function || f.variadic != allocation.variadic ||
            f.count != allocation.count || types.parameters[f.offset] != pointer) continue;
        bool match = true;
        for (unsigned j = 1; j < f.count; ++j)
            match &= types.parameters[f.offset+j] == types.parameters[allocation.offset+j];
        if (!match) continue;
        // A usual sized delete cannot serve as a placement delete.
        if (f.count == 2 && fundamental(types.parameters[f.offset+1],FT_UNSIGNED_LONG_INT))
            throw std::runtime_error("placement allocation matches usual sized delete");
        if (selected) return 0;
        selected = e;
    }
    if (selected) {
        if (deleted_transfer(selected)) throw std::runtime_error("deleted placement deallocation");
        check_access(selected,scope,entities[selected].owner);
    }
    return selected;
}
} }
