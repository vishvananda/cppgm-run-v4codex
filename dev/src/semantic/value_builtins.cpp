#include "semantic/analyzer.h"
#include "support/builtin_registry.h"
#include "support/type_traits.h"
#include <cstring>
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeId Analyzer::builtin_value_type(unsigned operation, TypeId target, TypeId source)
{
    auto kind = ValueBuiltin(operation);
    target = types.unqualified(target); source = types.unqualified(source);
    if (kind == ValueBuiltin::ReduceOr)
        return vector_kind(types[source].kind) && integral(types[source].child) ? types[source].child : 0;
    if (kind == ValueBuiltin::ConvertVector)
        return vector_kind(types[source].kind) && vector_kind(types[target].kind) &&
            vector_elements(source) == vector_elements(target) ? target : 0;
    if (kind != ValueBuiltin::BitCast || !complete_object_type(source) || !complete_object_type(target) ||
        types[target].kind == TypeKind::LRef || types[target].kind == TypeKind::RRef || types[target].kind == TypeKind::Array ||
        size(source) != size(target)) return 0;
    return builtin_type_property(unsigned(BuiltinTrait::TriviallyCopyable),source) &&
        builtin_type_property(unsigned(BuiltinTrait::TriviallyCopyable),target) ? target : 0;
}
Expression Analyzer::builtin_value_expression(NodeId n, ScopeId s)
{
    auto node = ast[n]; auto source = expression(node.first,s);
    Expression result;
    result.type = builtin_value_type(node.flags,node.detail ? type_id(node.detail,s) : 0,source.type);
    if (!result.type) throw std::runtime_error("invalid typed builtin operand");
    if (ValueBuiltin(node.flags) == ValueBuiltin::BitCast) {
        // Inspect the representation of the operand itself: no array decay,
        // converting constructor, or language copy is part of this operation.
        Conversion c; c.target = source.type; c.rank = 0;
        record_conversion(result,node.first,c); observe_scalar(node.first);
    } else record_conversion(result,node.first,conversion(node.first,types.unqualified(source.type)));
    return result;
}
TypeId Analyzer::vector_binary_type(ETokenType op, TypeId a, TypeId b)
{
    a = types.unqualified(a); b = types.unqualified(b);
    if (!vector_kind(types[a].kind) || a != b) return 0;
    auto lane = types[a].child;
    if (op == OP_EQ || op == OP_NE || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE) {
        if (fundamental(lane,FT_BOOL)) return 0;
        auto bytes = size(lane);
        auto signed_lane = types.fundamental(bytes == 1 ? FT_SIGNED_CHAR : bytes == 2 ? FT_SHORT_INT : bytes == 4 ? FT_INT : bytes == 8 ? FT_LONG_INT : FT_INT128);
        return types.compound(types[a].kind,signed_lane,types[a].bound);
    }
    bool arithmetic_op = op == OP_PLUS || op == OP_MINUS || op == OP_STAR || op == OP_DIV;
    bool integer_op = op == OP_MOD || op == OP_AMP || op == OP_BOR || op == OP_XOR || op == OP_LSHIFT || op == OP_RSHIFT;
    return (arithmetic_op && !fundamental(lane,FT_BOOL)) || (integer_op && integral(lane)) ? a : 0;
}
Constant Analyzer::builtin_value_constant(unsigned operation, TypeId target, Constant source)
{
    if (!source.valid) return {};
    auto kind = ValueBuiltin(operation);
    if (kind == ValueBuiltin::BitCast) {
        // Scalar representations have no padding and can be transferred
        // without evaluating a language conversion. Pointers are not permitted
        // in a constant bit cast; runtime lowering copies their representation.
        unsigned char bytes[16] = {};
        auto width = size(source.type);
        if (width > sizeof(bytes)) return {};
        if (integral(source.type)) { auto value = integer_value(source); std::memcpy(bytes,&value,width); }
        else if (floating_type(source.type)) {
            auto value = floating_value(source);
            if (fundamental(source.type,FT_FLOAT)) { float x = value; std::memcpy(bytes,&x,4); if (floating_signaling(source)) bytes[2] &= 0xbf; }
            else if (fundamental(source.type,FT_DOUBLE)) { double x = value; std::memcpy(bytes,&x,8); if (floating_signaling(source)) bytes[6] &= 0xf7; }
            else return {};
        } else return {};
        if (integral(target)) {
            unsigned __int128 value = 0; std::memcpy(&value,bytes,width);
            if (fundamental(target,FT_BOOL) && value > 1) return {};
            return integer_constant(target,value);
        }
        if (fundamental(target,FT_FLOAT)) { float x; std::uint32_t bits; std::memcpy(&x,bytes,4); std::memcpy(&bits,bytes,4); bool snan = (bits & 0x7fc00000u) == 0x7f800000u && (bits & 0x3fffffu); return floating_constant(target,x,true,snan); }
        if (fundamental(target,FT_DOUBLE)) { double x; std::uint64_t bits; std::memcpy(&x,bytes,8); std::memcpy(&bits,bytes,8); bool snan = (bits & 0x7ff8000000000000ull) == 0x7ff0000000000000ull && (bits & 0x7ffffffffffffull); return floating_constant(target,x,true,snan); }
        return {};
    }
    auto lanes = vector_elements(source.type);
    if (lanes > 1000000) return {};
    if (kind == ValueBuiltin::ReduceOr) {
        auto value = integer_constant(target,0);
        for (std::uint64_t i = 0; i < lanes; ++i) {
            if (constant_depth && !constant_step()) return {};
            auto lane = evaluated_part(source,i);
            if (!lane.valid) return {};
            value = integer_constant(target,integer_value(value) | integer_value(lane));
        }
        return value;
    }
    std::vector<EvaluatedPart> parts;
    for (std::uint64_t i = 0; i < lanes; ++i) {
        if (constant_depth && !constant_step()) return {};
        EvaluatedPart part; part.selector = i;
        part.value = convert(evaluated_part(source,i),types[target].child,true);
        if (!part.value.valid) return {};
        parts.push_back(part);
    }
    return evaluated_object(target,parts);
}
} }
