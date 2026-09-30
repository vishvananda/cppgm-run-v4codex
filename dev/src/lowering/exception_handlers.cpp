#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
ir_model::CatchBinding Procedural::catch_binding(NodeId handler) const
{
    return linkage.presentation ? CatchBinding::Value : CatchBinding(sem.handler_bindings.get(handler));
}
unsigned Procedural::exception_selector(NodeId handler)
{
    auto key = (std::uint64_t(sem.facts[handler].type) << 2) + unsigned(catch_binding(handler)) + 1;
    if (auto old = exception_selectors.get(key)) return old;
    auto value = ++exception_selector_count; exception_selectors.put(key,value); return value;
}
Value Procedural::begin_catch(Operand object, NodeId handler)
{
    auto function = Operand::symbol(exception_function(1));
    if (linkage.presentation || linkage.host) return emit(Opcode::Call,IRType::Ptr,{function,object});
    Operand storage = Operand::null();
    // A converted const pointer reference owns its temporary in this handler's
    // frame. Nested catches/rethrows must not overwrite another live binding.
    if (catch_binding(handler) == CatchBinding::ConstReference) {
        bool member = sem.types[sem.facts[handler].type].kind == TypeKind::MemberPointer;
        auto slot = builder->add_slot(0,member ? IRType::I128 : IRType::Ptr);
        storage = emit(Opcode::Addr,IRType(),{Operand::slot(slot)}).operand;
    }
    return emit(Opcode::Call,IRType::Ptr,{function,object,storage});
}
} }
