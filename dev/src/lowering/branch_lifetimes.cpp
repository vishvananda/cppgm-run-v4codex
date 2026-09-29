#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
const semantic::Expression* Procedural::conversion_call(const semantic::Conversion& conversion) const
{
    auto c = &conversion;
    if (c->kind == semantic::Conversion::Kind::User)
        c = &sem.user_conversions[c->materialization].result;
    if (c->kind == semantic::Conversion::Kind::Construction) {
        const auto& object = sem.conversion_objects[c->materialization];
        return object.elided ? nullptr : &object.call;
    }
    if (c->kind == semantic::Conversion::Kind::List)
        return &sem.list_objects[c->materialization].call;
    return nullptr;
}
bool Procedural::cleanup_expression(NodeId n, bool omit_result, bool effects_only)
{
    if (!n) return false;
    if (cleanup_expressions.empty()) cleanup_expressions.resize(ast.nodes.size()*2);
    auto key = n*2+omit_result;
    // Both lazy facts fit in the existing byte per result-omission mode.
    // A proven absence of cleanup also proves absence of observable cleanup.
    auto shift = effects_only ? 2 : 0;
    auto cached = (cleanup_expressions[key] >> shift) & 3;
    if (cached) return cached == 2;
    if (effects_only && (cleanup_expressions[key] & 3) == 1) return false;
    auto cleanup = [&](EntityId object) {
        return sem.temporary_cleanup(object) && (!effects_only || sem.destructor_needed(sem.object_destructor(object)));
    };
    ++full_expression_work;
    auto temporary = sem.object_fact(n).temporary;
    bool needed = !omit_result && !sem.object_lifetime(temporary) && cleanup(temporary);
    const auto& discarded = sem.discarded_conversion(n);
    if (discarded.valid()) needed |= cleanup(sem.converted_temporary(discarded));
    auto arrow = sem.arrow_chains[sem.object_fact(n).arrow];
    for (unsigned j = 0; j < arrow.count; ++j) needed |= cleanup(sem.arrow_steps[arrow.first+j].temporary);
    auto expression = sem.expression_fact(n);
    auto incoming = expression.incoming;
    auto backing_cleanup = [&](const semantic::Conversion& c) {
        if (c.kind != semantic::Conversion::Kind::List) return false;
        auto backing = sem.list_objects[c.materialization].backing;
        return backing && !sem.object_lifetime(backing) && !sem.static_temporary(backing).object && cleanup(backing);
    };
    if (incoming) needed |= backing_cleanup(sem.conversion_fact(incoming));
    if (incoming && !omit_result) {
        auto c = sem.conversion_fact(incoming);
        if (c.kind == semantic::Conversion::Kind::User)
            needed |= cleanup(sem.user_conversions[c.materialization].source_temporary);
        if ((c.reference || c.ellipsis_object) && c.materialization) {
            auto object = sem.converted_temporary(c);
            needed |= !sem.object_lifetime(object) && cleanup(object);
        }
    }
    // Default arguments are semantic call edges outside the caller's syntax.
    // Include constructor and conversion-function defaults as well as calls.
    auto arguments = [&](const semantic::Expression& call) {
        for (unsigned i = 0; i < call.argument_count; ++i) {
            auto a = sem.call_argument(call,i);
            bool omit = omit_result && (ast[n].kind == syntax::Kind::Parenthesized || ast[n].kind == syntax::Kind::Initializer ||
                (ast[n].kind == syntax::Kind::Conditional && a != ast[n].first));
            if (a && a != n) needed |= cleanup_expression(a,omit,effects_only);
        }
    };
    arguments(expression);
    // Destination initialization owns its selected conversion independently of
    // the source expression (for example a converting constructor from a
    // string literal). Its defaults are full-expression lifetime edges too.
    auto initialization = sem.class_initialization(n,sem.facts[n].type);
    if (initialization.source)
        if (auto call = conversion_call(sem.conversion_fact(initialization.conversion))) arguments(*call);
    if (discarded.valid()) if (auto call = conversion_call(discarded)) arguments(*call);
    if (incoming) if (auto call = conversion_call(sem.conversion_fact(incoming))) arguments(*call);
    for (unsigned i = 0; i < expression.count; ++i) {
        const auto& c = sem.conversion_fact(expression.conversions+i);
        needed |= backing_cleanup(c);
        if (c.ellipsis_object) needed |= cleanup(sem.converted_temporary(c));
        if (auto call = conversion_call(c)) arguments(*call);
    }
    if (ast[n].kind == syntax::Kind::Lambda)
        for (auto i = sem.closure(sem.types[expression.type].entity).first_capture; i; i = sem.closure_captures[i].next)
            if (auto call = conversion_call(sem.conversion_fact(sem.closure_captures[i].conversion))) arguments(*call);
    if (expression.form == semantic::ExpressionForm::Typeid && sem.rtti_expression(n).dynamic)
        needed |= cleanup_expression(ast[n].first,false,effects_only);
    if (ast[n].kind != syntax::Kind::Lambda && ast[n].kind != syntax::Kind::Sizeof && ast[n].kind != syntax::Kind::TypeTrait)
    for (NodeId child = ast[n].first; child; child = ast[child].next) {
        bool omit = omit_result && (ast[n].kind == syntax::Kind::Parenthesized || ast[n].kind == syntax::Kind::Initializer ||
            (ast[n].kind == syntax::Kind::Conditional && child != ast[n].first));
        needed |= cleanup_expression(child,omit,effects_only);
    }
    cleanup_expressions[key] |= (needed ? 2 : 1) << shift;
    return needed;
}
SlotId Procedural::cleanup_selector(Value test, bool required)
{
    if (!required) return SlotId();
    Value truth = emit(Opcode::Compare,test.ir,{test.operand,test.ir.floating() ? Operand::floating(0) : Operand::integer(0)},Operation::Ne);
    SlotId slot = builder->add_slot(0,IRType::I64);
    emit(Opcode::Store,IRType::I64,{truth.operand,Operand::slot(slot)});
    return slot;
}
void Procedural::merge_temporaries(std::uint32_t common, std::uint32_t yes, std::uint32_t no, SlotId selector)
{
    if (yes == no) { live = yes; return; }
    if (!selector) throw std::logic_error("missing conditional lifetime selector");
    TemporaryState state; state.tail = common; state.depth = lifetime_state(common).depth+1;
    state.selector = selector; state.yes = yes; state.no = no;
    temporary_states.push_back(state); live = 0x80000000u | temporary_states.size();
}
void Procedural::destroy_lifetime(std::uint32_t id)
{
    auto action = lifetime_state(id);
    if (action.object) {
        auto temporary = id & 0x80000000u ? temporary_states[(id & 0x7fffffffu)-1] : TemporaryState();
        auto location = temporary.location;
        if (temporary.constructed) {
            auto leaf = sem.entities[action.object].type;
            while (sem.types[leaf].kind == TypeKind::Array) leaf = sem.types[leaf].child;
            auto test = block(), body = block(), end = block(); jump(test); start(test);
            auto count = emit(Opcode::Load,IRType::I64,{Operand::slot(temporary.constructed)});
            auto more = emit(Opcode::Compare,IRType::I64,{count.operand,Operand::integer(0)},Operation::Ne);
            emit(Opcode::Branch,IRType(),{more.operand,Operand::label(body),Operand::label(end)});
            start(body);
            auto next = emit(Opcode::Binary,IRType::I64,{count.operand,Operand::integer(1)},Operation::Sub);
            emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(temporary.constructed)});
            destroy(action.destructor,leaf,array_element(Value(Operand::value(location),IRType::Ptr),false,{},next.operand,sem.object_size(leaf)));
            jump(test); start(end); return;
        }
        if (location) destroy(action.destructor,sem.entities[action.object].type,Value(Operand::value(location),IRType::Ptr));
        else destroy_object(action.object,action.destructor);
        return;
    }
    auto branch = temporary_states[(id & 0x7fffffffu)-1];
    Value test = emit(Opcode::Load,IRType::I64,{Operand::slot(branch.selector)});
    BlockId yes = block(), no = block(), end = block();
    emit(Opcode::Branch,IRType(),{test.operand,Operand::label(yes),Operand::label(no)});
    start(yes); clean_inline(branch.yes,branch.tail); jump(end);
    start(no); clean_inline(branch.no,branch.tail); jump(end);
    start(end); live = branch.tail;
}
} }
