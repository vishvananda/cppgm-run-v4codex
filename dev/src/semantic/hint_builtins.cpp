#include "semantic/analyzer.h"
#include "support/builtin_registry.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::hint_builtin(IdentifierId name)
{
    auto spec = function_builtin(ids.spelling(name));
    if (spec < FunctionBuiltin::Prefetch || spec > FunctionBuiltin::FltRounds) return 0;
    auto v = types.fundamental(FT_VOID), i = types.fundamental(FT_INT);
    auto ptr = types.compound(TypeKind::Pointer,v);
    auto cp = types.compound(TypeKind::Pointer,types.qualify(v,1));
    auto size = types.fundamental(FT_UNSIGNED_LONG_INT);
    auto kind = spec == FunctionBuiltin::Prefetch ? Intrinsic::Prefetch :
        spec == FunctionBuiltin::AssumeAligned ? Intrinsic::AssumeAligned : Intrinsic::FltRounds;
    auto result = kind == Intrinsic::Prefetch ? v : kind == Intrinsic::AssumeAligned ? ptr : i;
    unsigned first = kind == Intrinsic::Prefetch ? 1 : kind == Intrinsic::AssumeAligned ? 2 : 0;
    unsigned last = kind == Intrinsic::FltRounds ? 0 : 3;
    for (unsigned count = first; count <= last; ++count) {
        std::vector<TypeId> args;
        if (count) args.push_back(cp);
        if (count >= 2) args.push_back(kind == Intrinsic::Prefetch ? i : size);
        if (count >= 3) args.push_back(kind == Intrinsic::Prefetch ? i : types.fundamental(FT_LONG_INT));
        auto e = declare_function(global,name,0,types.function(result,args,false));
        entities[e].exception_spec = 129;
        intrinsic_functions.put(e,unsigned(kind));
        if (kind == Intrinsic::FltRounds) { entities[e].c_linkage = true; assembler_names.put(e,ids.intern(TextView("fegetround",10))); }
    }
    return local(global,name);
}
} }
