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
    if (fact.form == semantic::ExpressionForm::OperatorCall) return call(n);
    if (fact.form >= semantic::ExpressionForm::FloatFinite && fact.form <= semantic::ExpressionForm::FloatClassify) return floating_builtin(n);
    if (fact.form == semantic::ExpressionForm::LiteralCall) {
        Value string = emit(Opcode::Addr, IRType(), {Operand::symbol(strings[n])});
        auto lit = ast.literals[ast[n].literal];
        Instruction widen(Opcode::Convert, IRType::I64); widen.source_type = IRType::I32; widen.operation = Operation::Sext;
        Value length = emit(widen, {Operand::integer(lit.elements-1)});
        Operand arguments[] = {Operand::symbol(symbol(sem.facts[n].entity)), string.operand, length.operand};
        Value v = guarded_call(Instruction(Opcode::Call, type(fact.type)), arguments, 3); v.type = fact.type; return v;
    }
    if (fact.form == semantic::ExpressionForm::Construction) {
        EntityId e = sem.object_fact(n).temporary;
        Value pointer = class_address(e,fact.type);
        construct(sem.facts[n].entity, n, pointer); activate_temporary(e);
        pointer.type = fact.type; pointer.address = true; return pointer;
    }
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
            if (sem.conversion_fact(fact.conversions).derived) {
                v = base_projection(address(v), 1); v.address = true;
            }
            v.type = fact.type; return v;
        }
        if (sem.conversion_fact(fact.conversions).kind == semantic::Conversion::Kind::Discarded) {
            NodeId direct = operand;
            while (ast[direct].kind == Kind::Parenthesized) direct = ast[direct].first;
            auto discarded = sem.expression_fact(direct);
            if (ast[direct].kind == Kind::IdExpression && discarded.category != ValueCategory::Prvalue &&
                !(sem.types[discarded.type].cv & 2) &&
                (sem.entities[discarded.entity].kind == semantic::EntityKind::Parameter ||
                 (sem.types[discarded.type].kind == TypeKind::Named && sem.entities[sem.types[discarded.type].entity].class_info))) {
                Value v = expression(operand, true);
                if (type(discarded.type).kind() == IRType::Object) address(v); else load(v);
                return Value(Operand(), IRType(), fact.type);
            }
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
    case Kind::New: return placement_new(n);
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
            if (sem.injected_storage(fact.entity)) { Value v = binding(fact.entity); v.type = fact.type; return v; }
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
        if (dest.bit_field && ast[n].kind != Kind::Postfix && ast[a].kind == Kind::Member && ast[a].op == OP_DOT) {
            NodeId object = ast[a].first;
            EntityId root = sem.expression_fact(object).entity;
            // Reacquire a prefix result's stable named storage using recorded
            // identities. Calls, pointers and reference objects are never replayed.
            if (ast[object].kind == Kind::IdExpression && root && !sem.nonstatic_field(root) && !reference(sem.entities[root].type)) {
                dest = field(address(binding(root)), dest.bit_field, sem.object_fact(a).adjustment);
                dest.type = sem.expression_fact(a).type;
            }
        }
        if (!dest.bit_field) value = convert(value, dest.type);
        value = store(value, dest);
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
        if (op == OP_ASS && sem.field_fact(sem.expression_fact(a).entity).bit_field) {
            rhs = load(expression(b)); dest = expression(a, true);
        } else if (op == OP_ASS) { rhs = converted(b, sem.conversion_fact(fact.conversions+1)); dest = expression(a, true); }
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
        rhs = store(rhs, dest); dest.cached = true; dest.stored = rhs.operand; return dest;
    }
    Value lhs = load(expression(a));
    Value rhs = load(expression(b));
    lhs = convert(lhs, sem.conversion_fact(fact.conversions).target, sem.conversion_fact(fact.conversions).fold_widen);
    rhs = convert(rhs, sem.conversion_fact(fact.conversions+1).target, sem.conversion_fact(fact.conversions+1).fold_widen);
    if (sem.conversion_fact(fact.conversions).derived) lhs = base_projection(lhs, 1);
    if (sem.conversion_fact(fact.conversions+1).derived) rhs = base_projection(rhs, 1);
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
Value Procedural::call(NodeId n, Value destination)
{
    auto fact = sem.expression_fact(n);
    if (fact.form == semantic::ExpressionForm::PseudoDestructor) {
        NodeId member = ast[n].first;
        while (ast[member].kind == Kind::Parenthesized) member = ast[member].first;
        Value object = expression(ast[member].first);
        if (ast[member].op == OP_ARROW) load(object);
        return Value(Operand(), IRType::Void, fact.type);
    }
    if (fact.form == semantic::ExpressionForm::Unreachable) return emit(Opcode::Unreachable, IRType(), {});
    bool class_result = sem.class_value(sem.facts[n].type);
    bool indirect_result = sem.indirect_value(sem.facts[n].type);
    bool own_result = class_result && destination.ir == IRType();
    if (own_result) destination = class_address(sem.object_fact(n).temporary,fact.type);
    std::size_t begin = call_work.size();
    call_work.push_back(Operand());
    if (indirect_result) call_work.push_back(destination.operand);
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
    for (unsigned j = 0; j < fact.argument_count; ++j) {
        NodeId argument = sem.call_arguments[fact.arguments+j];
        call_work.push_back(argument ? converted(argument, sem.conversion_fact(fact.conversions+j)).operand : Operand::integer(0));
    }
    NodeId callee = ast[n].first;
    EntityId selected = sem.facts[n].entity;
    Instruction i(Opcode::Call, indirect_result ? IRType(IRType::Void) : type(sem.facts[n].type));
    if (selected) call_work[begin] = Operand::symbol(symbol(selected));
    else {
        Value fn = load(expression(callee)); call_work[begin] = fn.operand;
        TypeId ft = sem.expression_fact(callee).type;
        if (sem.types[ft].kind == TypeKind::Pointer) ft = sem.types[ft].child;
        i.signature = signature(ft);
    }
    Value v = guarded_call(i, call_work.data()+begin, call_work.size()-begin); call_work.resize(begin); v.type = fact.type;
    if (class_result) {
        if (!indirect_result && !sem.empty_class(fact.type)) {
            Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(fact.type); copy.alignment = sem.object_alignment(fact.type);
            emit(copy,{v.operand,destination.operand});
        }
        if (own_result) activate_temporary(sem.object_fact(n).temporary);
        destination.type = fact.type; destination.address = true; return destination;
    }
    if (reference(sem.facts[n].type)) v.address = true;
    return v;
}
} }
