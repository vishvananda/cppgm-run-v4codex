#include "lowering/procedural.h"
#include <limits>

namespace cppgm { namespace lowering {
Value Procedural::floating_builtin(NodeId n)
{
    auto fact = sem.expression_fact(n);
    unsigned last = fact.argument_count-1;
    Value x = converted(sem.call_arguments[fact.arguments+last], sem.conversion_fact(fact.conversions+last));
    auto binary = [&](Operation op, Value a, Value b) { return emit(Opcode::Binary, IRType::I32, {a.operand, b.operand}, op); };
    Value one(Operand::integer(1), IRType::I32);
    auto invert = [&](Value v) { return binary(Operation::Sub, one, v); };
    auto compare = [&](Operation op, Operand a, Operand b) {
        return coerce(emit(Opcode::Compare, x.ir, {a,b}, op), IRType::I32);
    };
    auto nan = [&]() { return compare(Operation::Ne, x.operand, x.operand); };
    auto infinite = [&]() {
        Operand limit = Operand::floating(std::numeric_limits<long double>::infinity());
        Value negative = emit(Opcode::Unary, x.ir, {limit}, Operation::Neg);
        Value positive_test = compare(Operation::Eq, x.operand, limit);
        Value negative_test = compare(Operation::Eq, x.operand, negative.operand);
        return binary(Operation::Or, positive_test, negative_test);
    };
    auto finite = [&]() {
        Value not_nan = invert(nan());
        Value not_infinite = invert(infinite());
        return binary(Operation::And, not_nan, not_infinite);
    };
    auto normal = [&]() {
        long double minimum = x.ir == IRType::F32 ? std::numeric_limits<float>::min() :
            x.ir == IRType::F64 ? std::numeric_limits<double>::min() : std::numeric_limits<long double>::min();
        Operand limit = Operand::floating(minimum);
        Value negative = emit(Opcode::Unary, x.ir, {limit}, Operation::Neg);
        Value positive_test = compare(Operation::Ge, x.operand, limit);
        Value negative_test = compare(Operation::Le, x.operand, negative.operand);
        Value magnitude = binary(Operation::Or, positive_test, negative_test);
        return binary(Operation::And, finite(), magnitude);
    };
    Value result;
    if (fact.form == semantic::ExpressionForm::FloatFinite) result = finite();
    else if (fact.form == semantic::ExpressionForm::FloatInfinite) result = infinite();
    else if (fact.form == semantic::ExpressionForm::FloatNormal) result = normal();
    else {
        Value is_nan = nan(), is_infinite = infinite();
        Value not_nan = invert(is_nan), not_infinite = invert(is_infinite);
        Value is_finite = binary(Operation::And, not_nan, not_infinite);
        Value is_zero = compare(Operation::Eq, x.operand, Operand::floating(0));
        Value is_normal = normal();
        Value not_zero = invert(is_zero), not_normal = invert(is_normal);
        Value is_subnormal = binary(Operation::And, is_finite, binary(Operation::And, not_zero, not_normal));
        Value predicates[] = {is_nan, is_infinite, is_normal, is_subnormal, is_zero};
        result = Value(Operand::integer(0), IRType::I32);
        for (unsigned j = 0; j < 5; ++j) {
            Value category = converted(sem.call_arguments[fact.arguments+j], sem.conversion_fact(fact.conversions+j));
            Value term = binary(Operation::Mul, category, predicates[j]);
            result = binary(Operation::Add, result, term);
        }
    }
    result.type = fact.type; return result;
}
} }
