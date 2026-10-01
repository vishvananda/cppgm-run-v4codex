#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
IRType Procedural::type(TypeId id)
{
    const semantic::Type t = sem.types[id];
    switch (t.kind) {
    case TypeKind::BlockPointer: case TypeKind::Pointer: case TypeKind::LRef: case TypeKind::RRef: case TypeKind::Function: return IRType::Ptr;
    case TypeKind::Vector: case TypeKind::ExtVector: return IRType::vector(sem.object_size(id),sem.object_alignment(id),t.kind == TypeKind::ExtVector);
    case TypeKind::Array: return IRType::object(sem.object_size(id), sem.object_alignment(id));
    case TypeKind::MemberPointer: return sem.types[t.child].kind == TypeKind::Function ? IRType(IRType::I128) : IRType(IRType::I64);
    case TypeKind::Named:
        if (sem.entities[t.entity].underlying) return type(sem.entities[t.entity].underlying);
        return IRType::object(sem.object_size(id), sem.object_alignment(id));
    case TypeKind::Fundamental: {
        if (sem.complex_type(id)) return IRType::complex(type(sem.complex_component(id)));
        if (semantic::bit_integer_kind(t.fundamental)) {
            auto bits = t.bound; bool unsign = t.fundamental == FT_UBITINT;
            return bits <= 8 ? (unsign ? IRType::U8 : IRType::I8) : bits <= 16 ? (unsign ? IRType::U16 : IRType::I16) :
                bits <= 32 ? (unsign ? IRType::U32 : IRType::I32) : bits <= 64 ? IRType(IRType::I64) : IRType::integer128_align8();
        }
        static const IRType::Kind kinds[] = {IRType::I8, IRType::I16, IRType::I32, IRType::I64, IRType::I64,
            IRType::U8, IRType::U16, IRType::U32, IRType::I64, IRType::I64, IRType::I32, IRType::I8,
            IRType::U16, IRType::U32, IRType::U8, IRType::F32, IRType::F64, IRType::F80, IRType::Void, IRType::I64, IRType::I128, IRType::I128};
        return kinds[t.fundamental];
    }
    default: throw std::runtime_error("unsupported lowering type");
    }
}
bool Procedural::reference(TypeId t) const { return sem.types[t].kind == TypeKind::LRef || sem.types[t].kind == TypeKind::RRef; }
NodeId Procedural::child(NodeId n, Kind k) const
{
    for (NodeId c = ast[n].first; c; c = ast[c].next) if (ast[c].kind == k) return c;
    return 0;
}
Value Procedural::emit_raw(Instruction i, const Operand* args, std::size_t count)
{
    // GNU zero-extent objects have an address but no representation to copy
    // or clear. Operands have already been evaluated; emit no empty bulk span.
    if ((i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit) && !i.bytes) return Value();
    // An expression can leave its containing statement (GNU statement body).
    // Its remaining consumers have no reachable instruction stream.
    if (ended) return Value(Operand::integer(0),i.result_type());
    if (full_expression.open && !emitting_cleanup && lowir_model::terminator(i.opcode)) close_expression_region();
    i.operands.begin = p.operands.size(); i.operands.count = count;
    for (std::size_t j = 0; j < count; ++j) p.operands.push_back(args[j]);
    if (i.result_type() != IRType()) i.destination = builder->value(0);
    builder->append(i);
    if (lowir_model::terminator(i.opcode)) ended = true;
    Value result(Operand::value(i.destination), i.result_type());
    result.nonnull = i.opcode == Opcode::Addr;
    if (i.opcode == Opcode::Load && count == 1 && args[0].kind == Operand::Slot &&
        this_slot && args[0].ref == this_slot.index &&
        (sem.constructor_member(active_function) || sem.destructor_member(active_function)))
        result.parameter_object = active_function;
    return result;
}
Value Procedural::emit(Instruction i, const std::vector<Operand>& args) { return emit(i, args.data(), args.size()); }
Value Procedural::emit(Instruction i, std::initializer_list<Operand> args) { return emit(i, args.begin(), args.size()); }
Value Procedural::emit(Opcode op, IRType t, std::initializer_list<Operand> args, Operation action)
{
    Instruction i(op, t); i.operation = action;
    return emit(i, args);
}
Value Procedural::load(Value v)
{
    if (!v.address) return v;
    if (v.bit_field) return load_bit_field(v);
    if (sem.types[v.type].cv & 4) {
        if (!inline_atomic(v.type)) {
            auto target = sem.types.non_atomic(sem.types.unqualified(v.type));
            auto slot = Operand::slot(builder->add_slot(0,type(target)));
            atomic_runtime(AtomicOp::Load,sem.object_size(target),v.operand,slot);
            return Value(slot,type(target),target);
        }
        auto raw = atomic_representation(v.type);
        return atomic_value(emit(Opcode::AtomicLoad,raw,{address(v).operand,Operand::integer(5)}),sem.types.non_atomic(sem.types.unqualified(v.type)));
    }
    if (sem.class_value(v.type) || semantic::vector_kind(sem.types[v.type].kind)) { v.address = false; v.ir = type(v.type); return v; }
    if (sem.types[v.type].kind == TypeKind::Array || sem.types[v.type].kind == TypeKind::Function) return address(v);
    if (v.cached && !(sem.types[v.type].cv & 2)) return Value(v.stored, type(v.type), v.type);
    Instruction i(Opcode::Load, type(v.type)); i.is_volatile = sem.types[v.type].cv & 2;
    Value r = emit(i, {v.operand}); r.type = v.type; r.member_zero_adjustment = v.member_zero_adjustment; return normalize_bit_integer(r);
}
Value Procedural::address(Value v)
{
    if (!v.address) throw std::logic_error("missing addressable semantic value");
    if (v.operand.kind == Operand::Slot || v.operand.kind == Operand::Symbol) {
        auto object = v.parameter_object; auto overlap = v.overlapping;
        v = emit(Opcode::Addr, IRType(), {v.operand}); v.parameter_object = object; v.overlapping = overlap;
    }
    v.address = false; v.ir = IRType::Ptr; return v;
}
Value Procedural::store(Value v, Value location)
{
    if (location.bit_field) return store_bit_field(v, location);
    if (sem.types[location.type].cv & 4) {
        if (!inline_atomic(location.type)) {
            atomic_runtime(AtomicOp::Store,sem.object_size(location.type),location.operand,atomic_buffer(v)); return v;
        }
        auto raw = atomic_representation(location.type);
        auto bits = atomic_bits(v,raw);
        emit(Opcode::AtomicStore,raw,{bits.operand,address(location).operand,Operand::integer(5)}); return v;
    }
    if (sem.complex_type(location.type)) {
        auto real = load(complex_component(v,0)); auto imag = load(complex_component(v,1));
        store(real,complex_component(location,0)); store(imag,complex_component(location,1)); return v;
    }
    if (type(location.type).kind() == IRType::Object) {
        Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(location.type); copy.alignment = sem.object_alignment(location.type);
        emit(copy,{v.operand,address(location).operand}); return v;
    }
    Instruction i(Opcode::Store, type(location.type)); i.is_volatile = sem.types[location.type].cv & 2;
    emit(i, {v.operand, location.operand});
    return v;
}
Value Procedural::coerce(Value v, IRType to, bool unsign, bool to_unsigned, bool fold_widen, bool preserve_widen)
{
    if (v.ir == to) return v;
    Instruction i(Opcode::Convert, to); i.source_type = v.ir;
    if (to.floating() && v.ir.floating()) i.operation = to.bytes() > v.ir.bytes() ? Operation::Fpext : Operation::Fptrunc;
    else if (to.floating()) i.operation = unsign ? Operation::Uitofp : Operation::Sitofp;
    else if (v.ir.floating()) i.operation = to_unsigned ? Operation::Fptoui : Operation::Fptosi;
    else if (to.bytes() == v.ir.bytes()) {
        if (v.operand.literal()) return Value(v.operand, to);
        return emit(Opcode::Copy, to, {v.operand});
    }
    else i.operation = to.bytes() < v.ir.bytes() ? Operation::Trunc : unsign ? Operation::Zext : Operation::Sext;
    // Required immediate widening may be represented by the final literal.
    if (v.operand.kind == Operand::Integer && to.integer() && v.ir.integer() &&
        (!preserve_widen || to.width() <= v.ir.width()) && (to.width() <= 32 || to.width() < v.ir.width() || fold_widen || !to_unsigned)) {
        std::uint64_t bits = v.operand.data.integer;
        if (v.ir.width() < 64) {
            auto mask = (std::uint64_t(1) << v.ir.width()) - 1;
            bits &= mask;
            if (!unsign && (bits & (std::uint64_t(1) << (v.ir.width()-1)))) bits |= ~mask;
        }
        if (to.width() < 64) bits &= (std::uint64_t(1) << to.width()) - 1;
        auto literal = Operand::integer(bits);
        if (to == IRType::I128) literal.integer_high(v.ir == IRType::I128 ? v.operand.integer_high() :
            !unsign && (bits >> 63) ? ~std::uint64_t(0) : 0);
        return Value(literal, to);
    }
    return emit(i, {v.operand});
}
Value Procedural::convert(Value v, TypeId to, bool fold_widen, bool preserve_widen)
{
    if (reference(to)) {
        TypeId referred = sem.types[to].child;
        if (!v.address || v.bit_field || sem.types.unqualified(v.type) != sem.types.unqualified(referred)) {
            SlotId existing = sem.types.unqualified(v.type) == sem.types.unqualified(referred) ? v.materialized : SlotId();
            bool pointer_conversion = sem.types[referred].kind == TypeKind::Pointer &&
                sem.types[v.type].kind != TypeKind::Pointer;
            v = convert(v, referred);
            // Retain a pointer conversion at a reference boundary. A value
            // already typed as a pointer only needs its temporary's store.
            if (pointer_conversion && v.operand.literal()) v = emit(Opcode::Copy, IRType::Ptr, {v.operand});
            SlotId slot = existing ? existing : builder->add_slot(0, type(referred));
            Value location(Operand::slot(slot), type(referred), referred, true);
            store(v, location); v = location;
        }
        return address(v);
    }
    TypeId from = v.type;
    if (sem.complex_type(from) || sem.complex_type(to)) return complex_convert(v,to);
    if (sem.types[to].kind == TypeKind::MemberPointer && (!from || sem.types[from].kind != TypeKind::MemberPointer))
        return member_pointer_value(0,to);
    v = normalize_bit_integer(load(v));
    IRType target = type(to);
    if (from && sem.types[from].kind == TypeKind::Fundamental && sem.types[from].fundamental == FT_NULLPTR_T && target == IRType::I64)
        return Value(Operand::integer(0), target, to);
    if (target == IRType::Void) { v.type = to; v.ir = target; return v; }
    bool from_bool = from && sem.types[from].kind == TypeKind::Fundamental && sem.types[from].fundamental == FT_BOOL;
    if (sem.types[to].kind == TypeKind::Fundamental && sem.types[to].fundamental == FT_BOOL && !from_bool) {
        Operand zero = v.ir.floating() ? Operand::floating(0) : Operand::integer(0);
        if (sem.types[from].kind == TypeKind::MemberPointer) v = truth_operand(v);
        v = emit(Opcode::Compare, v.ir, {v.operand, zero}, Operation::Ne);
    }
    bool unsign = from && sem.unsigned_type(from);
    if (target == IRType::Ptr && v.operand.literal()) {
        if (v.operand.kind == Operand::Null || (from && sem.types[from].kind == TypeKind::Named))
            v = emit(Opcode::Copy, target, {v.operand});
        else v.ir = target;
    } else v = coerce(v, target, unsign, sem.unsigned_type(to), fold_widen, preserve_widen);
    v.type = to; return normalize_bit_integer(v);
}
Value Procedural::converted(NodeId n, const semantic::Conversion& c)
{
    SourceInvocationScope invocation(source_invocation,0,c.default_argument);
    if (c.reference) if (auto temporary = sem.retained_scalar(n,c.target)) {
        auto destination = binding(temporary);
        if (!sem.entities[temporary].constant.valid) {
            auto scalar = c; scalar.target = sem.entities[temporary].type;
            scalar.reference = scalar.temporary = false;
            store(converted(n,scalar),destination);
        }
        auto result = address(destination); result.type = c.target; return result;
    }
    if (c.kind == semantic::Conversion::Kind::List) return list_conversion(c);
    if (c.kind == semantic::Conversion::Kind::User) return user_conversion(n,c);
    if (c.kind == semantic::Conversion::Kind::Construction) {
        auto materialized = sem.conversion_objects[c.materialization];
        if (materialized.use != semantic::ConversionUse::Temporary || !materialized.temporary)
            throw std::logic_error("value conversion lacks temporary storage");
        EntityId object = materialized.temporary;
        TypeId t = sem.entities[object].type;
        Value pointer = class_address(object,t), destination = class_temporary(object,t);
        construct_value(n,c,pointer);
        if (c.reference || c.ellipsis_object) activate_temporary(object);
        return c.reference || c.ellipsis_object || sem.indirect_parameter(t) ? pointer : Value(destination.operand,type(t),t);
    }
    if (c.empty_copy && !c.reference) {
        SlotId slot = builder->add_slot(0, type(c.target));
        Value destination(Operand::slot(slot), type(c.target), c.target, true);
        Value pointer = address(destination);
        construct_value(n,c,pointer);
        return c.ellipsis_object ? pointer : Value(Operand::slot(slot), type(c.target), c.target);
    }
    if (c.ellipsis_object) return address(expression(n));
    return converted_value(expression(n,c.reference && sem.expression_fact(n).category != ValueCategory::Prvalue),c);
}
Value Procedural::converted_value(Value v, const semantic::Conversion& c)
{
    if (c.derived) {
        if (sem.types[c.target].kind == TypeKind::MemberPointer) return member_pointer_conversion(load(v),c);
        v = c.reference && !c.temporary ? base_projection(address(v),c.adjustment) : pointer_projection(load(v),c.adjustment);
        if (c.temporary) { v.type = sem.types[c.target].child; return convert(v, c.target); }
        v.type = c.target; return v;
    }
    if (c.reference && !c.temporary && v.address) return address(v);
    if (c.kind == semantic::Conversion::Kind::Explicit && v.type && type(v.type) == type(c.target) &&
        type(c.target).integer() && sem.unsigned_type(v.type) != sem.unsigned_type(c.target)) {
        v = load(v);
        if (!v.operand.literal()) { auto result = emit(Opcode::Copy, v.ir, {v.operand}); result.type = c.target; return result; }
    }
    Value result = convert(v,c.target,c.fold_widen,c.preserve_widen);
    if (c.kind == semantic::Conversion::Kind::Explicit && sem.types[c.target].kind == TypeKind::Pointer &&
        result.operand.kind == Operand::Integer && result.operand.data.integer) {
        result = emit(Opcode::Copy,IRType::Ptr,{result.operand}); result.type = c.target;
    }
    return result;
}
Value Procedural::incoming(NodeId n)
{
    auto x = sem.expression_fact(n);
    return x.incoming ? converted(n, sem.conversion_fact(x.incoming)) : load(expression(n));
}
Value Procedural::base_projection(Value base, unsigned steps)
{
    if (steps) {
        bool nonnull = base.nonnull;
        auto parameter = base.parameter_object;
        const auto& path = sem.base_adjustments[steps];
        if (path.virtual_row) {
            if (parameter && parameter == active_function) {
                base = lifecycle_address(sem.base_virtual_anchor(steps));
                base = emit(Opcode::Index,IRType::I8,{base.operand,Operand::integer(path.virtual_tail)});
                base.nonnull = nonnull; base.parameter_object = parameter; return base;
            }
            auto hidden = parameter_base_addresses.get((std::uint64_t(parameter)<<32)|sem.base_virtual_anchor(steps));
            if (parameter && hidden) {
                base = emit(Opcode::Index,IRType::I8,{Operand::value(lowir_model::ValueId(hidden)),Operand::integer(path.virtual_tail)});
                base.nonnull = nonnull; base.parameter_object = parameter; return base;
            }
            auto table = emit(Opcode::Load,IRType::Ptr,{base.operand});
            auto row = Operand::integer(path.virtual_row); row.negative_integer = true;
            auto location = emit(Opcode::Index,IRType::I8,{table.operand,row});
            auto offset = emit(Opcode::Load,IRType::I64,{location.operand});
            base = emit(Opcode::Index,IRType::I8,{base.operand,offset.operand});
            base = emit(Opcode::Index,IRType::I8,{base.operand,Operand::integer(path.virtual_tail)});
            base.nonnull = nonnull; base.parameter_object = parameter; return base;
        }
        auto offset = Operand::integer(path.total);
        offset.negative_integer = std::int64_t(sem.base_adjustments[steps].total) < 0;
        base = emit(Opcode::Index, IRType::I8,{base.operand, offset});
        base.nonnull = nonnull; base.parameter_object = parameter;
    }
    return base;
}
Value Procedural::implicit_object()
{
    auto object = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
    if (initializer_receiver) {
        Instruction index(Opcode::Index,IRType::I8); index.projection = ir_model::IPK_FIELD;
        object = emit(index,{object.operand,Operand::integer(sem.construction_storage[initializer_receiver].offset)});
    }
    return object;
}
Value Procedural::field(Value base, EntityId e, unsigned steps, TypeId object)
{
    base = base_projection(base, steps);
    auto offset = sem.field_projection(e,object).offset;
    Instruction i(Opcode::Index, IRType::I8); i.projection = ir_model::IPK_FIELD;
    Value v = emit(i, {base.operand, Operand::integer(offset)});
    v.type = sem.entities[e].type;
    if (sem.field_fact(e).bit_field) v.bit_field = e;
    if (reference(v.type)) { v = load(Value(v.operand, IRType::Ptr, v.type, true)); v.type = sem.types[sem.entities[e].type].child; }
    v.address = true; return v;
}
Value Procedural::binding(EntityId e)
{
    if (!e) throw std::logic_error("missing resolved declaration");
    auto projection = sem.binding_projection(e);
    if (projection.object) {
        auto base = address(binding(projection.object));
        if (projection.member) return field(base,projection.member,projection.adjustment);
        auto t = sem.entities[e].type;
        Instruction index(Opcode::Index,IRType::I8); index.projection = ir_model::IPK_ARRAY_ELEMENT;
        auto value = emit(index,{base.operand,Operand::integer(projection.element*sem.object_size(t))});
        value.type = t; value.address = true; return value;
    }
    const auto& entity = sem.entities[e];
    TypeId t = entity.type;
    if (sem.static_temporary(e).object) return Value(Operand::symbol(symbols[e]),type(t),t,true);
    if (object_addresses[e]) {
        Value value(Operand::value(object_addresses[e]),type(t),t,true);
        if (entity.kind == semantic::EntityKind::Parameter && sem.class_value(t)) value.parameter_object = e;
        return value;
    }
    if (sem.nonstatic_field(e)) {
        auto storage = sem.field_projection(e).object;
        if (storage) return field(address(binding(storage)), e);
        if (!this_slot) throw std::logic_error("missing implicit object");
        auto owner = sem.scopes[sem.entities[active_function].owner].entity;
        auto type = initializer_receiver ? sem.entities[sem.construction_storage[initializer_receiver].field].type : sem.entities[owner].type;
        return field(implicit_object(), e, 0, type);
    }
    // An uninitialized automatic declaration may legally be bypassed by goto.
    // Storage identity is independent of whether its declaration falls through.
    if ((!objects[e] || p.slots[objects[e].index-1].owner.index != function.index) && entity.kind == semantic::EntityKind::Variable &&
        sem.scopes[entity.owner].kind != semantic::ScopeKind::Namespace && !entity.is_static)
        objects[e] = builder->add_slot(0, type(t));
    bool local = objects[e].index != 0;
    Operand location = local ? Operand::slot(objects[e]) : Operand::symbol(symbol(e));
    if (entity.thread_local_storage) {
        auto wrapper = tls_wrappers.get(e);
        if (wrapper && active_tls != e) {
            Operand argument = Operand::symbol(SymbolId(wrapper));
            location = guarded_call(Instruction(Opcode::Call, IRType::Ptr), &argument, 1).operand;
        } else location = emit(Opcode::Addr, IRType(), {location}).operand;
    }
    if (reference(t)) {
        Value pointer = emit(Opcode::Load, IRType::Ptr, {location});
        t = sem.types[t].child;
        return Value(pointer.operand, IRType::Ptr, t, true);
    }
    // Incomplete arrays are addressable without demanding a layout.
    IRType ir = sem.types[t].kind == TypeKind::Array ? IRType(IRType::Ptr) : type(t);
    Value result(location, ir, t, true);
    if (entity.kind == semantic::EntityKind::Parameter && sem.class_value(t)) result.parameter_object = e;
    result.member_zero_adjustment = sem.member_pointer_zero_adjustment(e); return result;
}
BlockId Procedural::block() { return builder->block(0); }
void Procedural::start(BlockId b) { builder->start_block(b); ended = false; }
void Procedural::jump(BlockId b) { if (!ended) emit(Opcode::Jump, IRType(), {Operand::label(b)}); }
} }
