#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
bool Procedural::discarded_access(NodeId n)
{
    // Immutable parsed forms: memoize once per NodeId, including negative
    // results, so nested discarded conditionals do not rescan their subtrees.
    if (discard_accesses.empty()) discard_accesses.resize(ast.nodes.size());
    if (discard_accesses[n]) return discard_accesses[n] == 2;
    ++discard_work;
    Kind k = ast[n].kind;
    bool access;
    if (k == Kind::Conditional) {
        NodeId b = ast[ast[n].first].next;
        access = discarded_access(b) && discarded_access(ast[b].next);
    } else if (k == Kind::Parenthesized) access = discarded_access(ast[n].first);
    else if (k == Kind::Binary && ast[n].op == OP_COMMA) access = discarded_access(ast[ast[n].first].next);
    else access = k == Kind::IdExpression || k == Kind::Member || k == Kind::Subscript ||
        (k == Kind::Unary && ast[n].op == OP_STAR);
    discard_accesses[n] = access ? 2 : 1;
    return access;
}
void Procedural::discard(NodeId n, bool access)
{
    if (!n) return;
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    if (sem.expression_fact(n).form == semantic::ExpressionForm::OperatorCall) { expression(n); return; }
    if (ast[n].kind == Kind::Conditional) {
        // Prvalue arms undergo their normal conversions even if discarded.
        // A discarded volatile glvalue is read only for the forms in
        // [expr]/11; a conditional requires both arms to qualify.
        if (sem.expression_fact(n).category == ValueCategory::Prvalue) { expression(n); return; }
        access = access && discarded_access(n);
        NodeId a = ast[n].first, b = ast[a].next, c = ast[b].next;
        Value test = load(expression(a));
        if (test.ir.floating()) test = emit(Opcode::Compare, test.ir, {test.operand, Operand::floating(0)}, Operation::Ne);
        BlockId yes = block(), no = block(), end = block();
        emit(Opcode::Branch, IRType(), {test.operand, Operand::label(yes), Operand::label(no)});
        start(yes); discard(b, access); jump(end);
        start(no); discard(c, access); jump(end); start(end); return;
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
            Value v = c.kind == semantic::Conversion::Kind::User ? user_conversion(n,c) : load(binding(sem.facts[n].entity));
            if (v.ir.floating()) v = emit(Opcode::Compare, v.ir, {v.operand, Operand::floating(0)}, Operation::Ne);
            emit(Opcode::Branch, IRType(), {v.operand, Operand::label(yes), Operand::label(no)}); return;
        }
        n = ast[n].first;
    }
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    if (!cleanup_expression(n) && sem.expression_fact(n).form != semantic::ExpressionForm::OperatorCall && ast[n].kind == Kind::Binary && (ast[n].op == OP_LAND || ast[n].op == OP_LOR)) {
        BlockId rhs = block(); bool land = ast[n].op == OP_LAND;
        condition(ast[n].first, land ? rhs : yes, land ? no : rhs);
        start(rhs); condition(ast[ast[n].first].next, yes, no); return;
    }
    auto initial = live;
    auto incoming = sem.expression_fact(n).incoming;
    Value v = incoming && sem.conversion_fact(incoming).kind == semantic::Conversion::Kind::User ? converted(n,sem.conversion_fact(incoming)) : load(expression(n));
    if (v.ir.floating()) v = emit(Opcode::Compare, v.ir, {v.operand, Operand::floating(0)}, Operation::Ne);
    clean_inline(live,initial);
    emit(Opcode::Branch, IRType(), {v.operand, Operand::label(yes), Operand::label(no)});
}
Value Procedural::conditional(NodeId n, bool location, Value destination, std::uint32_t branches)
{
    auto fact = sem.expression_fact(n);
    TypeId target = fact.type;
    bool supplied = destination.ir != IRType::Void;
    bool object = supplied || (sem.class_value(target) && fact.category == ValueCategory::Prvalue);
    location |= sem.types[target].kind == TypeKind::Array || sem.types[target].kind == TypeKind::Function;
    IRType ir = location ? IRType(IRType::Ptr) : type(target);
    bool has_result = ir != IRType::Void;
    SlotId slot = has_result && !object ? builder->add_slot(0, ir) : SlotId();
    if (object && !supplied) destination = class_address(sem.object_fact(n).temporary,target);
    BlockId yes = block(), no = block(), end = block();
    NodeId a = ast[n].first, b = ast[a].next, c = ast[b].next;
    auto test_conversion = sem.conversion_fact(fact.conversions);
    Value test = test_conversion.kind == semantic::Conversion::Kind::User ? converted(a,test_conversion) : load(expression(a));
    if (test.ir.floating()) test = emit(Opcode::Compare,test.ir,{test.operand,Operand::floating(0)},Operation::Ne);
    SlotId selector = cleanup_selector(test,cleanup_expression(b) || cleanup_expression(c));
    auto common = live;
    emit(Opcode::Branch, IRType(), {test.operand, Operand::label(yes), Operand::label(no)});
    start(yes);
    if (object) construct_value(b,sem.conversion_fact(branches ? branches : fact.conversions+1),destination);
    else {
        Value y = location ? converted(b,sem.conversion_fact(fact.conversions+1)) : convert(expression(b),target);
        if (has_result) emit(Opcode::Store,ir,{y.operand,Operand::slot(slot)});
    }
    auto yes_live = live; jump(end);
    start(no); live = common;
    if (object) construct_value(c,sem.conversion_fact(branches ? branches+1 : fact.conversions+2),destination);
    else {
        Value z = location ? converted(c,sem.conversion_fact(fact.conversions+2)) : convert(expression(c),target);
        if (has_result) emit(Opcode::Store,ir,{z.operand,Operand::slot(slot)});
    }
    auto no_live = live; jump(end); start(end);
    merge_temporaries(common,yes_live,no_live,selector);
    if (object) {
        if (!supplied) activate_temporary(sem.object_fact(n).temporary);
        destination.type = target; destination.address = true; return destination;
    }
    Value v = has_result ? emit(Opcode::Load, ir, {Operand::slot(slot)}) : Value();
    v.type = target; v.address = location; return v;
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
    if (lhs.operand.kind == Operand::Integer && (land ? !lhs.operand.data.integer : bool(lhs.operand.data.integer)))
        return Value(Operand::integer(!land), IRType::I64, sem.expression_fact(n).type);
    SlotId slot = builder->add_slot(0, IRType::I64);
    BlockId rhs = block(), short_path = block(), end = block();
    if (lhs.ir.floating()) lhs = emit(Opcode::Compare, lhs.ir, {lhs.operand, Operand::floating(0)}, Operation::Ne);
    SlotId selector = cleanup_selector(lhs,cleanup_expression(b));
    auto common = live;
    emit(Opcode::Branch, IRType(), {lhs.operand, Operand::label(land ? rhs : short_path), Operand::label(land ? short_path : rhs)});
    start(rhs);
    Value value = second_conversion.kind == semantic::Conversion::Kind::User ? converted(b,second_conversion) : load(expression(b));
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
        value = c.kind == semantic::Conversion::Kind::User ? user_conversion(cond,c) : convert(binding(sem.facts[cond].entity), sem.facts[cond].type);
    }
    else value = incoming(ast[cond].first);
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
            else { discard(c); clean_inline(live, lifetime.entry); }
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
