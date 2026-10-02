#include "lowir/loop_simplify.h"
#include "lowir/folding.h"
namespace lowir_model {
bool constant_trip(Type type, Operation op, Operand start, Operand limit, Operand step,
    bool subtract, std::uint64_t& trips, Operand& final)
{
    if (!type.integer() || type.width() > 64 || start.kind != Operand::Integer ||
        limit.kind != Operand::Integer || step.kind != Operand::Integer) return false;
    using Wide = __int128;
    unsigned width = type.width();
    Wide modulus = Wide(1)<<width, sign = modulus/2;
    bool is_unsigned = op == Operation::Ult || op == Operation::Ule || op == Operation::Ugt || op == Operation::Uge;
    if (op == Operation::Eq || op == Operation::Ne)
        is_unsigned = type == Type::U8 || type == Type::U16 || type == Type::U32 || type == Type::I1;
    auto number = [&](Operand a, bool unsign) {
        Wide n = a.data.integer & std::uint64_t(modulus-1);
        return !unsign && n >= sign ? n-modulus : n;
    };
    Wide a = number(start,is_unsigned), b = number(limit,is_unsigned);
    Wide delta = number(step,false); if (subtract) delta = -delta;
    Wide count = 0;
    switch (op) {
    case Operation::Lt: case Operation::Ult:
        if (a < b) { if (delta <= 0) return false; count = (b-a+delta-1)/delta; } break;
    case Operation::Le: case Operation::Ule:
        if (a <= b) { if (delta <= 0) return false; count = (b-a)/delta+1; } break;
    case Operation::Gt: case Operation::Ugt:
        if (a > b) { if (delta >= 0) return false; count = (a-b-delta-1)/(-delta); } break;
    case Operation::Ge: case Operation::Uge:
        if (a >= b) { if (delta >= 0) return false; count = (a-b)/(-delta)+1; } break;
    case Operation::Ne:
        if (a != b) {
            if (!delta || (b-a)%delta || (b-a)/delta < 0) return false;
            count = (b-a)/delta;
        } break;
    case Operation::Eq:
        if (a == b) { if (!delta) return false; count = 1; } break;
    default: return false;
    }
    Wide end = a+count*delta;
    if (end < (is_unsigned ? 0 : -sign) || end > (is_unsigned ? modulus-1 : sign-1) ||
        count > Wide(~std::uint64_t(0))) return false;
    trips = std::uint64_t(count);
    final = normalize_integer(Operand::integer(std::uint64_t(end)),type);
    return true;
}
}
