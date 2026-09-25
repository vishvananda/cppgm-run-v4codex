#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
bool Procedural::discarded_access(NodeId n)
{
    ++discard_work;
    auto fact = sem.expression_fact(n);
    return fact.discarded_form && fact.category == ValueCategory::Lvalue;
}
void Procedural::discard(NodeId n, bool access)
{
    if (!n) return;
    const auto& discarded = sem.discarded_conversion(n);
    if (access && discarded.valid()) {
        auto temporary = sem.converted_temporary(discarded);
        auto destination = class_address(temporary,sem.entities[temporary].type);
        construct_value(n,discarded,destination);
        activate_temporary(temporary); return;
    }
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    if (sem.expression_fact(n).form == semantic::ExpressionForm::OperatorCall) { expression(n); return; }
    // A discarded address still evaluates its source, but a plain name or
    // dot projection has no value access of its own.
    if (!access && sem.expression_fact(n).form == semantic::ExpressionForm::Ordinary) {
        if (ast[n].kind == Kind::IdExpression) return;
        if (ast[n].kind == Kind::Member && ast[n].op != OP_ARROW) { discard(ast[n].first, false); return; }
    }
    if (ast[n].kind == Kind::Conditional) {
        // Prvalue arms undergo their normal conversions even if discarded.
        // A discarded volatile glvalue is read only for the forms in
        // [expr]/11; a conditional requires both arms to qualify.
        if (sem.expression_fact(n).category == ValueCategory::Prvalue) { expression(n); return; }
        access = access && discarded_access(n);
        NodeId a = ast[n].first, b = ast[a].next, c = ast[b].next;
        auto conversion = sem.conversion_fact(sem.expression_fact(n).conversions);
        Value test = conversion.kind == semantic::Conversion::Kind::User ? converted(a,conversion) : load(expression(a));
        test = truth_operand(test);
        if (test.ir.floating()) test = emit(Opcode::Compare, test.ir, {test.operand, Operand::floating(0)}, Operation::Ne);
        auto selector = cleanup_selector(test,cleanup_expression(b) || cleanup_expression(c));
        auto common = live;
        BlockId yes = block(), no = block(), end = block();
        emit(Opcode::Branch, IRType(), {test.operand, Operand::label(yes), Operand::label(no)});
        start(yes); discard(b, access); auto yes_live = live; jump(end);
        start(no); live = common; discard(c, access); auto no_live = live; jump(end);
        start(end); merge_temporaries(common,yes_live,no_live,selector); return;
    }
    if (ast[n].kind == Kind::Binary && ast[n].op == OP_COMMA) {
        discard(ast[n].first); discard(ast[ast[n].first].next, access); return;
    }
    Value value = expression(n);
    if (access && value.address && (sem.types[value.type].cv & 2) && discarded_access(n)) load(value);
}
void Procedural::condition(NodeId n, BlockId yes, BlockId no)
{
    if (ast[n].kind == Kind::Condition) {
        if (!ast[n].first) { jump(yes); return; }
        if (sem.facts[n].entity) {
            object(sem.facts[n].entity);
            auto c = sem.conversion_fact(sem.expression_fact(n).conversions);
            auto initial = live;
            if (c.kind == semantic::Conversion::Kind::User && live) {
                full_expression.enabled = full_expression.lexical = true; guard_expression(n);
            }
            Value v = c.kind == semantic::Conversion::Kind::User ? user_conversion(n,c) : load(binding(sem.facts[n].entity));
            v = truth_operand(v);
            if (v.ir.floating()) v = emit(Opcode::Compare, v.ir, {v.operand, Operand::floating(0)}, Operation::Ne);
            finish_full_expression(initial);
            emit(Opcode::Branch, IRType(), {v.operand, Operand::label(yes), Operand::label(no)}); return;
        }
        n = ast[n].first;
    }
    while (ast[n].kind == Kind::Parenthesized &&
        sem.conversion_fact(sem.expression_fact(n).incoming).kind != semantic::Conversion::Kind::User) n = ast[n].first;
    if (ast[n].kind == Kind::KeywordLiteral && (ast[n].op == KW_TRUE || ast[n].op == KW_FALSE)) {
        jump(ast[n].op == KW_TRUE ? yes : no); return;
    }
    // Semantic constant names carry the same boolean fact as a literal.
    // Consume that fact directly; this does not evaluate or rescan expressions.
    if (ast[n].kind == Kind::IdExpression && sem.facts[n].value &&
        type(sem.expression_fact(n).type).integer() &&
        !(sem.types[sem.expression_fact(n).type].cv & 2)) {
        auto value = sem.constant_fact(n);
        if (value.valid) { jump(value.bits ? yes : no); return; }
    }
    if (!cleanup_expression(n) && sem.expression_fact(n).form != semantic::ExpressionForm::OperatorCall && ast[n].kind == Kind::Binary && (ast[n].op == OP_LAND || ast[n].op == OP_LOR)) {
        BlockId rhs = block(); bool land = ast[n].op == OP_LAND;
        condition(ast[n].first, land ? rhs : yes, land ? no : rhs);
        start(rhs); condition(ast[ast[n].first].next, yes, no); return;
    }
    auto initial = live;
    begin_full_expression(n);
    auto incoming = sem.expression_fact(n).incoming;
    Value v = incoming && sem.conversion_fact(incoming).kind == semantic::Conversion::Kind::User ? converted(n,sem.conversion_fact(incoming)) : load(expression(n));
    v = truth_operand(v);
    if (v.ir.floating()) v = emit(Opcode::Compare, v.ir, {v.operand, Operand::floating(0)}, Operation::Ne);
    if (live != initial) {
        close_expression_region(); full_expression = FullExpression();
        auto state = live;
        auto yes_cleanup = block(), no_cleanup = block();
        emit(Opcode::Branch,IRType(),{v.operand,Operand::label(yes_cleanup),Operand::label(no_cleanup)});
        start(yes_cleanup); clean_inline(state,initial); jump(yes);
        start(no_cleanup); clean_inline(state,initial); jump(no); return;
    }
    finish_full_expression(initial);
    emit(Opcode::Branch, IRType(), {v.operand, Operand::label(yes), Operand::label(no)});
}
Value Procedural::conditional(NodeId n, bool location, Value destination, std::uint32_t branches, bool terminal, const semantic::ScalarConsumption* consumption)
{
    auto fact = sem.expression_fact(n);
    TypeId target = fact.type;
    bool supplied = destination.ir != IRType::Void;
    bool object = !consumption && (supplied || (sem.class_value(target) && fact.category == ValueCategory::Prvalue));
    TypeId consumed_type = consumption ? consumption->target : target;
    bool saved_scalar = full_expression.scalar_terminal, saved_unreachable = full_expression.scalar_unreachable;
    if (consumption) full_expression.scalar_terminal = true;
    location |= sem.types[target].kind == TypeKind::Array || sem.types[target].kind == TypeKind::Function;
    IRType ir = location ? IRType(IRType::Ptr) : type(consumed_type);
    bool has_result = ir != IRType::Void;
    NodeId a = ast[n].first, b = ast[a].next, c = ast[b].next;
    auto test_conversion = sem.conversion_fact(fact.conversions);
    bool summarized = test_conversion.kind == semantic::Conversion::Kind::User &&
        sem.conversion_result(test_conversion.function).valid;
    Value test;
    if (summarized) test = truth_operand(converted(a,test_conversion));
    // The runtime body proof does not make the source a C++ constant
    // expression. It does give this selected conversion a known branch after
    // evaluating its receiver; selected-arm conversions/lifetimes still run.
    if (summarized && test.operand.kind == Operand::Integer) {
        bool yes = test.operand.data.integer != 0;
        auto source = yes ? b : c;
        auto conversion = sem.conversion_fact((branches ? branches : fact.conversions+1) + !yes);
        auto common = live;
        bool enclosing_branch = full_expression.terminal_branch;
        full_expression.terminal_branch = terminal && object;
        Value result;
        if (object) {
            if (!supplied) destination = class_address(sem.object_fact(n).temporary,target);
            construct_value(source,conversion,destination,terminal);
            result = destination; result.type = target; result.address = true;
        } else {
            result = location || conversion.kind == semantic::Conversion::Kind::User ? converted(source,conversion) : convert(expression(source),target);
            if (consumption) result = converted_value(result,sem.conversion_fact(consumption->conversion));
            if (consumption && supplied) { store(result,destination); result = destination; }
            else { result.type = consumed_type; result.address = location; }
        }
        if (terminal) clean_inline(live,common);
        if (object && !supplied) activate_temporary(sem.object_fact(n).temporary);
        full_expression.terminal_branch = enclosing_branch;
        full_expression.scalar_terminal = saved_scalar;
        full_expression.scalar_unreachable = saved_unreachable;
        return result;
    }
    SlotId slot = has_result && !object && !supplied ? builder->add_slot(0, ir) : SlotId();
    if (object && !supplied) destination = class_address(sem.object_fact(n).temporary,target);
    BlockId yes = block(), no = block(), end = block();
    if (!summarized) {
        test = test_conversion.kind == semantic::Conversion::Kind::User ? converted(a,test_conversion) : load(expression(a));
        test = truth_operand(test);
    }
    if (test.ir.floating()) test = emit(Opcode::Compare,test.ir,{test.operand,Operand::floating(0)},Operation::Ne);
    SlotId selector = cleanup_selector(test,!terminal && (cleanup_expression(b,object) || cleanup_expression(c,object)));
    auto common = live;
    emit(Opcode::Branch, IRType(), {test.operand, Operand::label(yes), Operand::label(no)});
    start(yes);
    bool enclosing_branch = full_expression.terminal_branch;
    full_expression.terminal_branch = terminal && object;
    full_expression.scalar_unreachable = saved_unreachable || (consumption && consumption->truth == 1);
    if (object) construct_value(b,sem.conversion_fact(branches ? branches : fact.conversions+1),destination,terminal);
    else {
        auto conversion = sem.conversion_fact(fact.conversions+1);
        Value y = location || conversion.kind == semantic::Conversion::Kind::User ? converted(b,conversion) : convert(expression(b),target);
        if (consumption) y = converted_value(y,sem.conversion_fact(consumption->conversion));
        if (has_result) {
            if (consumption && supplied) store(y,destination);
            else if (ir.kind() == IRType::Object) store(y,Value(Operand::slot(slot),ir,target,true));
            else emit(Opcode::Store,ir,{y.operand,Operand::slot(slot)});
        }
    }
    if (terminal) clean_inline(live,common);
    auto yes_live = live; jump(end);
    start(no); live = common;
    full_expression.scalar_unreachable = saved_unreachable || (consumption && consumption->truth == 2);
    if (object) construct_value(c,sem.conversion_fact(branches ? branches+1 : fact.conversions+2),destination,terminal);
    else {
        auto conversion = sem.conversion_fact(fact.conversions+2);
        Value z = location || conversion.kind == semantic::Conversion::Kind::User ? converted(c,conversion) : convert(expression(c),target);
        if (consumption) z = converted_value(z,sem.conversion_fact(consumption->conversion));
        if (has_result) {
            if (consumption && supplied) store(z,destination);
            else if (ir.kind() == IRType::Object) store(z,Value(Operand::slot(slot),ir,target,true));
            else emit(Opcode::Store,ir,{z.operand,Operand::slot(slot)});
        }
    }
    if (terminal) clean_inline(live,common);
    auto no_live = live; jump(end); start(end);
    full_expression.terminal_branch = enclosing_branch;
    full_expression.scalar_terminal = saved_scalar; full_expression.scalar_unreachable = saved_unreachable;
    merge_temporaries(common,yes_live,no_live,selector);
    if (object) {
        if (!supplied) activate_temporary(sem.object_fact(n).temporary);
        destination.type = target; destination.address = true; return destination;
    }
    if (consumption && supplied) return destination;
    Value v = !has_result ? Value() : ir.kind() == IRType::Object ? Value(Operand::slot(slot),ir,target) : emit(Opcode::Load, ir, {Operand::slot(slot)});
    if (fact.category == ValueCategory::Prvalue && !location) v.materialized = slot;
    v.type = consumed_type; v.address = location; return v;
}
Value Procedural::logical(NodeId n)
{
    bool land = ast[n].op == OP_LAND;
    NodeId a = ast[n].first, b = ast[a].next;
    // Value context materializes a canonical truth slot. Condition context is
    // handled independently by condition(), without introducing that storage.
    auto fact = sem.expression_fact(n);
    auto first_conversion = sem.conversion_fact(fact.conversions), second_conversion = sem.conversion_fact(fact.conversions+1);
    Value lhs = first_conversion.kind == semantic::Conversion::Kind::User ? converted(a,first_conversion) : load(expression(a));
    lhs = truth_operand(lhs);
    if (lhs.operand.kind == Operand::Integer && (land ? !lhs.operand.data.integer : bool(lhs.operand.data.integer)))
        return Value(Operand::integer(!land), IRType::I64, sem.expression_fact(n).type);
    if (lhs.operand.kind == Operand::Integer) {
        Value rhs = second_conversion.kind == semantic::Conversion::Kind::User ? converted(b,second_conversion) : load(expression(b));
        rhs = truth_operand(rhs);
        IRType comparison = rhs.ir.floating() || rhs.ir == IRType::Ptr ? rhs.ir : IRType(IRType::I64);
        auto result = emit(Opcode::Compare,comparison,{rhs.operand,rhs.ir.floating() ? Operand::floating(0) : Operand::integer(0)},Operation::Ne);
        result.type = fact.type; return result;
    }
    SlotId slot = builder->add_slot(0, IRType::I64);
    BlockId rhs = block(), short_path = block(), end = block();
    if (lhs.ir.floating()) lhs = emit(Opcode::Compare, lhs.ir, {lhs.operand, Operand::floating(0)}, Operation::Ne);
    SlotId selector = cleanup_selector(lhs,cleanup_expression(b));
    auto common = live;
    emit(Opcode::Branch, IRType(), {lhs.operand, Operand::label(land ? rhs : short_path), Operand::label(land ? short_path : rhs)});
    start(rhs);
    Value value = second_conversion.kind == semantic::Conversion::Kind::User ? converted(b,second_conversion) : load(expression(b));
    value = truth_operand(value);
    IRType comparison = value.ir.floating() || value.ir == IRType::Ptr ? value.ir : IRType(IRType::I64);
    value = emit(Opcode::Compare, comparison, {value.operand, value.ir.floating() ? Operand::floating(0) : Operand::integer(0)}, Operation::Ne);
    emit(Opcode::Store, IRType::I64, {value.operand, Operand::slot(slot)});
    auto rhs_live = live; jump(end);
    start(short_path); live = common;
    emit(Opcode::Store, IRType::I64, {Operand::integer(!land), Operand::slot(slot)}); jump(end);
    start(end);
    merge_temporaries(common,land ? rhs_live : common,land ? common : rhs_live,selector);
    value = emit(Opcode::Load, IRType::I64, {Operand::slot(slot)});
    value.type = sem.expression_fact(n).type; return value;
}
void Procedural::collect_cases(NodeId n, std::vector<NodeId>& cases, NodeId& fallback)
{
    if (!n || ast[n].kind == Kind::Switch) return;
    if (ast[n].kind == Kind::Case) { labels[n] = block(); cases.push_back(n); }
    if (ast[n].kind == Kind::Default) { labels[n] = block(); fallback = n; }
    for (NodeId c = ast[n].first; c; c = ast[c].next) collect_cases(c, cases, fallback);
}
bool Procedural::mark_control_entries(NodeId n)
{
    if (!n) return false;
    ++control_work;
    Kind k = ast[n].kind;
    bool entry = k == Kind::Label || k == Kind::Case || k == Kind::Default;
    switch (k) {
    case Kind::Compound: case Kind::Then: case Kind::Else: case Kind::Label:
    case Kind::Case: case Kind::Default: case Kind::If: case Kind::Switch:
    case Kind::While: case Kind::Do: case Kind::For:
        for (NodeId c = ast[n].first; c; c = ast[c].next)
            entry |= mark_control_entries(c);
        break;
    default: break;
    }
    return control_entries[n] = entry;
}
void Procedural::switch_statement(NodeId n)
{
    NodeId cond = child(n, Kind::Condition), body = ast[cond].next;
    Value value;
    if (sem.facts[cond].entity) {
        object(sem.facts[cond].entity);
        auto c = sem.conversion_fact(sem.expression_fact(cond).conversions);
        auto initial = live;
        if (c.kind == semantic::Conversion::Kind::User && live) {
            full_expression.enabled = full_expression.lexical = true; guard_expression(cond);
        }
        value = c.kind == semantic::Conversion::Kind::User ? user_conversion(cond,c) : convert(binding(sem.facts[cond].entity), sem.facts[cond].type);
        finish_full_expression(initial);
    }
    else {
        auto initial = live; begin_full_expression(ast[cond].first);
        value = incoming(ast[cond].first); finish_full_expression(initial);
    }
    BlockId dispatch = block(), end = block(), saved = break_target;
    break_target = end;
    std::vector<NodeId> cases; NodeId fallback = 0;
    collect_cases(body, cases, fallback);
    jump(dispatch); start(dispatch);
    std::vector<Operand> operands{value.operand, Operand::label(fallback ? labels[fallback] : end)};
    for (NodeId c : cases) {
        auto constant = sem.constant_fact(ast[c].first);
        if (!constant.valid) throw std::logic_error("missing semantic case value");
        operands.push_back(Operand::integer(constant.bits)); operands.push_back(Operand::label(labels[c]));
    }
    emit(Instruction(Opcode::Switch), operands);
    statement(body); jump(end); start(end); break_target = saved;
}
void Procedural::statement(NodeId n)
{
    if (!n) return;
    Kind k = ast[n].kind;
    auto lifetime = sem.lifetime_use(n); live = lifetime.entry;
    if (k == Kind::Compound || k == Kind::Then || k == Kind::Else) {
        for (NodeId c = ast[n].first; c; c = ast[c].next) statement(c);
        if (!ended) clean_inline(lifetime.exit, lifetime.entry);
        live = lifetime.entry; return;
    }
    if (k == Kind::Label) {
        if (!labels[n]) labels[n] = block();
        jump(labels[n]); start(labels[n]); statement(ast[n].first); return;
    }
    if (k == Kind::Case || k == Kind::Default) {
        jump(labels[n]); start(labels[n]);
        statement(k == Kind::Case ? ast[ast[n].first].next : ast[n].first); return;
    }
    if (ended) {
        if (!control_entries[n]) return;
        // A terminator kills fallthrough, not explicit entries nested in a
        // control statement. Its header starts in an unreachable block;
        // labels inside it still receive their normal goto/switch edges.
        start(block());
    }
    switch (k) {
    case Kind::Class:
        if (auto e = sem.anonymous_object(n)) object(e);
        return;
    case Kind::SimpleDeclaration: {
        if (auto e = sem.anonymous_object(n)) object(e);
        NodeId list = child(n, Kind::InitDeclarators);
        for (NodeId item = ast[list].first; item; item = ast[item].next) {
            EntityId e = sem.facts[ast[item].first].entity;
            if (e && sem.entities[e].kind == semantic::EntityKind::Variable) object(e);
        }
        return;
    }
    case Kind::ExpressionStatement: case Kind::Iteration: case Kind::ForInit:
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::SimpleDeclaration) statement(c);
            else { begin_full_expression(c); discard(c); finish_full_expression(lifetime.entry); }
        }
        return;
    case Kind::Return: return_statement(n); return;
    case Kind::Goto: {
        NodeId target = sem.facts[n].target;
        if (!labels[target]) labels[target] = block();
        clean_inline(lifetime.entry, lifetime.target);
        jump(labels[target]); return;
    }
    case Kind::Break: clean_inline(lifetime.entry, lifetime.target); jump(break_target); return;
    case Kind::Continue: clean_inline(lifetime.entry, lifetime.target); jump(continue_target); return;
    case Kind::If: {
        BlockId yes = block(), no = block(), end;
        condition(child(n, Kind::Condition), yes, no);
        start(yes); statement(child(n, Kind::Then));
        if (!ended) { end = block(); jump(end); }
        start(no); statement(child(n, Kind::Else));
        if (!ended) { if (!end) end = block(); jump(end); }
        if (end) { start(end); clean_inline(lifetime.exit, lifetime.entry); }
        return;
    }
    case Kind::While: case Kind::For: case Kind::Do: {
        BlockId cond = block(), body = block(), step = k == Kind::For ? block() : cond, end = block();
        BlockId exit_cleanup = lifetime.exit != lifetime.entry ? block() : end;
        BlockId old_break = break_target, old_continue = continue_target;
        break_target = end; continue_target = step;
        if (k == Kind::For) statement(child(n, Kind::ForInit));
        if (k != Kind::Do) {
            jump(cond); start(cond); condition(child(n, Kind::Condition), body, exit_cleanup);
        } else jump(body);
        start(body);
        NodeId body_node = k == Kind::Do ? ast[n].first : ast[n].last;
        statement(body_node);
        if (!ended) clean_inline(live, sem.lifetime_use(body_node).entry);
        jump(step);
        if (k == Kind::For) { start(step); statement(child(n, Kind::Iteration)); jump(cond); }
        if (k == Kind::Do) { start(cond); condition(child(n, Kind::Condition), body, exit_cleanup); }
        if (exit_cleanup.index != end.index) { start(exit_cleanup); clean_inline(lifetime.exit, lifetime.entry); jump(end); }
        start(end); live = lifetime.entry; break_target = old_break; continue_target = old_continue; return;
    }
    case Kind::Switch: switch_statement(n); return;
    default: return;
    }
}
} }
