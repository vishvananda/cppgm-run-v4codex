#include "lowering/procedural.h"
#include "support/type_traits.h"
#include <cstring>
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
Value Procedural::expression(NodeId n, bool location)
{
    SourceInvocationScope invocation(source_invocation,sem.source_site(n));
    if (!n) throw std::logic_error("missing expression node");
    guard_expression(n);
    auto node = ast[n];
    auto fact = sem.expression_fact(n);
    if (node.kind == Kind::StatementExpression) return statement_expression(n);
    NodeId a = node.first;
    if (node.kind == Kind::FunctionName) return Value(Operand::symbol(symbol(fact.entity)),IRType::Ptr,fact.type,true);
    if (node.kind == Kind::VaArg) {
        auto list = converted(a,sem.conversion_fact(fact.conversions));
        return emit(Opcode::VaArg,type(fact.type),{list.operand});
    }
    if (fact.form == semantic::ExpressionForm::Typeid || fact.form == semantic::ExpressionForm::DynamicCast)
        return rtti_expression(n);
    if (fact.form == semantic::ExpressionForm::TypeinfoEqual || fact.form == semantic::ExpressionForm::TypeinfoUnequal) {
        auto receiver = sem.object_fact(n);
        Value left;
        if (receiver.node) {
            if (receiver.arrow) left = arrow_object(receiver.node,receiver.arrow);
            else {
                left = expression(receiver.node,true);
                left = sem.types[sem.expression_fact(receiver.node).type].kind == TypeKind::Pointer ? load(left) : address(left);
            }
        } else left = receiver.capture ? captured_address(receiver.capture) : implicit_object();
        left = base_projection(left,receiver.qualifier_adjustment);
        left = base_projection(left,receiver.adjustment);
        auto right = converted(sem.call_argument(fact),sem.conversion_fact(fact.conversions));
        // Direct queries in one TU share the canonical RTTI object. General
        // references may designate TU-local incomplete placeholders, whose
        // external name identities remain equal to the complete type's name.
        if (sem.expression_fact(receiver.node).form != semantic::ExpressionForm::Typeid ||
            sem.expression_fact(sem.call_argument(fact)).form != semantic::ExpressionForm::Typeid) {
            auto a = emit(Opcode::Index,IRType::I8,{left.operand,Operand::integer(8)});
            auto b = emit(Opcode::Index,IRType::I8,{right.operand,Operand::integer(8)});
            left = emit(Opcode::Load,IRType::Ptr,{a.operand});
            right = emit(Opcode::Load,IRType::Ptr,{b.operand});
        }
        auto value = emit(Opcode::Compare,IRType::Ptr,{left.operand,right.operand},
            fact.form == semantic::ExpressionForm::TypeinfoEqual ? Operation::Eq : Operation::Ne);
        value.type = fact.type; return value;
    }
    if (fact.form == semantic::ExpressionForm::ListValue) {
        auto c = sem.conversion_fact(fact.conversions);
        EntityId e = sem.list_objects[c.materialization].temporary;
        Value pointer = class_address(e,fact.type);
        list_conversion(c,pointer); activate_temporary(e);
        pointer.type = fact.type; pointer.address = true; return pointer;
    }
    if (fact.form == semantic::ExpressionForm::OperatorCall) return call(n);
    if (fact.form >= semantic::ExpressionForm::FloatFinite && fact.form <= semantic::ExpressionForm::FloatClassify) return floating_builtin(n);
    if (fact.form == semantic::ExpressionForm::LiteralCall) return call(n);
    if (fact.form == semantic::ExpressionForm::Construction) {
        EntityId e = sem.object_fact(n).temporary;
        Value pointer = class_address(e,fact.type);
        construct(sem.facts[n].entity, n, pointer); activate_temporary(e);
        pointer.type = fact.type; pointer.address = true; return pointer;
    }
    if (fact.form == semantic::ExpressionForm::Cast) {
        NodeId operand = node.kind == Kind::Cast ? ast[a].next : ast[ast[a].next].first;
        if (!operand) return initialization_value(0,fact.type);
        TypeId target = sem.facts[n].type;
        if (reference(target)) {
            auto c = sem.conversion_fact(fact.conversions);
            if (c.kind == semantic::Conversion::Kind::User || c.kind == semantic::Conversion::Kind::Construction || c.temporary) {
                Value v = converted(operand,c); v.address = true; v.type = fact.type; return v;
            }
            Value v = expression(operand, true);
            if (!v.address) {
                SlotId slot = builder->add_slot(0, v.ir);
                emit(Opcode::Store, v.ir, {v.operand, Operand::slot(slot)});
                v.operand = Operand::slot(slot); v.address = true;
            }
            if (sem.conversion_fact(fact.conversions).derived) {
                v = base_projection(address(v), sem.conversion_fact(fact.conversions).adjustment); v.address = true;
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
                 sem.entities[discarded.entity].kind == semantic::EntityKind::Variable ||
                 (sem.types[discarded.type].kind == TypeKind::Named && sem.entities[sem.types[discarded.type].entity].class_info))) {
                Value v = expression(operand, true);
                if (type(discarded.type).kind() == IRType::Object) address(v); else load(v);
                return Value(Operand(), IRType(), fact.type);
            }
            discard(operand); return Value(Operand(), IRType(), fact.type);
        }
        auto c = sem.conversion_fact(fact.conversions);
        if (sem.class_value(fact.type) && c.kind == semantic::Conversion::Kind::User) {
            EntityId object = sem.user_conversions[c.materialization].temporary;
            Value destination = class_address(object,fact.type);
            user_conversion(operand,c,destination); activate_temporary(object);
            destination.type = fact.type; destination.address = true; return destination;
        }
        Value v = converted(operand, c);
        v.type = fact.type; return v;
    }
    if (node.kind == Kind::TypeTrait && BuiltinTrait(node.flags) == BuiltinTrait::Offsetof && !sem.constant_fact(n).valid) return offsetof_expression(n);
    if (fact.form == semantic::ExpressionForm::ConstantQuery || node.kind == Kind::Sizeof || node.kind == Kind::SizeofPack || node.kind == Kind::TypeTrait) {
        auto c = sem.constant_fact(n);
        if (!c.valid) throw std::logic_error("missing semantic constant");
        if (type(fact.type) == IRType::Void) return Value();
        if (node.op == KW_NOEXCEPT) return Value(integer_operand(c),type(c.type),c.type);
        if (type(c.type).floating()) {
            auto v = floating_literal(c.type,sem.floating_value(c),sem.floating_signaling(c));
            if (v.operand.literal()) { v = emit(Opcode::Const,type(c.type),{v.operand}); v.type = c.type; }
            return v;
        }
        Value v = emit(Opcode::Const,type(c.type),{integer_operand(c)}); v.type = c.type; return v;
    }
    switch (node.kind) {
    case Kind::Throw: return throw_expression(n);
    case Kind::Lambda: {
        auto value = class_address(sem.object_fact(n).temporary,fact.type);
        initialize_closure(n,value);
        value.type = fact.type; value.address = true; return value;
    }
    case Kind::New: return placement_new(n);
    case Kind::Delete: return delete_expression(n);
    case Kind::Literal: {
        auto lit = ast.literals[node.literal];
        if (lit.kind == LiteralKind::string) { string_literal(n); return Value(Operand::symbol(strings[n]), IRType::Ptr, fact.type, true); }
        Operand o;
        if (lit.type == FT_FLOAT) { float v; std::memcpy(&v, lit.scalar.data(), sizeof(v)); o = Operand::floating(v); }
        else if (lit.type == FT_DOUBLE) { double v; std::memcpy(&v, lit.scalar.data(), sizeof(v)); o = Operand::floating(v); }
        else if (lit.type == FT_LONG_DOUBLE) { long double v; std::memcpy(&v, lit.scalar.data(), sizeof(v)); o = Operand::floating(v); }
        else { std::uint64_t v = 0; std::memcpy(&v, lit.scalar.data(), fundamental_width(lit.type)); o = Operand::integer(v); }
        return Value(o, type(fact.type), fact.type);
    }
    case Kind::KeywordLiteral:
        if (node.op == KW_THIS) {
            auto capture = sem.object_fact(n).capture;
            Value v = capture ? captured_address(capture) : implicit_object();
            v.type = fact.type; v.nonnull = true; return v;
        }
        return Value(node.op == KW_NULLPTR ? Operand::null() : Operand::integer(node.op == KW_TRUE),
        node.op == KW_NULLPTR ? IRType::Ptr : IRType::I64, fact.type);
    case Kind::IdExpression:
        if (auto capture = sem.object_fact(n).capture) {
            if (!sem.nonstatic_field(fact.entity)) {
                auto value = captured_address(capture); value.type = fact.type; value.address = true; return value;
            }
        }
        if (sem.entities[fact.entity].constant.valid && reference(sem.entities[fact.entity].constant.type) &&
            sem.constant_static_value(sem.entities[fact.entity].constant).kind != semantic::StaticValue::Invalid)
            return constant_operand(sem.entities[fact.entity].constant,fact.type);
        if (!location && sem.constant_fact(n).valid && sem.entities[fact.entity].constant.valid) {
            auto c = sem.constant_fact(n);
            if (type(c.type).scalar() && !sem.class_value(c.type)) return constant_operand(c,fact.type);
        }
        if (sem.entities[fact.entity].kind == semantic::EntityKind::Enumerator) {
            auto c = sem.entities[fact.entity].constant;
            return constant_operand(c,fact.type);
        }
        if (sem.nonstatic_field(fact.entity)) {
            auto storage = sem.field_projection(fact.entity,sem.object_fact(n).type).object;
            if (storage) { Value v = binding(fact.entity); v.type = fact.type; return v; }
            auto capture = sem.object_fact(n).capture;
            Value base = capture ? captured_address(capture) : implicit_object();
            // Preserve the captured receiver's object projection separately
            // from the selected member projection in the O0 LowIR view.
            if (capture) base = emit(Opcode::Index,IRType::I8,{base.operand,Operand::integer(0)});
            base = base_projection(base,sem.object_fact(n).qualifier_adjustment);
            Value v = field(base, fact.entity, sem.object_fact(n).adjustment, sem.object_fact(n).type); v.type = fact.type; return v;
        }
        return binding(fact.entity);
    case Kind::Parenthesized: return expression(a, location);
    case Kind::Fold: return fold(n,location);
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
        if (member.kind == semantic::EntityKind::Enumerator || (member.is_static && sem.constant_fact(n).valid && !location)) {
            if (sem.object_fact(n).arrow) arrow_object(a,sem.object_fact(n).arrow);
            else discard(a, false);
            return constant_operand(member.kind == semantic::EntityKind::Enumerator ? member.constant : sem.constant_fact(n),fact.type);
        }
        if (member.is_static) {
            if (sem.object_fact(n).arrow) arrow_object(a,sem.object_fact(n).arrow);
            else discard(a, false);
            return binding(fact.entity);
        }
        Value base = node.op == OP_ARROW ? arrow_object(a,sem.object_fact(n).arrow) : address(expression(a, true));
        base = base_projection(base,sem.object_fact(n).qualifier_adjustment);
        Value v = field(base, fact.entity, sem.object_fact(n).adjustment, sem.object_fact(n).type); v.type = fact.type;
        v.member_zero_adjustment = sem.member_pointer_read_zero(n); return v;
    }
    case Kind::BracedInit: case Kind::ParenInitializer: case Kind::Initializer:
        if (a) return expression(a, location);
        return Value(type(fact.type).floating() ? Operand::floating(0) : Operand::integer(0), type(fact.type), fact.type);
    default: throw std::runtime_error(std::string("unsupported lowering expression: ") + syntax::kind_name(node.kind));
    }
}
Value Procedural::unary(NodeId n)
{
    auto node = ast[n];
    NodeId a = node.first;
    auto fact = sem.expression_fact(n);
    ETokenType op = node.op;
    if (op == OP_AMP) {
        if (sem.types[fact.type].kind == TypeKind::MemberPointer) return member_pointer_value(sem.expression_fact(a).entity,fact.type);
        Value v = address(expression(a, true)); v.type = fact.type; return v;
    }
    if (op == OP_STAR) {
        Value v = converted(a, sem.conversion_fact(fact.conversions));
        v.type = fact.type; v.address = true; return v;
    }
    if (op == OP_INC || op == OP_DEC) {
        auto conversion = sem.conversion_fact(fact.conversions);
        Value dest = conversion.reference ? converted(a,conversion) : expression(a, true);
        if (conversion.reference) { dest.type = sem.types[conversion.target].child; dest.address = true; }
        if (sem.types[dest.type].cv & 4) {
            auto computation = sem.conversion_fact(fact.conversions+(conversion.reference ? fact.count-1 : 0)).target;
            return atomic_update(dest,Value(Operand::integer(1),IRType::I32,sem.types.fundamental(FT_INT)),
                op == OP_INC ? OP_PLUS : OP_MINUS,computation,node.kind == Kind::Postfix);
        }
        Value old = load(dest);
        TypeId promoted = sem.conversion_fact(fact.conversions+(conversion.reference ? fact.count-1 : 0)).target;
        Value value;
        if (sem.types[dest.type].kind == TypeKind::Fundamental && sem.types[dest.type].fundamental == FT_BOOL)
            value = Value(Operand::integer(1), IRType::U8, dest.type);
        else value = operation(op == OP_INC ? OP_PLUS : OP_MINUS, convert(old, promoted),
            Value(Operand::integer(1), IRType::I32, sem.types.fundamental(FT_INT)), promoted);
        if (dest.bit_field && node.kind != Kind::Postfix && ast[a].kind == Kind::Member && ast[a].op == OP_DOT) {
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
        if (node.kind == Kind::Postfix) return old;
        dest.cached = true; dest.stored = value.operand; return dest;
    }
    Value v = op == OP_LNOT && sem.conversion_fact(fact.conversions).kind != semantic::Conversion::Kind::User ? load(expression(a)) : converted(a, sem.conversion_fact(fact.conversions));
    if (op == OP_LNOT) v = truth_operand(v);
    if (op == OP_LNOT) v = emit(Opcode::Compare, v.ir, {v.operand, v.ir.floating() ? Operand::floating(0) : Operand::integer(0)}, Operation::Eq);
    else if (op != OP_PLUS) v = emit(Opcode::Unary, v.ir, {v.operand}, op == OP_MINUS ? Operation::Neg : Operation::Bitnot);
    v.type = fact.type; return v;
}
Value Procedural::binary(NodeId n, bool location)
{
    auto node = ast[n];
    NodeId a = node.first, b = ast[a].next;
    auto fact = sem.expression_fact(n);
    ETokenType op = node.op;
    if (op == OP_DOTSTAR || op == OP_ARROWSTAR) {
        if (sem.types[fact.type].kind == TypeKind::Function) throw std::runtime_error("bound member function requires a call");
        Value v = member_pointer_object(sem.object_fact(n)); v.type = fact.type; v.address = true; return v;
    }
    if (op == OP_COMMA) { discard(a); return expression(b, location); }
    if (op == OP_LAND || op == OP_LOR) return logical(n);
    if (node.kind == Kind::Assignment) {
        Value rhs, dest, lhs;
        if (op == OP_ASS && sem.field_fact(sem.expression_fact(a).entity).bit_field) {
            rhs = load(expression(b)); dest = expression(a, true);
        } else if (op == OP_ASS) { rhs = converted(b, sem.conversion_fact(fact.conversions+1)); dest = expression(a, true); }
        else {
            NodeId target = a;
            while (ast[target].kind == Kind::Parenthesized || ast[target].kind == Kind::Member) target = ast[target].first;
            EntityId object = sem.expression_fact(target).entity;
            bool direct = ast[target].op == KW_THIS || (ast[target].kind == Kind::IdExpression && object && !(sem.types[sem.entities[object].type].cv & 2));
            // A direct object needs no address evaluation; its read follows
            // the binary-operand convention. Indirection/calls must evaluate
            // the RHS before computing the LHS address, exactly once.
            auto destination = sem.conversion_fact(fact.conversions);
            auto address = [&]() {
                if (!destination.reference) return expression(a,true);
                auto value = converted(a,destination);
                value.type = fact.type; value.address = true; return value;
            };
            if (direct) { dest = address(); if (!(sem.types[dest.type].cv & 4)) lhs = load(dest); }
            rhs = converted(b, sem.conversion_fact(fact.conversions+1));
            if (!direct) { dest = address(); if (!(sem.types[dest.type].cv & 4)) lhs = load(dest); }
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
            TypeId common = sem.conversion_fact(fact.conversions+
                (sem.conversion_fact(fact.conversions).reference ? 2 : 0)).target;
            if (sem.types[dest.type].cv & 4) return atomic_update(dest,rhs,binary,common,false);
            lhs = convert(lhs, common);
            if (binary != OP_LSHIFT && binary != OP_RSHIFT && lhs.ir != IRType::Ptr && rhs.ir != IRType::Ptr)
                rhs = convert(rhs,common);
            rhs = operation(binary, lhs, rhs, common);
            rhs = convert(rhs, fact.type);
        }
        rhs = store(rhs, dest); dest.cached = true; dest.stored = rhs.operand; return dest;
    }
    auto left = sem.conversion_fact(fact.conversions), right = sem.conversion_fact(fact.conversions+1);
    Value lhs = left.kind == semantic::Conversion::Kind::User || left.derived ? converted(a,left) : load(expression(a));
    Value rhs = right.kind == semantic::Conversion::Kind::User || right.derived ? converted(b,right) : load(expression(b));
    lhs = convert(lhs, left.target, left.fold_widen, left.preserve_widen);
    rhs = convert(rhs, right.target, right.fold_widen, right.preserve_widen);
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
            if (b.ir == IRType::I64 && b.type && sem.unsigned_type(b.type) && !b.operand.literal())
                b = emit(Opcode::Copy,IRType::I64,{b.operand});
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
    Instruction instruction(compare ? Opcode::Compare : Opcode::Binary,a.ir);
    instruction.operation = action;
    // Source C++ evaluation uses the same permitted x87 excess precision as
    // constant evaluation, then materializes each result at its declared type.
    // The explicit course LowIR view retains its exact-width operation contract.
    if (!linkage.presentation && !compare && a.ir.floating() && a.ir != IRType::F80)
        instruction.source_type = IRType::F80;
    v = emit(instruction,{a.operand,b.operand});
    v.type = result; return v;
}
Value Procedural::call(NodeId n, Value destination)
{
    auto node = ast[n];
    auto fact = sem.expression_fact(n);
    if (fact.form == semantic::ExpressionForm::PseudoDestructor) {
        NodeId member = node.first;
        while (ast[member].kind == Kind::Parenthesized) member = ast[member].first;
        if (ast[member].op == OP_ARROW) arrow_object(ast[member].first,sem.object_fact(member).arrow);
        else expression(ast[member].first);
        return Value(Operand(), IRType::Void, fact.type);
    }
    if (fact.form == semantic::ExpressionForm::InvokeMemberData) {
        auto value = member_pointer_object(sem.object_fact(n));
        value.type = fact.type; value.address = true; return value;
    }
    if (fact.form == semantic::ExpressionForm::Unreachable) return emit(Opcode::Unreachable, IRType(), {});
    if (fact.form == semantic::ExpressionForm::Abort) return abort_call();
    if (fact.form == semantic::ExpressionForm::Expect) {
        auto value = converted(sem.call_argument(fact,0),sem.conversion_fact(fact.conversions));
        converted(sem.call_argument(fact,1),sem.conversion_fact(fact.conversions+1));
        return value;
    }
    auto intrinsic = sem.intrinsic_function(sem.facts[n].entity);
    if (intrinsic == semantic::Intrinsic::Atomic) return atomic_call(n,destination);
    if (intrinsic != semantic::Intrinsic::None) return intrinsic_call(n,intrinsic);
    bool class_result = sem.class_value(sem.facts[n].type);
    SlotId result_slot = !class_result && type(sem.facts[n].type) != IRType::Void && full_expression.enabled && (unwind_expression(n) || cleanup_expression(n,false,true)) ? builder->add_slot(0,type(sem.facts[n].type)) : SlotId();
    bool indirect_result = sem.indirect_value(sem.facts[n].type);
    bool own_result = class_result && destination.ir == IRType();
    if (own_result) destination = class_address(sem.object_fact(n).temporary,fact.type);
    std::size_t begin = call_work.size();
    call_work.push_back(Operand());
    if (indirect_result) call_work.push_back(destination.operand);
    auto object_use = sem.object_fact(n);
    bool saved_storage = full_expression.argument_storage;
    bool storage_only = fact.argument_count != 0;
    for (unsigned j = 0; j < fact.argument_count; ++j) {
        auto arg = sem.call_argument(fact,j), target = sem.conversion_fact(fact.conversions+j).target;
        if (reference(target)) target = sem.types[target].child;
        storage_only &= ast[arg].kind == Kind::Lambda && !sem.closure(sem.types[sem.expression_fact(arg).type].entity).first_capture && sem.types.unqualified(target) == sem.types.unqualified(sem.expression_fact(arg).type);
    }
    // Captureless arguments only require addressable storage. Finish receiver
    // activation before allocating those slots; the actual call opens its
    // own region using the completed receiver's live suffix.
    full_expression.argument_storage |= storage_only;
    Value member_function;
    if (object_use.callable_entry) {
        // An immediately invoked captureless lambda has no receiver effects.
        // Its already selected ABI entry owns the parameter-only signature.
    } else if (object_use.member_pointer) {
        auto object = member_pointer_object(object_use,&member_function);
        call_work.push_back(object.operand);
    } else if (object_use.type) {
        Value object;
        if (object_use.node) {
            if (object_use.arrow) object = arrow_object(object_use.node,object_use.arrow);
            else {
                object = expression(object_use.node, true);
                object = sem.types[sem.expression_fact(object_use.node).type].kind == TypeKind::Pointer ? load(object) : address(object);
            }
        } else object = object_use.capture ? captured_address(object_use.capture) : implicit_object();
        object = base_projection(object,object_use.qualifier_adjustment);
        call_work.push_back(base_projection(object,object_use.adjustment).operand);
    } else if (object_use.node) {
        if (object_use.arrow) arrow_object(object_use.node,object_use.arrow);
        else {
            Value object = expression(object_use.node);
            if (sem.types[object.type].kind == TypeKind::Pointer) load(object);
        }
    }
    full_expression.argument_storage = saved_storage;
    if (fact.form == semantic::ExpressionForm::LiteralCall) literal_arguments(n);
    // The course's indirect-call fixtures evaluate arguments before fetching
    // the callee; C++11 leaves their relative evaluation order unspecified.
    for (unsigned j = 0; j < fact.argument_count; ++j) {
        NodeId argument = sem.call_argument(fact,j);
        call_work.push_back(argument ? converted(argument, sem.conversion_fact(fact.conversions+j)).operand : Operand::integer(0));
    }
    NodeId callee = object_use.callee ? object_use.callee : node.first;
    EntityId selected = sem.facts[n].entity;
    Instruction i(Opcode::Call, indirect_result ? IRType(IRType::Void) : type(sem.facts[n].type));
    if (object_use.callable_entry) {
        call_work[begin] = emit(Opcode::Addr,IRType(),{Operand::symbol(symbol(object_use.callable_entry))}).operand;
        i.signature = signature(sem.entities[object_use.callable_entry].type);
    } else if (object_use.member_pointer) {
        call_work[begin] = object_use.member_target ?
            emit(Opcode::Addr,IRType(),{Operand::symbol(member_function_symbol(sem.member_target_value(object_use.member_target).entity))}).operand : member_function.operand;
        auto pointer_type = sem.expression_fact(object_use.member_pointer).type;
        if (auto root = sem.fold_root(object_use.member_pointer))
            pointer_type = sem.fold_step(sem.fold_step(root).right).operation.result.type;
        i.signature = signature(pointer_type);
    } else if (object_use.virtual_slot) {
        call_work[begin] = virtual_function(Value(call_work[begin+1+indirect_result],IRType::Ptr),object_use.virtual_slot).operand;
        i.signature = virtual_signature(selected);
    } else if (selected) call_work[begin] = Operand::symbol(symbol(selected));
    else {
        auto c = sem.conversion_fact(object_use.callee_conversion);
        Value fn = object_use.callee_conversion ? converted(callee,c) : load(expression(callee));
        if (object_use.block_signature) {
            // The source callable is evaluated once. ABI.2010.3.16 places the
            // invocation entry after isa, flags and reserved on Linux x86-64.
            call_work.insert(call_work.begin()+begin+1+indirect_result,fn.operand);
            auto entry = emit(Opcode::Index,IRType::I8,{fn.operand,Operand::integer(16)});
            call_work[begin] = emit(Opcode::Load,IRType::Ptr,{entry.operand}).operand;
            i.signature = signature(object_use.block_signature);
        } else call_work[begin] = fn.operand;
        TypeId ft = object_use.callee_conversion ? c.target : sem.expression_fact(callee).type;
        if (sem.types[ft].kind == TypeKind::Pointer) ft = sem.types[ft].child;
        if (!object_use.block_signature) i.signature = signature(ft);
    }
    Value v = guarded_call(i, call_work.data()+begin, call_work.size()-begin); call_work.resize(begin); v.type = fact.type;
    if (result_slot) {
        emit(Opcode::Store,v.ir,{v.operand,Operand::slot(result_slot)});
        v = emit(Opcode::Load,v.ir,{Operand::slot(result_slot)}); v.type = fact.type;
    }
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
