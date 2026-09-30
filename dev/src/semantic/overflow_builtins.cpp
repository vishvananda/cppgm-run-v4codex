#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::overflow_signature(EntityId family, const std::vector<Expression>& values, const std::vector<NodeId>* nodes)
{
    std::vector<TypeId> args;
    for (unsigned j = 0; j < 3; ++j) args.push_back(types.unqualified(nodes ? expressions[(*nodes)[j]].type : values[j].type));
    for (unsigned j = 0; j < 2; ++j) if (!integral(args[j]) || scoped_enum(args[j])) return 0;
    if (!pointer(args[2])) return 0;
    auto target = types[args[2]].child;
    if (types[target].kind != TypeKind::Fundamental || !integral(target) || fundamental(target,FT_BOOL) || (types[target].cv & 1)) return 0;
    auto signature = types.function(types.fundamental(FT_BOOL),args,false);
    auto kind = intrinsic_function(family); auto identity = key(unsigned(kind),signature);
    if (auto old = overflow_signatures.get(identity)) return old;
    auto e = make_entity(EntityKind::Function,global,entities[family].name,0);
    entities[e].type = signature; entities[e].exception_spec = 129;
    intrinsic_functions.put(e,unsigned(kind)); overflow_signatures.put(identity,e); return e;
}
} }
