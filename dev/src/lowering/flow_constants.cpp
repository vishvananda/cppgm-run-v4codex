#include "lowering/flow_proof.h"
#include <limits>
namespace cppgm { namespace lowering {
using namespace lowir_model;
FlowInteger flow_integer(const Instruction& i, FlowInteger a, FlowInteger b)
{
    bool binary = i.opcode == Opcode::Binary || i.opcode == Opcode::Compare;
    if (!i.type.integer() || i.type.width() > 64 ||
        (!binary && i.opcode != Opcode::Const && i.opcode != Opcode::Copy &&
         i.opcode != Opcode::Convert && i.opcode != Opcode::Unary)) return FlowInteger(FlowInteger::Varying);
    if (a.state == FlowInteger::Varying || (binary && b.state == FlowInteger::Varying)) return FlowInteger(FlowInteger::Varying);
    if (a.state == FlowInteger::Pending || (binary && b.state == FlowInteger::Pending)) return FlowInteger();
    auto width = i.type.width();
    auto mask = width == 64 ? ~std::uint64_t(0) : (std::uint64_t(1)<<width)-1;
    auto signed_value = [&](std::uint64_t bits) {
        bits &= mask;
        if (width < 64 && (bits & (std::uint64_t(1)<<(width-1)))) bits |= ~mask;
        return std::int64_t(bits);
    };
    auto x = a.bits & mask, y = b.bits & mask;
    auto sx = signed_value(x), sy = signed_value(y);
    FlowInteger value(FlowInteger::Constant);
    if (i.opcode == Opcode::Const || i.opcode == Opcode::Copy) value.bits = x;
    else if (i.opcode == Opcode::Convert) {
        if (!i.source_type.integer() || i.source_type.width() > 64) return FlowInteger(FlowInteger::Varying);
        auto from = i.source_type.width();
        auto input_mask = from == 64 ? ~std::uint64_t(0) : (std::uint64_t(1)<<from)-1;
        value.bits = a.bits & input_mask;
        if (i.operation == Operation::Sext && from < 64 && (value.bits & (std::uint64_t(1)<<(from-1)))) value.bits |= ~input_mask;
        else if (i.operation != Operation::Sext && i.operation != Operation::Zext && i.operation != Operation::Trunc) return FlowInteger(FlowInteger::Varying);
        value.bits &= mask;
    } else if (i.opcode == Opcode::Unary) {
        if (i.operation == Operation::Neg) value.bits = (0-x)&mask;
        else if (i.operation == Operation::Bitnot) value.bits = (~x)&mask;
        else if (i.operation == Operation::Not) value.bits = !x;
        else return FlowInteger(FlowInteger::Varying);
    } else if (i.opcode == Opcode::Compare) {
        switch (i.operation) {
        case Operation::Eq: value.bits = x==y; break;
        case Operation::Ne: value.bits = x!=y; break;
        case Operation::Lt: value.bits = sx<sy; break;
        case Operation::Le: value.bits = sx<=sy; break;
        case Operation::Gt: value.bits = sx>sy; break;
        case Operation::Ge: value.bits = sx>=sy; break;
        case Operation::Ult: value.bits = x<y; break;
        case Operation::Ule: value.bits = x<=y; break;
        case Operation::Ugt: value.bits = x>y; break;
        case Operation::Uge: value.bits = x>=y; break;
        default: return FlowInteger(FlowInteger::Varying);
        }
    } else if (i.opcode == Opcode::Binary) {
        switch (i.operation) {
        case Operation::Add: value.bits=x+y; break;
        case Operation::Sub: value.bits=x-y; break;
        case Operation::Mul: value.bits=x*y; break;
        case Operation::And: value.bits=x&y; break;
        case Operation::Or: value.bits=x|y; break;
        case Operation::Xor: value.bits=x^y; break;
        case Operation::Shl: if(y>=width)return FlowInteger(FlowInteger::Varying); value.bits=x<<y; break;
        case Operation::Ushr: if(y>=width)return FlowInteger(FlowInteger::Varying); value.bits=x>>y; break;
        case Operation::Shr: if(y>=width)return FlowInteger(FlowInteger::Varying); value.bits=std::uint64_t(sx>>y); break;
        case Operation::Udiv: if(!y)return FlowInteger(FlowInteger::Varying); value.bits=x/y; break;
        case Operation::Umod: if(!y)return FlowInteger(FlowInteger::Varying); value.bits=x%y; break;
        case Operation::Div: case Operation::Mod:
            if(!sy || (sx==std::numeric_limits<std::int64_t>::min() && sy==-1))return FlowInteger(FlowInteger::Varying);
            value.bits=i.operation==Operation::Div?sx/sy:sx%sy; break;
        default: return FlowInteger(FlowInteger::Varying);
        }
        value.bits &= mask;
    } else return FlowInteger(FlowInteger::Varying);
    return value;
}
} }
