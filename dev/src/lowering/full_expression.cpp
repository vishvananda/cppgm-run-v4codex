#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
bool Procedural::unwind_expression(NodeId n)
{
    if (!n) return false;
    if (unwind_expressions.empty()) unwind_expressions.resize(ast.nodes.size());
    if (unwind_expressions[n]) return unwind_expressions[n] == 2;
    ++full_expression_work;
    bool result = false;
    auto x = sem.expression_fact(n);
    EntityId callee = sem.facts[n].entity;
    bool call = (ast[n].kind == Kind::Call && x.form != semantic::ExpressionForm::Cast &&
        x.form != semantic::ExpressionForm::ListValue && x.form != semantic::ExpressionForm::PseudoDestructor) ||
        x.form == semantic::ExpressionForm::OperatorCall || (callee && sem.constructor_member(callee));
    if (call) result = !callee || ((sem.constructor_member(callee) ? sem.constructor_needed(callee) : true) && !sem.function_nonthrowing(callee));
    auto conversion = [&](const semantic::Conversion& c) {
        if (c.kind == semantic::Conversion::Kind::User) result |= !sem.function_nonthrowing(c.function);
        if (c.kind == semantic::Conversion::Kind::Construction && c.materialization && !sem.conversion_objects[c.materialization].elided)
            result |= !sem.trivial_transfer(c.function) && !sem.function_nonthrowing(c.function);
        if (c.kind == semantic::Conversion::Kind::List) {
            auto object = sem.list_objects[c.materialization];
            auto plan = sem.list_plans[object.plan];
            if (plan.constructor) result |= sem.constructor_needed(plan.constructor) && !sem.function_nonthrowing(plan.constructor);
        }
    };
    if (x.incoming) conversion(sem.conversion_fact(x.incoming));
    for (unsigned j = 0; j < x.count; ++j) conversion(sem.conversion_fact(x.conversions+j));
    for (NodeId child = ast[n].first; child; child = ast[child].next) result |= unwind_expression(child);
    unwind_expressions[n] = result ? 2 : 1; return result;
}
void Procedural::begin_full_expression(NodeId n, bool omit_result)
{
    if (full_expression.enabled) throw std::logic_error("nested full-expression owner");
    full_expression.enabled = cleanup_expression(n,omit_result);
    if (full_expression.enabled) guard_expression(n);
}
void Procedural::guard_expression(NodeId n)
{
    if (!full_expression.enabled || full_expression.open || emitting_cleanup) return;
    while (ast[n].kind == Kind::Parenthesized || ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenInitializer)
        n = ast[n].first;
    if (ast[n].kind == Kind::Conditional || (ast[n].kind == Kind::Binary && (ast[n].op == OP_LAND || ast[n].op == OP_LOR))) return;
    if (unwind_expression(n)) open_expression_region();
}
void Procedural::open_expression_region()
{
    if (full_expression.open || emitting_cleanup) return;
    ++full_expression_regions;
    BlockId cleanup;
    if (full_expression.lexical && live) {
        // A condition declaration already owns its complete lexical prefix;
        // there are no intermediate temporary suffixes needing a resume join.
        cleanup = block(); cleanup_blocks.push_back({live,BlockId(),cleanup});
    } else {
        if (!resume_terminal) resume_terminal = block();
        cleanup = cleanup_suffix(live,resume_terminal);
    }
    full_expression.open = true;
    emit(Opcode::EhTry,IRType(),{Operand::label(cleanup)});
}
void Procedural::close_expression_region()
{
    if (!full_expression.open) return;
    full_expression.open = false;
    emit(Opcode::EhEnd,IRType(),{});
    if ((resume_terminal && !resume_emitted) || cleanup_cursor < cleanup_blocks.size()) {
        auto continuation = block(); jump(continuation); flush_cleanups(); start(continuation);
    }
}
void Procedural::finish_full_expression(std::uint32_t stop)
{
    clean_inline(live,stop); close_expression_region(); full_expression = FullExpression();
}
} }
