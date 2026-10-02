#include "lowir/folding.h"
#include <cstring>
#include <limits>
namespace lowir_model {
namespace {
using cppgm::ExtendedFloat;
ExtendedFloat rounded(ExtendedFloat x, Type t) {
    if (t == Type::F16) return cppgm::half_value(cppgm::half_bits(x));
    if (t == Type::F32) return static_cast<float>(x);
    if (t == Type::F64) return static_cast<double>(x);
    if (t == Type::F80) return static_cast<long double>(x);
    return x;
}
bool literal(const Program& p, Operand a, Type t, ExtendedFloat& x) {
    if (a.kind != Operand::Floating || a.signaling_nan) return false;
    x = a.extended_floating ? a.data.extended : ExtendedFloat(a.data.floating);
    if (a.ref && t != Type::F80) {
        const auto& r = p.floating_literals[a.ref-1];
        if (t == Type::F32) { float f; std::memcpy(&f,&r.f32,4); x = f; }
        else if (t == Type::F64) { double f; std::memcpy(&f,&r.f64,8); x = f; }
        else x = t == Type::F16 ? cppgm::half_value(r.f16) : r.f128;
    } else x = rounded(x,t);
    // Leave NaN payloads, signaling and target denormal exception behavior to
    // execution. In particular, f80 loads themselves can quiet an sNaN.
    return !cppgm::nan_float(x);
}
bool ordinary(ExtendedFloat x, Type t) {
    if (!x || !cppgm::finite_float(x)) return true;
    ExtendedFloat minimum = t == Type::F16 ? cppgm::half_value(1024) :
        t == Type::F32 ? ExtendedFloat(std::numeric_limits<float>::min()) :
        t == Type::F64 ? ExtendedFloat(std::numeric_limits<double>::min()) :
        t == Type::F80 ? ExtendedFloat(std::numeric_limits<long double>::min()) : 0;
    return (x < 0 ? -x : x) >= minimum;
}
}
bool fold_floating(const Program& p, const Instruction& i, const Operand* a, Operand& out)
{
    if (!i.type.floating() || !i.operands.count) return false;
    ExtendedFloat x;
    if (i.opcode == Opcode::Convert) {
        if (i.source_type.integer() && a[0].kind == Operand::Integer &&
            (i.operation == Operation::Sitofp || i.operation == Operation::Uitofp)) {
            unsigned width = i.source_type.width();
            using Bits = unsigned __int128;
            Bits mask = width == 128 ? ~Bits(0) : (Bits(1)<<width)-1;
            Bits bits = ((Bits(a[0].integer_high())<<64)|a[0].data.integer)&mask;
            bool negative = i.operation == Operation::Sitofp && (bits&(Bits(1)<<(width-1)));
            Bits magnitude = negative ? (-bits)&mask : bits;
            unsigned precision = i.type == Type::F16 ? 11 : i.type == Type::F32 ? 24 :
                i.type == Type::F64 ? 53 : i.type == Type::F80 ? 64 : 113;
            // Exact integers need no rounding-mode assumption and raise no
            // inexact exception. Larger magnitudes conservatively execute.
            if (magnitude > (Bits(1)<<precision)) return false;
            x = ExtendedFloat(magnitude); if (negative) x = -x;
            if (rounded(x,i.type) != x) return false;
        } else if (i.source_type.floating() &&
            (i.operation == Operation::Fpext || i.operation == Operation::Fptrunc)) {
            if (!literal(p,a[0],i.source_type,x) || !ordinary(x,i.source_type) ||
                rounded(x,i.type) != x || !ordinary(x,i.type)) return false;
        } else return false;
    } else {
        if (!literal(p,a[0],i.type,x)) return false;
        if (i.opcode == Opcode::Compare && i.operands.count == 2) {
            ExtendedFloat y;
            if (!literal(p,a[1],i.type,y) || !ordinary(x,i.type) || !ordinary(y,i.type)) return false;
            bool result;
            switch (i.operation) {
            case Operation::Eq: result = x == y; break;
            case Operation::Ne: result = x != y; break;
            case Operation::Lt: case Operation::Ult: result = x < y; break;
            case Operation::Le: case Operation::Ule: result = x <= y; break;
            case Operation::Gt: case Operation::Ugt: result = x > y; break;
            case Operation::Ge: case Operation::Uge: result = x >= y; break;
            default: return false;
            }
            out = Operand::integer(result); return true;
        }
        if (i.opcode == Opcode::Unary && i.operation == Operation::Neg && ordinary(x,i.type)) x = -x;
        else if (i.opcode != Opcode::Const && i.opcode != Opcode::Copy) return false;
    }
    out = i.type == Type::F128 ? Operand::extended(x) : Operand::floating(static_cast<long double>(x));
    return true;
}
}
