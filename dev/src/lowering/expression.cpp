#include "lowering/procedural.h"
#include <cstring>
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
Value Procedural::expression(NodeId n, bool location)
{
    if (!n) throw std::logic_error("missing expression node");
    auto fact = sem.expression_fact(n);
    NodeId a = ast[n].first;
    if (fact.form == semantic::ExpressionForm::Cast) {
        NodeId operand = ast[n].kind == Kind::Cast ? ast[a].next : ast[ast[a].next].first;
        if (!operand) return Value(type(fact.type).floating() ? Operand::floating(0) : Operand::integer(0), type(fact.type), fact.type);
        TypeId target = sem.facts[n].type;
        if (reference(target)) {
            Value v = expression(operand, true);
            if (!v.address) {
                SlotId slot = builder->add_slot(0, v.ir);
                emit(Opcode::Store, v.ir, {v.operand, Operand::slot(slot)});
                v.operand = Operand::slot(slot); v.address = true;
            }
            v.type = fact.type; return v;
        }
        if (sem.conversion_fact(fact.conversions).kind == semantic::Conversion::Kind::Discarded) {
            discard(operand); return Value(Operand(), IRType(), fact.type);
        }
        Value v = converted(operand, sem.conversion_fact(fact.conversions));
        v.type = fact.type; return v;
    }
    if (fact.form == semantic::ExpressionForm::ConstantQuery || ast[n].kind == Kind::Sizeof || ast[n].kind == Kind::TypeTrait) {
        auto c = sem.constant_fact(n);
        if (!c.valid) throw std::logic_error("missing semantic constant");
        Value v = emit(Opcode::Const, type(c.type), {Operand::integer(c.bits)}); v.type = c.type; return v;
    }
    switch (ast[n].kind) {
    case Kind::Literal: {
        auto lit = ast.literals[ast[n].literal];
        if (lit.kind == LiteralKind::string) return Value(Operand::symbol(strings[n]), IRType::Ptr, fact.type, true);
        Operand o;
        if (lit.type == FT_FLOAT) { float v; std::memcpy(&v, lit.scalar.data(), sizeof(v)); o = Operand::floating(v); }
        else if (lit.type == FT_DOUBLE) { double v; std::memcpy(&v, lit.scalar.data(), sizeof(v)); o = Operand::floating(v); }
        else if (lit.type == FT_LONG_DOUBLE) { long double v; std::memcpy(&v, lit.scalar.data(), sizeof(v)); o = Operand::floating(v); }
        else { std::uint64_t v = 0; std::memcpy(&v, lit.scalar.data(), fundamental_width(lit.type)); o = Operand::integer(v); }
        return Value(o, type(fact.type), fact.type);
    }
    case Kind::KeywordLiteral:
        if (ast[n].op == KW_THIS) {
            Value v = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)}); v.type = fact.type; return v;
        }
        return Value(ast[n].op == KW_NULLPTR ? Operand::null() : Operand::integer(ast[n].op == KW_TRUE),
        ast[n].op == KW_NULLPTR ? IRType::Ptr : IRType::I64, fact.type);
    case Kind::IdExpression:
        if (!location && sem.constant_fact(n).valid && sem.entities[fact.entity].constant.valid) {
            auto c = sem.constant_fact(n); return Value(Operand::integer(c.bits), type(fact.type), fact.type);
        }
        if (sem.entities[fact.entity].kind == semantic::EntityKind::Enumerator) {
            auto c = sem.entities[fact.entity].constant;
            return Value(Operand::integer(c.bits), type(fact.type), fact.type);
        }
        if (sem.nonstatic_field(fact.entity)) {
            Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
            Value v = field(base, fact.entity, sem.object_fact(n).adjustment); v.type = fact.type; return v;
        }
        return binding(fact.entity);
    case Kind::Parenthesized: return expression(a, location);
    case Kind::Unary: case Kind::Postfix: return unary(n);
    case Kind::Binary: case Kind::Assignment: return binary(n, location);
    case Kind::Conditional: return conditional(n, location);
    case Kind::Call: return call(n);
    case Kind::Subscript: {
        Value left = converted(a, sem.conversion_fact(fact.conversions));
        Value right = converted(ast[a].next, sem.conversion_fact(fact.conversions+1));
        if (left.ir != IRType::Ptr) std::swap(left, right);
        IRType element = type(fact.type);
        if (element.kind() == IRType::Object) {
            right = coerce(right, IRType::I64, sem.unsigned_type(right.type));
            right = emit(Opcode::Binary, IRType::I64, {right.operand, Operand::integer(sem.object_size(fact.type))}, Operation::Mul);
            element = IRType::I8;
        }
        Instruction i(Opcode::Index, element); i.projection = ir_model::IPK_ARRAY_ELEMENT;
        Value v = emit(i, {left.operand, right.operand});
        v.type = fact.type; v.address = true; return v;
    }
    case Kind::Member: {
        auto member = sem.entities[fact.entity];
        if (member.kind == semantic::EntityKind::Enumerator || (member.is_static && member.constant.valid && !location)) {
            discard(a, false);
            return Value(Operand::integer(member.constant.bits), type(fact.type), fact.type);
        }
        if (member.is_static) { discard(a, false); return binding(fact.entity); }
        Value base = ast[n].op == OP_ARROW ? load(expression(a)) : address(expression(a, true));
        Value v = field(base, fact.entity, sem.object_fact(n).adjustment); v.type = fact.type; return v;
    }
    case Kind::BracedInit: case Kind::ParenInitializer: case Kind::Initializer:
        if (a) return expression(a, location);
        return Value(type(fact.type).floating() ? Operand::floating(0) : Operand::integer(0), type(fact.type), fact.type);
    default: throw std::runtime_error(std::string("unsupported lowering expression: ") + syntax::kind_name(ast[n].kind));
    }
}
Value Procedural::unary(NodeId n)
{
    NodeId a = ast[n].first;
    auto fact = sem.expression_fact(n);
    ETokenType op = ast[n].op;
    if (op == OP_AMP) { Value v = address(expression(a, true)); v.type = fact.type; return v; }
    if (op == OP_STAR) {
        Value v = converted(a, sem.conversion_fact(fact.conversions));
        v.type = fact.type; v.address = true; return v;
    }
    if (op == OP_INC || op == OP_DEC) {
        Value dest = expression(a, true), old = load(dest);
        TypeId promoted = sem.conversion_fact(fact.conversions).target;
        Value value;
        if (sem.types[dest.type].kind == TypeKind::Fundamental && sem.types[dest.type].fundamental == FT_BOOL)
            value = Value(Operand::integer(1), IRType::U8, dest.type);
        else value = operation(op == OP_INC ? OP_PLUS : OP_MINUS, convert(old, promoted),
            Value(Operand::integer(1), IRType::I32, sem.types.fundamental(FT_INT)), promoted);
        value = convert(value, dest.type); store(value, dest);
        if (ast[n].kind == Kind::Postfix) return old;
        dest.cached = true; dest.stored = value.operand; return dest;
    }
    Value v = op == OP_LNOT ? load(expression(a)) : converted(a, sem.conversion_fact(fact.conversions));
    if (op == OP_LNOT) v = emit(Opcode::Compare, v.ir, {v.operand, v.ir.floating() ? Operand::floating(0) : Operand::integer(0)}, Operation::Eq);
    else if (op != OP_PLUS) v = emit(Opcode::Unary, v.ir, {v.operand}, op == OP_MINUS ? Operation::Neg : Operation::Bitnot);
    v.type = fact.type; return v;
}
Value Procedural::binary(NodeId n, bool location)
{
    NodeId a = ast[n].first, b = ast[a].next;
    auto fact = sem.expression_fact(n);
    ETokenType op = ast[n].op;
    if (op == OP_COMMA) { discard(a); return expression(b, location); }
    if (op == OP_LAND || op == OP_LOR) return logical(n);
    if (ast[n].kind == Kind::Assignment) {
        Value rhs, dest, lhs;
        if (op == OP_ASS) { rhs = converted(b, sem.conversion_fact(fact.conversions+1)); dest = expression(a, true); }
        else {
            NodeId target = a;
            while (ast[target].kind == Kind::Parenthesized) target = ast[target].first;
            EntityId object = sem.expression_fact(target).entity;
            bool direct = ast[target].kind == Kind::IdExpression && object && !reference(sem.entities[object].type);
            // A direct object needs no address evaluation; its read follows
            // the binary-operand convention. Indirection/calls must evaluate
            // the RHS before computing the LHS address, exactly once.
            if (direct) { dest = expression(a, true); lhs = load(dest); }
            rhs = converted(b, sem.conversion_fact(fact.conversions+1));
            if (!direct) { dest = expression(a, true); lhs = load(dest); }
        }
        if (op != OP_ASS) {
            ETokenType binary = OP_PLUS;
            switch (op) {
            case OP_PLUSASS: binary = OP_PLUS; break;
            case OP_MINUSASS: binary = OP_MINUS; break;
            case OP_STARASS: binary = OP_STAR; break;
            case OP_DIVASS: binary = OP_DIV; break;
            case OP_MODASS: binary = OP_MOD; break;
            case OP_BANDASS: binary = OP_AMP; break;
            case OP_BORASS: binary = OP_BOR; break;
            case OP_XORASS: binary = OP_XOR; break;
            case OP_LSHIFTASS: binary = OP_LSHIFT; break;
            case OP_RSHIFTASS: binary = OP_RSHIFT; break;
            default: break;
            }
            TypeId common = sem.conversion_fact(fact.conversions).target;
            lhs = convert(lhs, common);
            rhs = operation(binary, lhs, rhs, common);
            rhs = convert(rhs, fact.type);
        }
        store(rhs, dest); dest.cached = true; dest.stored = rhs.operand; return dest;
    }
    Value lhs = load(expression(a));
    Value rhs = load(expression(b));
    lhs = convert(lhs, sem.conversion_fact(fact.conversions).target);
    rhs = convert(rhs, sem.conversion_fact(fact.conversions+1).target);
    return operation(op, lhs, rhs, fact.type);
}
Value Procedural::operation(ETokenType op, Value a, Value b, TypeId result)
{
    Value v;
    if ((op == OP_PLUS || op == OP_MINUS) && (a.ir == IRType::Ptr || b.ir == IRType::Ptr)) {
        if (a.ir != IRType::Ptr) std::swap(a, b);
        TypeId pointer_type = a.type;
        auto pt = sem.types[pointer_type];
        std::uint64_t scale = sem.object_size(pt.child);
        if (b.ir == IRType::Ptr) {
            v = emit(Opcode::Binary, IRType::Ptr, {a.operand, b.operand}, Operation::Sub);
            if (scale > 1) {
                unsigned shift = 0; while ((std::uint64_t(1) << shift) < scale) ++shift;
                bool power = (scale & (scale-1)) == 0;
                v = emit(Opcode::Binary, IRType::I64, {v.operand, Operand::integer(power ? shift : scale)}, power ? Operation::Shr : Operation::Div);
            }
        } else {
            b = coerce(b, IRType::I64, sem.unsigned_type(b.type), false, true);
            if (scale > 1) b = emit(Opcode::Binary, IRType::I64, {b.operand, Operand::integer(scale)}, Operation::Mul);
            if (op == OP_MINUS) b = emit(Opcode::Binary, IRType::I64, {Operand::integer(0), b.operand}, Operation::Sub);
            v = emit(Opcode::Index, IRType::I8, {a.operand, b.operand});
        }
        v.type = result; return v;
    }
    bool unsign = a.type && sem.unsigned_type(a.type);
    Operation action = Operation::None;
    bool compare = false;
    switch (op) {
    case OP_PLUS: action = Operation::Add; break;
    case OP_MINUS: action = Operation::Sub; break;
    case OP_STAR: action = Operation::Mul; break;
    case OP_DIV: action = unsign ? Operation::Udiv : Operation::Div; break;
    case OP_MOD: action = unsign ? Operation::Umod : Operation::Mod; break;
    case OP_AMP: action = Operation::And; break;
    case OP_BOR: action = Operation::Or; break;
    case OP_XOR: action = Operation::Xor; break;
    case OP_LSHIFT: action = Operation::Shl; break;
    case OP_RSHIFT: action = unsign ? Operation::Ushr : Operation::Shr; break;
    case OP_EQ: action = Operation::Eq; compare = true; break;
    case OP_NE: action = Operation::Ne; compare = true; break;
    case OP_LT: action = unsign ? Operation::Ult : Operation::Lt; compare = true; break;
    case OP_LE: action = unsign ? Operation::Ule : Operation::Le; compare = true; break;
    case OP_GT: action = unsign ? Operation::Ugt : Operation::Gt; compare = true; break;
    case OP_GE: action = unsign ? Operation::Uge : Operation::Ge; compare = true; break;
    default: throw std::logic_error("missing binary lowering operation");
    }
    v = emit(compare ? Opcode::Compare : Opcode::Binary, a.ir, {a.operand, b.operand}, action);
    v.type = result; return v;
}
Value Procedural::call(NodeId n)
{
    auto fact = sem.expression_fact(n);
    if (fact.form == semantic::ExpressionForm::Unreachable) return emit(Opcode::Unreachable, IRType(), {});
    std::size_t begin = call_work.size();
    call_work.push_back(Operand());
    auto object_use = sem.object_fact(n);
    if (object_use.type) {
        Value object;
        if (object_use.node) {
            object = expression(object_use.node, true);
            object = sem.types[sem.expression_fact(object_use.node).type].kind == TypeKind::Pointer ? load(object) : address(object);
        } else object = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
        call_work.push_back(base_projection(object, sem.object_fact(n).adjustment).operand);
    } else if (object_use.node) {
        Value object = expression(object_use.node);
        if (sem.types[object.type].kind == TypeKind::Pointer) load(object);
    }
    // The course's indirect-call fixtures evaluate arguments before fetching
    // the callee; C++11 leaves their relative evaluation order unspecified.
    for (unsigned j = 0; j < fact.argument_count; ++j)
        call_work.push_back(converted(sem.call_arguments[fact.arguments+j], sem.conversion_fact(fact.conversions+j)).operand);
    NodeId callee = ast[n].first;
    EntityId selected = sem.facts[n].entity;
    Instruction i(Opcode::Call, type(sem.facts[n].type));
    if (selected) call_work[begin] = Operand::symbol(symbol(selected));
    else {
        Value fn = load(expression(callee)); call_work[begin] = fn.operand;
        TypeId ft = sem.expression_fact(callee).type;
        if (sem.types[ft].kind == TypeKind::Pointer) ft = sem.types[ft].child;
        i.signature = signature(ft);
    }
    Value v = emit(i, call_work.data()+begin, call_work.size()-begin); call_work.resize(begin); v.type = fact.type;
    if (reference(sem.facts[n].type)) v.address = true;
    return v;
}
} }
