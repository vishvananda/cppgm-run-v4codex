#include "lowering/flow_proof.h"
#include <limits>
namespace cppgm { namespace lowering {
using namespace lowir_model;
FlowInteger FlowConstants::get(const Operand& operand) const
{
    FlowInteger result;
    if (operand.kind == Operand::Integer && !operand.wide_integer) {
        result.known = true; result.bits = operand.data.integer;
    } else if (operand.kind == Operand::Temporary && operand.ref >= first && operand.ref-first < values.size())
        result = values[operand.ref-first];
    return result;
}
FlowConstants flow_constants(const Program& p, Range range, std::size_t& work)
{
    FlowConstants result; unsigned last = 0;
    for (auto n = range.begin; n < range.end(); ++n) {
        ++work;
        auto id = p.instructions[n].destination.index; if (!id) continue;
        if (!result.first || id < result.first) result.first = id;
        if (id > last) last = id;
    }
    if (!result.first) return result;
    result.values.resize(last-result.first+1);
    std::vector<unsigned char> definitions(result.values.size());
    for (auto n = range.begin; n < range.end(); ++n) {
        ++work;
        auto id = p.instructions[n].destination.index; if (!id) continue;
        auto& count = definitions[id-result.first]; if (count < 2) ++count;
    }
    // Prove single-definition integer expressions once in emission order.
    // Loads, phi joins, calls, floats and repeated definitions stay unknown;
    // this neither transforms IR nor starts semantic/constexpr evaluation.
    for (auto n = range.begin; n < range.end(); ++n) {
        ++work;
        const auto& i = p.instructions[n];
        if (!i.destination || definitions[i.destination.index-result.first] != 1 || !i.type.integer() || i.type.width() > 64 || !i.operands.count) continue;
        auto a = result.get(p.operands[i.operands.begin]); if (!a.known) continue;
        auto b = i.operands.count > 1 ? result.get(p.operands[i.operands.begin+1]) : FlowInteger();
        auto width = i.type.width();
        auto mask = width == 64 ? ~std::uint64_t(0) : (std::uint64_t(1)<<width)-1;
        auto signed_value = [&](std::uint64_t bits) {
            bits &= mask;
            if (width < 64 && (bits & (std::uint64_t(1)<<(width-1)))) bits |= ~mask;
            return std::int64_t(bits);
        };
        auto x = a.bits & mask, y = b.bits & mask;
        auto sx = signed_value(x), sy = signed_value(y);
        FlowInteger value; value.known = true;
        if (i.opcode == Opcode::Const || i.opcode == Opcode::Copy) value.bits = x;
        else if (i.opcode == Opcode::Convert) {
            if (!i.source_type.integer() || i.source_type.width() > 64) continue;
            auto from = i.source_type.width();
            auto input_mask = from == 64 ? ~std::uint64_t(0) : (std::uint64_t(1)<<from)-1;
            value.bits = a.bits & input_mask;
            if (i.operation == Operation::Sext && from < 64 && (value.bits & (std::uint64_t(1)<<(from-1)))) value.bits |= ~input_mask;
            else if (i.operation != Operation::Sext && i.operation != Operation::Zext && i.operation != Operation::Trunc) continue;
            value.bits &= mask;
        } else if (i.opcode == Opcode::Unary) {
            if (i.operation == Operation::Neg) value.bits = (0-x)&mask;
            else if (i.operation == Operation::Bitnot) value.bits = (~x)&mask;
            else if (i.operation == Operation::Not) value.bits = !x;
            else continue;
        } else if (i.opcode == Opcode::Compare && b.known) {
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
            default: continue;
            }
        } else if (i.opcode == Opcode::Binary && b.known) {
            switch (i.operation) {
            case Operation::Add: value.bits=x+y; break;
            case Operation::Sub: value.bits=x-y; break;
            case Operation::Mul: value.bits=x*y; break;
            case Operation::And: value.bits=x&y; break;
            case Operation::Or: value.bits=x|y; break;
            case Operation::Xor: value.bits=x^y; break;
            case Operation::Shl: if(y>=width)continue; value.bits=x<<y; break;
            case Operation::Ushr: if(y>=width)continue; value.bits=x>>y; break;
            case Operation::Shr: if(y>=width)continue; value.bits=std::uint64_t(sx>>y); break;
            case Operation::Udiv: if(!y)continue; value.bits=x/y; break;
            case Operation::Umod: if(!y)continue; value.bits=x%y; break;
            case Operation::Div: case Operation::Mod:
                if(!sy || (sx==std::numeric_limits<std::int64_t>::min() && sy==-1))continue;
                value.bits=i.operation==Operation::Div?sx/sy:sx%sy; break;
            default: continue;
            }
            value.bits &= mask;
        } else continue;
        result.values[i.destination.index-result.first] = value;
    }
    return result;
}
} }
