#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::closure_adapter(EntityId e)
{
    auto closure = sem.closure_adapter(e);
    function = FunctionId(p.symbols[symbol(e).index-1].entity);
    builder.reset(new lowir_model::FunctionBuilder(p,function));
    reset_lifetime(e); start(block());
    emit(Opcode::Return,IRType::Ptr,{Operand::symbol(symbol(closure.thunk))});
    builder.reset();
}
} }
