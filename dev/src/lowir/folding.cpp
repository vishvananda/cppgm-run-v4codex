#include "lowir/folding.h"
namespace lowir_model {
namespace {
using Bits = unsigned __int128;
Bits bits(Operand a) { return (Bits(a.integer_high())<<64) | a.data.integer; }
Bits mask(unsigned width) { return width == 128 ? ~Bits(0) : (Bits(1)<<width)-1; }
bool signed_type(Type t) { return t == Type::I8 || t == Type::I16 || t == Type::I32 || t == Type::I64 || t == Type::I128; }
Operand literal(Bits n, Type t) {
    auto width = t.width(); n &= mask(width);
    if (signed_type(t) && (n & (Bits(1)<<(width-1)))) n |= ~mask(width);
    auto r = Operand::integer(std::uint64_t(n));
    r.negative_integer = signed_type(t) && (n>>127);
    if (t == Type::I128) r.integer_high(std::uint64_t(n>>64));
    return r;
}
bool arithmetic(Operation op, Bits a, Bits b, unsigned width, Bits& r) {
    auto sign = Bits(1)<<(width-1);
    switch (op) {
    case Operation::Add: r = a+b; break;
    case Operation::Sub: r = a-b; break;
    case Operation::Mul: r = a*b; break;
    case Operation::And: r = a&b; break;
    case Operation::Or: r = a|b; break;
    case Operation::Xor: r = a^b; break;
    case Operation::Shl: if (b >= width) return false; r = a<<unsigned(b); break;
    case Operation::Ushr: if (b >= width) return false; r = a>>unsigned(b); break;
    case Operation::Shr:
        if (b >= width) return false;
        if (a&sign) a |= ~mask(width);
        r = a>>unsigned(b);
        if ((a>>127) && b) r |= ~Bits(0)<<(128-unsigned(b));
        break;
    case Operation::Udiv: case Operation::Umod:
        if (!b) return false;
        r = op == Operation::Udiv ? a/b : a%b; break;
    case Operation::Div: case Operation::Mod: {
        if (!b || (a == sign && b == mask(width))) return false;
        bool an = a&sign, bn = b&sign;
        Bits x = an ? (-a)&mask(width) : a, y = bn ? (-b)&mask(width) : b;
        r = op == Operation::Div ? x/y : x%y;
        if (op == Operation::Div ? an != bn : an) r = -r;
        break;
    }
    case Operation::Eq: r = a == b; break;
    case Operation::Ne: r = a != b; break;
    case Operation::Lt: r = (a^sign) < (b^sign); break;
    case Operation::Le: r = (a^sign) <= (b^sign); break;
    case Operation::Gt: r = (a^sign) > (b^sign); break;
    case Operation::Ge: r = (a^sign) >= (b^sign); break;
    case Operation::Ult: r = a < b; break;
    case Operation::Ule: r = a <= b; break;
    case Operation::Ugt: r = a > b; break;
    case Operation::Uge: r = a >= b; break;
    default: return false;
    }
    return true;
}
}
Operand normalize_integer(Operand a, Type t) { return literal(bits(a),t); }
bool same_scalar(Operand a, Operand b) {
    if (a.kind != b.kind) return false;
    if (a.kind == Operand::Integer) return bits(a) == bits(b);
    if (a.kind == Operand::Floating) return false;
    return a.ref == b.ref;
}
bool preserves_operand_type(const Program& p, const Instruction& i, unsigned argument, Operand replacement)
{
    if (i.opcode != Opcode::Switch && i.opcode != Opcode::Call) return true;
    auto original = p.operands[i.operands.begin+argument];
    if (original.kind != Operand::Temporary) return true;
    if (replacement.kind == Operand::Temporary && original.ref == replacement.ref) return true;
    Type fallback = Type::I64;
    bool implicit = i.opcode == Opcode::Switch && !argument;
    if (i.opcode == Opcode::Call && argument) {
        const Signature* sig = i.signature ? &p.signatures[i.signature.index-1] : nullptr;
        auto callee = p.operands[i.operands.begin];
        if (callee.kind == Operand::Symbol && p.symbols[callee.ref-1].kind == Symbol::FunctionSymbol)
            sig = &p.signatures[p.functions[p.symbols[callee.ref-1].entity-1].signature.index-1];
        if (!sig) return false;
        implicit = argument > sig->parameters.count;
        if (!implicit) {
            const auto& param = p.parameters[sig->parameters.begin+argument-1];
            implicit = param.passing != PPM_DIRECT;
            fallback = param.type;
        }
    }
    if (!implicit) return true;
    Type next = replacement.kind == Operand::Temporary ? p.values[replacement.ref-1].type : fallback;
    return next == p.values[original.ref-1].type;
}
bool fold_integer(const Instruction& i, const Operand* a, Operand& out)
{
    if (i.debug_value()) return false;
    if (!i.type.integer() || !i.operands.count || a[0].kind != Operand::Integer) return false;
    Bits x = bits(a[0])&mask(i.type.width()), result = 0;
    if (i.opcode == Opcode::Const || i.opcode == Opcode::Copy) result = x;
    else if (i.opcode == Opcode::Convert) {
        if (!i.source_type.integer()) return false;
        result = bits(a[0])&mask(i.source_type.width());
        if (i.operation == Operation::Sext) {
            if (result & (Bits(1)<<(i.source_type.width()-1))) result |= ~mask(i.source_type.width());
        } else if (i.operation != Operation::Zext && i.operation != Operation::Trunc) return false;
    } else if (i.opcode == Opcode::Unary) {
        if (i.operation == Operation::Neg) result = -x;
        else if (i.operation == Operation::Bitnot) result = ~x;
        else if (i.operation == Operation::Not) result = !x;
        else if (i.operation == Operation::Bswap) {
            for (unsigned j = 0; j < i.type.bytes(); ++j) { result = (result<<8) | (x&255); x >>= 8; }
        } else return false;
    } else if ((i.opcode == Opcode::Binary || i.opcode == Opcode::Compare) && a[1].kind == Operand::Integer) {
        if (!arithmetic(i.operation,x,bits(a[1])&mask(i.type.width()),i.type.width(),result)) return false;
    } else return false;
    out = literal(result,i.result_type()); return true;
}
bool discardable(const Instruction& i)
{
    if (i.debug_value()) return false;
    switch (i.opcode) {
    case Opcode::Const: case Opcode::Copy: case Opcode::Phi: case Opcode::Addr: case Opcode::Index: return true;
    case Opcode::Unary: case Opcode::Compare: return i.type.integer();
    case Opcode::Convert: return i.type.integer() && i.source_type.integer();
    case Opcode::Binary:
        return i.type.integer() && i.operation != Operation::Div && i.operation != Operation::Udiv &&
            i.operation != Operation::Mod && i.operation != Operation::Umod;
    default: return false;
    }
}
}
