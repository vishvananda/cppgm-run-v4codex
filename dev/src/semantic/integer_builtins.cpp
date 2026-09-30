#include "semantic/analyzer.h"
#include "support/builtin_registry.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::integer_builtin_function(IdentifierId name)
{
    auto spec = integer_builtin(ids.spelling(name));
    if (spec.operation == IntegerBuiltin::None) return 0;
    auto kind = spec.operation == IntegerBuiltin::Clz ? Intrinsic::Clz :
        spec.operation == IntegerBuiltin::Ctz ? Intrinsic::Ctz :
        spec.operation == IntegerBuiltin::Popcount ? Intrinsic::Popcount :
        spec.operation == IntegerBuiltin::Parity ? Intrinsic::Parity :
        spec.operation == IntegerBuiltin::Ffs ? Intrinsic::Ffs : Intrinsic::Bswap;
    auto input = types.fundamental(spec.width == 16 ? FT_UNSIGNED_SHORT_INT :
        spec.width == 1 ? FT_UNSIGNED_LONG_INT : spec.width == 64 ? FT_UNSIGNED_LONG_LONG_INT : FT_UNSIGNED_INT);
    if (kind == Intrinsic::Ffs) input = types.fundamental(spec.width == 1 ? FT_LONG_INT :
        spec.width == 64 ? FT_LONG_LONG_INT : FT_INT);
    auto result = kind == Intrinsic::Bswap ? input : types.fundamental(FT_INT);
    std::vector<TypeId> args;
    if (spec.width) args.push_back(input);
    else kind = kind == Intrinsic::Clz ? Intrinsic::Clzg : kind == Intrinsic::Ctz ? Intrinsic::Ctzg : Intrinsic::Popcountg;
    auto e = declare_function(global,name,0,types.function(result,args,false));
    entities[e].exception_spec = 129;
    intrinsic_functions.put(e,unsigned(kind)); return e;
}
EntityId Analyzer::integer_signature(EntityId family, TypeId operand, unsigned count)
{
    auto kind = intrinsic_function(family);
    if (count != 1 && !(count == 2 && (kind == Intrinsic::Clzg || kind == Intrinsic::Ctzg))) return 0;
    operand = types.unqualified(operand);
    if (types[operand].kind != TypeKind::Fundamental || !integral(operand) || !is_unsigned(operand) || fundamental(operand,FT_BOOL)) return 0;
    auto identity = key(unsigned(kind)*4+count,operand);
    if (auto old = integer_signatures.get(identity)) return old;
    auto i = types.fundamental(FT_INT);
    std::vector<TypeId> args{operand}; if (count == 2) args.push_back(i);
    auto e = make_entity(EntityKind::Function,global,entities[family].name,0);
    entities[e].type = types.function(i,args,false); entities[e].exception_spec = 129;
    intrinsic_functions.put(e,unsigned(kind)); integer_signatures.put(identity,e); return e;
}
Constant Analyzer::integer_builtin_constant(NodeId n, ScopeId s)
{
    auto call = expressions[n]; auto kind = intrinsic_function(facts[n].entity);
    auto input = constant_node_conversion(call_argument(call,0),conversions[call.conversions],s);
    if (!input.valid) return Constant();
    Constant fallback;
    if (call.argument_count == 2) {
        fallback = constant_node_conversion(call_argument(call,1),conversions[call.conversions+1],s);
        if (!fallback.valid) return Constant();
    }
    auto bits = width(input.type);
    WideInteger x = integer_value(input);
    if (bits < 128) x &= (WideInteger(1)<<bits)-1;
    WideInteger value = 0;
    if (kind == Intrinsic::Bswap) {
        for (unsigned j = 0; j < bits; j += 8) { value = (value<<8) | (x&255); x >>= 8; }
    } else if (kind == Intrinsic::Popcount || kind == Intrinsic::Popcountg || kind == Intrinsic::Parity) {
        for (; x; x &= x-1) ++value;
        if (kind == Intrinsic::Parity) value &= 1;
    } else {
        if (!x) return fallback.valid ? fallback : kind == Intrinsic::Ffs ? Constant(call.type,0) : Constant();
        if (kind == Intrinsic::Clz || kind == Intrinsic::Clzg) {
            auto top = WideInteger(1) << (bits-1);
            while (!(x & top)) { ++value; x <<= 1; }
        } else { while (!(x&1)) { ++value; x >>= 1; } }
        if (kind == Intrinsic::Ffs) ++value;
    }
    return integer_constant(call.type,value);
}
} }
