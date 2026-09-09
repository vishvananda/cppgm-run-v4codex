#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
IRType Procedural::type(TypeId id)
{
    const semantic::Type t = sem.types[id];
    switch (t.kind) {
    case TypeKind::Pointer: case TypeKind::LRef: case TypeKind::RRef: case TypeKind::Function: return IRType::Ptr;
    case TypeKind::Array: return IRType::object(sem.object_size(id), sem.object_alignment(id));
    case TypeKind::Named:
        if (sem.entities[t.entity].underlying) return type(sem.entities[t.entity].underlying);
        return IRType::object(sem.object_size(id), sem.object_alignment(id));
    case TypeKind::Fundamental: {
        static const IRType::Kind kinds[] = {IRType::I8, IRType::I16, IRType::I32, IRType::I64, IRType::I64,
            IRType::U8, IRType::U16, IRType::U32, IRType::I64, IRType::I64, IRType::I32, IRType::I8,
            IRType::U16, IRType::U32, IRType::U8, IRType::F32, IRType::F64, IRType::F80, IRType::Void, IRType::I64};
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
Value Procedural::emit(Instruction i, const Operand* args, std::size_t count)
{
    i.operands.begin = p.operands.size(); i.operands.count = count;
    for (std::size_t j = 0; j < count; ++j) p.operands.push_back(args[j]);
    if (i.result_type() != IRType()) i.destination = builder->value(0);
    builder->append(i);
    if (lowir_model::terminator(i.opcode)) ended = true;
    return Value(Operand::value(i.destination), i.result_type());
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
    if (sem.types[v.type].kind == TypeKind::Array || sem.types[v.type].kind == TypeKind::Function) return address(v);
    if (v.cached && !(sem.types[v.type].cv & 2)) return Value(v.stored, type(v.type), v.type);
    Instruction i(Opcode::Load, type(v.type)); i.is_volatile = sem.types[v.type].cv & 2;
    Value r = emit(i, {v.operand}); r.type = v.type; return r;
}
Value Procedural::address(Value v)
{
    if (!v.address) throw std::logic_error("missing addressable semantic value");
    if (v.operand.kind == Operand::Slot || v.operand.kind == Operand::Symbol)
        v = emit(Opcode::Addr, IRType(), {v.operand});
    v.address = false; v.ir = IRType::Ptr; return v;
}
Value Procedural::store(Value v, Value location)
{
    if (location.bit_field) return store_bit_field(v, location);
    Instruction i(Opcode::Store, type(location.type)); i.is_volatile = sem.types[location.type].cv & 2;
    emit(i, {v.operand, location.operand});
    return v;
}
Value Procedural::coerce(Value v, IRType to, bool unsign, bool to_unsigned, bool fold_widen)
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
    if (v.operand.kind == Operand::Integer && to.integer() && v.ir.integer() && (to.width() <= 32 || to.width() < v.ir.width() || fold_widen || !to_unsigned)) {
        std::uint64_t bits = v.operand.data.integer;
        if (v.ir.width() < 64) {
            auto mask = (std::uint64_t(1) << v.ir.width()) - 1;
            bits &= mask;
            if (!unsign && (bits & (std::uint64_t(1) << (v.ir.width()-1)))) bits |= ~mask;
        }
        if (to.width() < 64) bits &= (std::uint64_t(1) << to.width()) - 1;
        return Value(Operand::integer(bits), to);
    }
    return emit(i, {v.operand});
}
Value Procedural::convert(Value v, TypeId to, bool fold_widen)
{
    if (reference(to)) {
        TypeId referred = sem.types[to].child;
        if (!v.address || v.bit_field || sem.types.unqualified(v.type) != sem.types.unqualified(referred)) {
            v = convert(v, referred);
            SlotId slot = builder->add_slot(0, type(referred));
            Value location(Operand::slot(slot), type(referred), referred, true);
            store(v, location); v = location;
        }
        return address(v);
    }
    TypeId from = v.type;
    v = load(v);
    IRType target = type(to);
    if (from && sem.types[from].kind == TypeKind::Fundamental && sem.types[from].fundamental == FT_NULLPTR_T && target == IRType::I64)
        return Value(Operand::integer(0), target, to);
    if (target == IRType::Void) { v.type = to; v.ir = target; return v; }
    bool from_bool = from && sem.types[from].kind == TypeKind::Fundamental && sem.types[from].fundamental == FT_BOOL;
    if (sem.types[to].kind == TypeKind::Fundamental && sem.types[to].fundamental == FT_BOOL && !from_bool) {
        v = emit(Opcode::Compare, v.ir, {v.operand, v.ir.floating() ? Operand::floating(0) : Operand::integer(0)}, Operation::Ne);
    }
    bool unsign = from && sem.unsigned_type(from);
    if (target == IRType::Ptr && v.operand.literal()) {
        if (v.operand.kind == Operand::Null || (from && sem.types[from].kind == TypeKind::Named))
            v = emit(Opcode::Copy, target, {v.operand});
        else v.ir = target;
    } else v = coerce(v, target, unsign, sem.unsigned_type(to), fold_widen);
    v.type = to; return v;
}
Value Procedural::converted(NodeId n, const semantic::Conversion& c)
{
    if (c.kind == semantic::Conversion::Kind::Construction) {
        auto materialized = sem.conversion_objects[c.materialization];
        EntityId object = materialized.temporary;
        TypeId t = sem.entities[object].type;
        objects[object] = builder->add_slot(0, type(t));
        Value destination(Operand::slot(objects[object]), type(t), t, true);
        Value pointer = address(destination);
        std::size_t begin = call_work.size();
        call_work.push_back(Operand::symbol(symbol(materialized.constructor))); call_work.push_back(pointer.operand);
        for (unsigned j = 0; j < materialized.call.argument_count; ++j)
            call_work.push_back(converted(sem.call_arguments[materialized.call.arguments+j], sem.conversion_fact(materialized.call.conversions+j)).operand);
        guarded_call(Instruction(Opcode::Call, IRType::Void), call_work.data()+begin, call_work.size()-begin);
        call_work.resize(begin); activate_temporary(object);
        return c.reference ? pointer : Value(destination.operand, type(t), t);
    }
    if (c.empty_copy && !c.reference) {
        SlotId slot = builder->add_slot(0, type(c.target));
        Value destination(Operand::slot(slot), type(c.target), c.target, true);
        address(destination);
        address(expression(n, true));
        return Value(Operand::slot(slot), type(c.target), c.target);
    }
    Value v = expression(n, c.reference);
    if (c.derived) {
        v = base_projection(c.reference && !c.temporary ? address(v) : load(v), 1);
        if (c.temporary) { v.type = sem.types[c.target].child; return convert(v, c.target); }
        v.type = c.target; return v;
    }
    if (c.reference && !c.temporary && v.address) return address(v);
    return convert(v, c.target);
}
Value Procedural::incoming(NodeId n)
{
    auto x = sem.expression_fact(n);
    return x.incoming ? converted(n, sem.conversion_fact(x.incoming)) : load(expression(n));
}
Value Procedural::base_projection(Value base, unsigned steps)
{
    // Single inheritance puts the entire selected base path at offset zero.
    if (steps) base = emit(Opcode::Index, IRType::I8, {base.operand, Operand::integer(0)});
    return base;
}
Value Procedural::field(Value base, EntityId e, unsigned steps)
{
    base = base_projection(base, steps);
    std::uint64_t offset = sem.entities[e].member_offset;
    for (EntityId storage = sem.injected_storage(e); storage && sem.nonstatic_field(storage); storage = sem.injected_storage(storage))
        offset += sem.entities[storage].member_offset;
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
    const auto& entity = sem.entities[e];
    TypeId t = entity.type;
    if (sem.nonstatic_field(e)) {
        if (auto storage = sem.injected_storage(e)) {
            if (!sem.nonstatic_field(storage)) return field(address(binding(storage)), e);
        }
        if (!this_slot) throw std::logic_error("missing implicit object");
        return field(emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)}), e);
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
    return Value(location, ir, t, true);
}
BlockId Procedural::block() { return builder->block(0); }
void Procedural::start(BlockId b) { builder->start_block(b); ended = false; }
void Procedural::jump(BlockId b) { if (!ended) emit(Opcode::Jump, IRType(), {Operand::label(b)}); }
} }
