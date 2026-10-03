#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
SymbolId Procedural::member_function_symbol(EntityId member)
{
    auto slot = sem.member_fact(member).virtual_slot;
    auto target = symbol(member);
    if (!slot) return target;
    if (auto known = linkage.member_thunk_symbols.get(target.index)) return SymbolId(known);
    // The source-to-LowIR member-pointer ABI is a callable address plus a
    // receiver displacement. A virtual member's callable performs dispatch;
    // a conversion changes only the displacement, even for a nonpolymorphic
    // named owner. No caller must guess the representation from that owner.
    auto output = fresh_symbol("@virtual_member");
    linkage.member_thunk_symbols.put(target.index,output.index);
    auto& metadata = p.symbols[output.index-1].metadata;
    metadata.binding = ir_model::SBM_INTERNAL;
    metadata.object = p.intern("__cppgm_virtual_member_"+std::to_string(output.index));
    Function f; f.symbol = output;
    FunctionId owner(p.functions.size()+1); f.signature = signature(sem.call_type(member),owner);
    p.functions.push_back(f);
    auto& s = p.symbols[output.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = owner.index;
    member_thunks.push_back({member,output,slot});
    return output;
}
void Procedural::emit_member_thunks()
{
    for (auto thunk : member_thunks) {
        function = FunctionId(p.symbols[thunk.symbol.index-1].entity);
        if (p.functions[function.index-1].blocks.count) continue;
        builder.reset(new FunctionBuilder(p,function)); reset_lifetime(thunk.member);
        returned = sem.types[sem.entities[thunk.member].type].child;
        start(block());
        auto sig = p.signatures[p.functions[function.index-1].signature.index-1];
        std::vector<Operand> args(1);
        for (unsigned j = 0; j < sig.parameters.count; ++j)
            args.push_back(Operand::value(p.parameters[sig.parameters.begin+j].value));
        auto receiver = 1+unsigned(sem.indirect_value(returned));
        args[0] = virtual_function(Value(args[receiver],IRType::Ptr),thunk.slot).operand;
        Instruction call(Opcode::Call,sig.result); call.signature = virtual_signature(thunk.member);
        auto result = emit_raw(call,args.data(),args.size());
        if (sig.result == IRType::Void) emit(Opcode::Return,IRType(),{});
        else emit(Opcode::Return,sig.result,{result.operand});
        builder.reset();
    }
}
} }
