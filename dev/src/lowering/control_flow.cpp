#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
void Procedural::condition(NodeId n, BlockId yes, BlockId no)
{
    if (ast[n].kind == Kind::Condition) {
        if (!ast[n].first) { jump(yes); return; }
        if (sem.facts[n].entity) {
            object(sem.facts[n].entity);
            Value v = load(binding(sem.facts[n].entity));
            if (v.ir.floating()) v = emit(Opcode::Compare, v.ir, {v.operand, Operand::floating(0)}, Operation::Ne);
            emit(Opcode::Branch, IRType(), {v.operand, Operand::label(yes), Operand::label(no)}); return;
        }
        n = ast[n].first;
    }
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    if (ast[n].kind == Kind::Binary && (ast[n].op == OP_LAND || ast[n].op == OP_LOR)) {
        BlockId rhs = block(); bool land = ast[n].op == OP_LAND;
        condition(ast[n].first, land ? rhs : yes, land ? no : rhs);
        start(rhs); condition(ast[ast[n].first].next, yes, no); return;
    }
    Value v = load(expression(n));
    if (v.ir.floating()) v = emit(Opcode::Compare, v.ir, {v.operand, Operand::floating(0)}, Operation::Ne);
    emit(Opcode::Branch, IRType(), {v.operand, Operand::label(yes), Operand::label(no)});
}
Value Procedural::conditional(NodeId n, bool location)
{
    auto fact = sem.expression_fact(n);
    TypeId target = fact.type;
    // An array/function glvalue necessarily transports its address.
    location |= sem.types[target].kind == TypeKind::Array || sem.types[target].kind == TypeKind::Function;
    IRType ir = location ? IRType(IRType::Ptr) : type(target);
    bool has_result = ir != IRType::Void;
    SlotId slot = has_result ? builder->add_slot(0, ir) : SlotId();
    BlockId yes = block(), no = block(), end = block();
    NodeId a = ast[n].first, b = ast[a].next, c = ast[b].next;
    Value test = load(expression(a));
    if (test.ir.floating()) test = emit(Opcode::Compare, test.ir, {test.operand, Operand::floating(0)}, Operation::Ne);
    emit(Opcode::Branch, IRType(), {test.operand, Operand::label(yes), Operand::label(no)});
    start(yes);
    Value y = location ? address(expression(b, true)) : convert(expression(b), target);
    if (has_result) emit(Opcode::Store, ir, {y.operand, Operand::slot(slot)});
    jump(end);
    start(no);
    Value z = location ? address(expression(c, true)) : convert(expression(c), target);
    if (has_result) emit(Opcode::Store, ir, {z.operand, Operand::slot(slot)});
    jump(end); start(end);
    Value v = has_result ? emit(Opcode::Load, ir, {Operand::slot(slot)}) : Value();
    v.type = target; v.address = location; return v;
}
Value Procedural::logical(NodeId n)
{
    bool land = ast[n].op == OP_LAND;
    NodeId a = ast[n].first, b = ast[a].next;
    // Value context materializes a canonical truth slot. Condition context is
    // handled independently by condition(), without introducing that storage.
    Value lhs = load(expression(a));
    if (lhs.operand.kind == Operand::Integer && (land ? !lhs.operand.data.integer : bool(lhs.operand.data.integer)))
        return Value(Operand::integer(!land), IRType::I64, sem.expression_fact(n).type);
    SlotId slot = builder->add_slot(0, IRType::I64);
    BlockId rhs = block(), short_path = block(), end = block();
    if (lhs.ir.floating()) lhs = emit(Opcode::Compare, lhs.ir, {lhs.operand, Operand::floating(0)}, Operation::Ne);
    emit(Opcode::Branch, IRType(), {lhs.operand, Operand::label(land ? rhs : short_path), Operand::label(land ? short_path : rhs)});
    start(rhs);
    Value value = load(expression(b));
    IRType comparison = value.ir.floating() ? value.ir : IRType(IRType::I64);
    value = emit(Opcode::Compare, comparison, {value.operand, value.ir.floating() ? Operand::floating(0) : Operand::integer(0)}, Operation::Ne);
    emit(Opcode::Store, IRType::I64, {value.operand, Operand::slot(slot)}); jump(end);
    start(short_path);
    emit(Opcode::Store, IRType::I64, {Operand::integer(!land), Operand::slot(slot)}); jump(end);
    start(end);
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
void Procedural::switch_statement(NodeId n)
{
    NodeId cond = child(n, Kind::Condition), body = ast[cond].next;
    Value value;
    if (sem.facts[cond].entity) { object(sem.facts[cond].entity); value = convert(binding(sem.facts[cond].entity), sem.facts[cond].type); }
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
    if (k == Kind::Compound || k == Kind::Then || k == Kind::Else) {
        for (NodeId c = ast[n].first; c; c = ast[c].next) statement(c);
        return;
    }
    if (k == Kind::Label) {
        if (!labels[n]) labels[n] = block();
        jump(labels[n]); start(labels[n]); statement(ast[n].first); return;
    }
    if (k == Kind::Case || k == Kind::Default) {
        jump(labels[n]); start(labels[n]);
        statement(k == Kind::Case ? ast[ast[n].first].next : ast[n].first); return;
    }
    if (ended) return;
    switch (k) {
    case Kind::SimpleDeclaration: {
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
            else expression(c);
        }
        return;
    case Kind::Return:
        if (type(returned) == IRType::Void) {
            if (ast[n].first) expression(ast[n].first);
            emit(Opcode::Return, IRType(), {});
        } else {
            Value value = ast[n].first ? convert(expression(ast[n].first, reference(returned)), returned) : Value(Operand::integer(0), type(returned));
            emit(Opcode::Return, type(returned), {value.operand});
        }
        return;
    case Kind::Goto: {
        NodeId target = sem.facts[n].target;
        if (!labels[target]) labels[target] = block();
        jump(labels[target]); return;
    }
    case Kind::Break: jump(break_target); return;
    case Kind::Continue: jump(continue_target); return;
    case Kind::If: {
        BlockId yes = block(), no = block(), end = block();
        condition(child(n, Kind::Condition), yes, no);
        start(yes); statement(child(n, Kind::Then)); bool yes_ends = ended; jump(end);
        start(no); statement(child(n, Kind::Else)); bool no_ends = ended; jump(end);
        if (!(yes_ends && no_ends)) start(end);
        return;
    }
    case Kind::While: case Kind::For: case Kind::Do: {
        BlockId cond = block(), body = block(), step = k == Kind::For ? block() : cond, end = block();
        BlockId old_break = break_target, old_continue = continue_target;
        break_target = end; continue_target = step;
        if (k == Kind::For) statement(child(n, Kind::ForInit));
        if (k != Kind::Do) {
            jump(cond); start(cond); condition(child(n, Kind::Condition), body, end);
        } else jump(body);
        start(body); statement(k == Kind::Do ? ast[n].first : ast[n].last); jump(step);
        if (k == Kind::For) { start(step); statement(child(n, Kind::Iteration)); jump(cond); }
        if (k == Kind::Do) { start(cond); condition(child(n, Kind::Condition), body, end); }
        start(end); break_target = old_break; continue_target = old_continue; return;
    }
    case Kind::Switch: switch_statement(n); return;
    default: return;
    }
}
} }
