#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using syntax::Kind;
using namespace lowir_model;
void Procedural::prepare_reference_guards(EntityId e)
{
    for (auto choice = sem.reference_choices(e); choice; choice = sem.reference_alternatives[choice].next) {
        EntityId object = sem.reference_alternatives[choice].object;
        if (!sem.temporary_cleanup(object)) continue;
        auto guard = builder->add_slot(0,IRType::I64);
        local_reference_guards.put(object,guard.index);
        emit(Opcode::Store,IRType::I64,{Operand::integer(0),Operand::slot(guard)});
    }
}
void Procedural::destroy_reference_choices(EntityId e)
{
    for (auto choice = sem.reference_choices(e); choice; choice = sem.reference_alternatives[choice].next) {
        EntityId object = sem.reference_alternatives[choice].object;
        auto guard = local_reference_guards.get(object);
        if (!guard) continue;
        auto test = emit(Opcode::Load,IRType::I64,{Operand::slot(SlotId(guard))});
        auto run = block(), end = block();
        emit(Opcode::Branch,IRType(),{test.operand,Operand::label(run),Operand::label(end)});
        start(run);
        destroy(sem.object_destructor(object),sem.entities[object].type,address(class_temporary(object,sem.entities[object].type)));
        jump(end); start(end);
    }
}
void Procedural::reference_global(EntityId e)
{
    auto storage = sem.static_temporary(e);
    auto add = [&](IRType type, bool aggregate, const std::string& name) {
        Global g; g.symbol = fresh_symbol(name); g.type = type; g.structured = aggregate;
        g.data.begin = p.data.size(); g.data.count = 1;
        DataItem zero; zero.zero_bytes = type.bytes(); p.data.push_back(zero); p.globals.push_back(g);
        auto& symbol = p.symbols[g.symbol.index-1]; symbol.kind = Symbol::GlobalSymbol; symbol.entity = p.globals.size();
        symbol.metadata.binding = ir_model::SBM_INTERNAL;
        return g.symbol;
    };
    TypeId t = sem.entities[e].type;
    symbols[e] = add(type(t),sem.class_value(t) || sem.types[t].kind == TypeKind::Array,"@__reference_"+std::to_string(e));
    if (storage.conditional && sem.destructor_needed(sem.object_destructor(e)))
        reference_guards.put(e,add(IRType::I64,false,"@__reference_live_"+std::to_string(e)).index);
}
void Procedural::initialize_reference(EntityId e, Value location)
{
    EntityId object = sem.reference_scalar(e);
    Value target = binding(object);
    NodeId n = sem.entities[e].initializer;
    while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments) n = ast[n].first;
    auto c = sem.conversion_fact(sem.expression_fact(n).incoming);
    Value pointer;
    if (c.kind == semantic::Conversion::Kind::User) {
        pointer = address(target); user_conversion(n,c,pointer);
    } else {
        c.reference = c.temporary = false; c.target = sem.entities[object].type;
        store(converted(n,c),target); pointer = address(target);
    }
    store(pointer,location);
}
Value Procedural::abort_call()
{
    if (!abort_symbol) {
        Function f; f.symbol = fresh_symbol("@__builtin_abort"); f.declaration = true;
        auto t = sem.types.function(sem.types.fundamental(FT_VOID),{},false);
        FunctionId owner(p.functions.size()+1); f.signature = signature(t,owner);
        auto& boundary = p.signatures[f.signature.index-1].boundary;
        boundary.unwind = ir_model::CUM_NO; boundary.returns = ir_model::CRM_NORETURN;
        p.functions.push_back(f); abort_symbol = f.symbol;
        auto& s = p.symbols[f.symbol.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = owner.index;
        s.metadata.role = ir_model::SR_TERMINATE; s.metadata.object = p.intern("abort");
    }
    emit(Opcode::Call,IRType::Void,{Operand::symbol(abort_symbol)});
    return emit(Opcode::Unreachable,IRType(),{});
}
} }
