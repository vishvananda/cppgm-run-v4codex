#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using semantic::InitKind;
bool Procedural::array_parameter(TypeId type)
{
    if (sem.types[type].kind != TypeKind::Array) return false;
    if (auto known = aggregate_array_parameters.get(type)) { ++aggregate_array_hits; return known == 2; }
    ++aggregate_array_work;
    auto element = sem.types[type].child;
    // The helper transports the representation of scalar array members only.
    // Class construction and volatile element accesses keep their ordered path.
    // Canonical types are immutable; each shared array tail is classified once
    // for this translation unit, independently of initializer multiplicity.
    bool safe = sem.types[element].kind == TypeKind::Array ? array_parameter(element) :
        !(sem.types[element].cv & 2) && !sem.class_value(element);
    aggregate_array_parameters.put(type,safe ? 2 : 1); return safe;
}
namespace {
bool full_parameters(semantic::Analyzer& sem, std::uint32_t plan)
{
    bool omitted = false;
    for (auto c = sem.initializers[plan].first; c; c = sem.initializers[c].next) {
        auto item = sem.initializers[c];
        if (item.helper_copy || sem.types[item.type].kind == TypeKind::Array || (omitted && item.source)) return true;
        omitted |= !item.source;
    }
    return false;
}
}
SymbolId Procedural::aggregate_helper(std::uint32_t plan)
{
    auto action = sem.initializers[plan];
    TypeId target = action.type;
    unsigned supplied = 0; bool transfer = false;
    bool all = full_parameters(sem,plan);
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        supplied += all || sem.initializers[child].source != 0;
        transfer |= sem.initializers[child].helper_transfer != 0;
    }
    // Scalar helpers differ by their explicit prefix; omitted trailing fields
    // are initialized in the helper. Transfer recipes additionally own selected
    // constructors and temporary identities, so those helpers belong to the
    // complete semantic plan instead of being shared by target type alone.
    // Proven representation copies use function-local slots, so their full
    // parameter shape is shared across initializer occurrences of this type.
    auto identity = (std::uint64_t(target) << 32) | (transfer ? (std::uint32_t(1) << 31) | plan : supplied);
    if (auto known = aggregate_helpers.get(identity)) return p.functions[known-1].symbol;
    AggregateHelper helper; helper.type = target; helper.actions = aggregate_actions.size(); helper.count = 0;
    std::vector<TypeId> parameters(1, sem.types.compound(TypeKind::Pointer, target));
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        aggregate_actions.push_back(child); ++helper.count;
        if (all || item.source) parameters.push_back(array_parameter(item.type) ?
            sem.types.compound(TypeKind::Pointer,item.type) : item.type);
    }
    helper.function = FunctionId(p.functions.size()+1);
    lowir_model::Function f; f.symbol = fresh_symbol("@__aggregate_" + std::to_string(target));
    f.signature = signature(sem.types.function(sem.types.fundamental(FT_VOID), parameters, false), helper.function);
    bool throwing = false;
    for (auto child = action.first; child; child = sem.initializers[child].next)
        throwing |= sem.initializers[child].helper_transfer != 0;
    if (!throwing) p.signatures[f.signature.index-1].boundary.unwind = ir_model::CUM_NO;
    p.functions.push_back(f);
    auto& symbol = p.symbols[f.symbol.index-1]; symbol.kind = lowir_model::Symbol::FunctionSymbol;
    symbol.entity = helper.function.index; symbol.metadata.binding = ir_model::SBM_INTERNAL;
    aggregate_helpers.put(identity, helper.function.index); aggregate_definitions.push_back(helper);
    return f.symbol;
}
bool Procedural::call_aggregate_helper(std::uint32_t plan, Value location)
{
    auto action = sem.initializers[plan];
    if (action.kind != InitKind::Group || sem.types[action.type].kind != TypeKind::Named) return false;
    // A single scalar/reference argument precedes its sole destination store;
    // there is no earlier field initialization for it to observe. Array
    // arguments still need the proof for their own element stores.
    auto first = sem.initializers[action.first];
    if (!action.helper_safe && (first.next || sem.types[first.type].kind == TypeKind::Array)) return false;
    // An empty aggregate has no initialization actions. Its caller still owns
    // distinct object storage, but there is no helper body or call to emit.
    if (!action.first) return true;
    bool all = full_parameters(sem,plan);
    bool transfer = false;
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        if (!item.field) return false;
        // A by-value class argument is initialized before the helper call.
        // Its transfer into the member must precede later initializer clauses.
        // A transfer may precede later clauses only with a checked proof that
        // it cannot throw or publish observable effects outside these objects.
        if (item.helper_transfer && item.next && !item.helper_commutes) return false;
        transfer |= item.helper_transfer && !sem.trivial_destructor(item.type);
        if (array_parameter(item.type)) continue;
        if ((!type(item.type).scalar() && !item.helper_transfer && !item.helper_copy) ||
            (item.kind != InitKind::Scalar && item.kind != InitKind::Converted && !(all && item.kind == InitKind::Value))) return false;
    }
    if (transfer) {
        // The helper owns destructible by-value parameters. Retain the O0
        // protected argument boundary, including an empty caller live prefix.
        full_expression.enabled = true; open_expression_region();
    }
    SymbolId callee = aggregate_helper(plan);
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(callee)); call_work.push_back(address(location).operand);
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        if (!all && !item.source) continue;
        if (array_parameter(item.type)) {
            auto slot = builder->add_slot(0,type(item.type));
            Value at = address(Value(Operand::slot(slot),type(item.type),item.type,true));
            at.address = true; at.type = item.type;
            initialize_plan(child,at);
            call_work.push_back(at.operand);
        } else call_work.push_back((item.kind == InitKind::Converted ? converted(item.source,sem.conversion_fact(item.conversion)) : initialization_value(item.source,item.type)).operand);
    }
    guarded_call(Instruction(Opcode::Call, IRType::Void), call_work.data()+begin, call_work.size()-begin);
    call_work.resize(begin); return true;
}
void Procedural::emit_aggregate_helpers()
{
    for (auto helper : aggregate_definitions) {
        function = helper.function; reset_lifetime(0); initialized_units = semantic::Index();
        builder.reset(new lowir_model::FunctionBuilder(p, function)); start(block());
        auto signature = p.signatures[p.functions[function.index-1].signature.index-1];
        std::vector<SlotId> slots;
        for (unsigned j = 0; j < signature.parameters.count; ++j) {
            auto parameter = p.parameters[signature.parameters.begin+j];
            auto action = j ? sem.initializers[aggregate_actions[helper.actions+j-1]] : semantic::InitAction();
            auto slot = builder->add_slot(0, j && !array_parameter(action.type) ? type(action.type) : parameter.type); slots.push_back(slot);
            if (action.helper_copy) {
                if (!sem.empty_class(action.type)) {
                    Value at = address(Value(Operand::slot(slot),type(action.type),action.type,true));
                    Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(action.type); copy.alignment = sem.object_alignment(action.type);
                    emit(copy,{Operand::value(parameter.value),at.operand});
                }
            } else if (action.helper_parameter) {
                objects[action.helper_parameter] = slot;
                object_addresses[action.helper_parameter] = lowir_model::ValueId();
                if (sem.indirect_parameter(action.type)) object_addresses[action.helper_parameter] = parameter.value;
                else if (!sem.empty_class(action.type)) {
                    Value at = address(Value(Operand::slot(slot),type(action.type),action.type,true));
                    Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(action.type); copy.alignment = sem.object_alignment(action.type);
                    emit(copy,{Operand::value(parameter.value),at.operand});
                }
                activate_temporary(action.helper_parameter);
            } else emit(Opcode::Store, parameter.type, {Operand::value(parameter.value), Operand::slot(slot)});
        }
        auto parameters = live;
        semantic::Index retired;
        for (unsigned j = 0; j < helper.count; ++j) {
            auto action = sem.initializers[aggregate_actions[helper.actions+j]];
            EntityId field = action.field; TypeId t = action.type;
            Value value;
            bool array = array_parameter(t);
            if (!action.helper_transfer && !action.helper_copy && !array) {
                value = j+1 < slots.size() ? emit(Opcode::Load,type(t),{Operand::slot(slots[j+1])}) : initialization_value(0,t);
                value.type = t;
            }
            Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(slots[0])});
            Instruction index(Opcode::Index, IRType::I8); index.projection = ir_model::IPK_FIELD;
            Value at = emit(index, {base.operand, Operand::integer(sem.entities[field].member_offset)});
            at.type = t; at.address = true; at.init_offset = sem.entities[field].member_offset; at.initializing = true;
            if (sem.field_fact(field).bit_field) at.bit_field = field;
            if (array) {
                auto source = emit(Opcode::Load,IRType::Ptr,{Operand::slot(slots[j+1])});
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(t); copy.alignment = sem.object_alignment(t);
                emit(copy,{source.operand,at.operand});
            } else if (action.helper_transfer || action.helper_copy) {
                Value source = action.helper_copy ? address(Value(Operand::slot(slots[j+1]),type(t),t,true)) :
                    address(binding(action.helper_parameter));
                if (action.helper_copy || sem.direct_transfer(action.helper_transfer)) {
                    if (!sem.empty_class(t)) {
                        Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(t); copy.alignment = sem.object_alignment(t);
                        emit(copy,{source.operand,at.operand});
                    }
                } else {
                    auto transfer = sem.conversion_objects[sem.conversion_fact(action.conversion).materialization].call;
                    std::size_t begin = call_work.size();
                    call_work.push_back(Operand::symbol(symbol(action.helper_transfer))); call_work.push_back(at.operand); call_work.push_back(source.operand);
                    for (unsigned k = 1; k < transfer.argument_count; ++k)
                        call_work.push_back(converted(sem.call_argument(transfer,k),sem.conversion_fact(transfer.conversions+k)).operand);
                    guarded_call(Instruction(Opcode::Call,IRType::Void),call_work.data()+begin,call_work.size()-begin); call_work.resize(begin);
                }
            } else store(value, at);
            if (!sem.initializer_suffix_nonthrowing(action.next))
                activate_subobject(t,address(at),SlotId(),retired);
        }
        if (!retired.empty()) {
            close_expression_region(); semantic::Index cache;
            live = retire_construction(live,parameters,retired,cache);
        }
        // The aggregate is complete, but argument destruction can still fail
        // before this helper returns ownership to its caller.
        auto temporaries = live; live = 0;
        bool throwing_cleanup = false;
        for (unsigned j = 0; j < helper.count; ++j) {
            auto item = sem.initializers[aggregate_actions[helper.actions+j]];
            throwing_cleanup |= item.helper_parameter && !sem.function_nonthrowing(sem.object_destructor(item.helper_parameter));
            throwing_cleanup |= item.helper_transfer && sem.types[sem.entities[item.helper_transfer].type].count > 1;
        }
        if (temporaries && throwing_cleanup && sem.destructor_needed(sem.type_destructor(helper.type))) {
            Value target = emit(Opcode::Load,IRType::Ptr,{Operand::slot(slots[0])});
            activate_subobject(helper.type,target,SlotId(),retired);
        }
        auto completed = live;
        if (temporaries) {
            semantic::Index empty, cache;
            live = retire_construction(temporaries,0,empty,cache,completed);
        }
        clean_inline(live,completed); live = 0;
        emit(Opcode::Return, IRType(), {}); emit_cleanups(); builder.reset();
    }
}
} }
