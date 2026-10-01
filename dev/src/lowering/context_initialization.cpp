#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::initialize_context_reference(const semantic::Analyzer::ContextReference& plan, Value location)
{
    auto t = sem.entities[plan.storage].type;
    if (!objects[plan.storage]) objects[plan.storage] = source_slot(plan.storage);
    Value storage(Operand::slot(objects[plan.storage]),type(t),t,true);
    if (plan.initialize) {
        if (sem.class_value(t)) initialize_constant_array(plan.storage,storage);
        else store(constant_operand(sem.entities[plan.storage].constant,t),storage);
    }
    auto pointer = address(storage);
    if (plan.offset) pointer = emit(Opcode::Index,IRType::I8,{pointer.operand,Operand::integer(plan.offset)});
    pointer.type = location.type; store(pointer,location);
}
} }
