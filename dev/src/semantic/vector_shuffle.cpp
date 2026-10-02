#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::shuffle_signature(EntityId family, const std::vector<Expression>& values, const std::vector<NodeId>* nodes)
{
    std::vector<TypeId> args;
    unsigned count = nodes ? nodes->size() : values.size();
    for (unsigned j=0; j<count; ++j) args.push_back(types.unqualified(nodes ? expressions[(*nodes)[j]].type : values[j].type));
    if (!vector_kind(types[args[0]].kind) || (count == 3 && args[0] != args[1])) return 0;
    auto mask = args.back();
    if (!vector_kind(types[mask].kind) || !integral(types[mask].child) ||
        vector_elements(mask) != vector_elements(args[0]) || size(mask) != size(args[0])) return 0;
    auto signature = types.function(args[0],args,false);
    if (auto old = shuffle_signatures.get(signature)) return old;
    auto e = make_entity(EntityKind::Function,global,entities[family].name,0);
    entities[e].type = signature; entities[e].exception_spec = 129;
    intrinsic_functions.put(e,unsigned(Intrinsic::VectorShuffle)); shuffle_signatures.put(signature,e); return e;
}
} }
