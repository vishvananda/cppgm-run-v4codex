#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
bool Procedural::unwind_expression(NodeId n, bool body_proof)
{
    if (!n) return false;
    auto& cache = body_proof ? proven_unwind_expressions : unwind_expressions;
    if (cache.empty()) cache.resize(ast.nodes.size());
    if (cache[n]) return cache[n] == 2;
    ++full_expression_work;
    bool result = ast[n].kind == Kind::Throw;
    if (ast[n].kind == Kind::SimpleDeclaration) result |= !sem.expression_nonthrowing(n);
    auto x = sem.expression_fact(n);
    EntityId callee = sem.facts[n].entity;
    bool call = (ast[n].kind == Kind::Call && x.form != semantic::ExpressionForm::Cast &&
        x.form != semantic::ExpressionForm::ListValue && x.form != semantic::ExpressionForm::PseudoDestructor) ||
        x.form == semantic::ExpressionForm::OperatorCall || (callee && sem.constructor_member(callee));
    if (call && x.form != semantic::ExpressionForm::Expect) result = !callee || ((sem.constructor_member(callee) ? sem.constructor_needed(callee) : true) && !sem.function_nonthrowing(callee));
    if (call && body_proof && callee && !sem.object_fact(n).virtual_slot &&
        !sem.object_fact(n).member_pointer && sem.scalar_body_nonthrowing(callee)) result = false;
    auto arrow = sem.arrow_chains[sem.object_fact(n).arrow];
    for (unsigned j = 0; j < arrow.count; ++j) result |= !sem.function_nonthrowing(sem.arrow_steps[arrow.first+j].function);
    auto arguments = [&](const semantic::Expression& call) {
        for (unsigned i = 0; i < call.argument_count; ++i) {
            auto a = sem.call_argument(call,i);
            if (a && a != n) result |= unwind_expression(a,body_proof);
        }
    };
    arguments(x);
    auto conversion = [&](const semantic::Conversion& c) {
        if (c.kind == semantic::Conversion::Kind::User) result |= !sem.function_nonthrowing(c.function);
        if (c.kind == semantic::Conversion::Kind::Construction && c.materialization && !sem.conversion_objects[c.materialization].elided)
            result |= !sem.trivial_transfer(c.function) && !sem.function_nonthrowing(c.function);
        if (c.kind == semantic::Conversion::Kind::List) {
            if (body_proof) result = true;
            auto object = sem.list_objects[c.materialization];
            auto plan = sem.list_plans[object.plan];
            if (plan.constructor) result |= sem.constructor_needed(plan.constructor) && !sem.function_nonthrowing(plan.constructor);
        }
        if (auto call = conversion_call(c)) arguments(*call);
    };
    const auto& discarded = sem.discarded_conversion(n);
    if (discarded.valid()) conversion(discarded);
    if (x.incoming) conversion(sem.conversion_fact(x.incoming));
    for (unsigned j = 0; j < x.count; ++j) conversion(sem.conversion_fact(x.conversions+j));
    if (ast[n].kind == Kind::Lambda)
        for (auto i = sem.closure(sem.types[x.type].entity).first_capture; i; i = sem.closure_captures[i].next)
            if (sem.closure_captures[i].conversion) conversion(sem.conversion_fact(sem.closure_captures[i].conversion));
    if (x.form == semantic::ExpressionForm::Typeid && sem.rtti_expression(n).dynamic)
        result |= unwind_expression(ast[n].first,body_proof);
    if (ast[n].kind != Kind::Lambda && ast[n].kind != Kind::Sizeof && ast[n].kind != Kind::TypeTrait)
        for (NodeId child = ast[n].first; child; child = ast[child].next) result |= unwind_expression(child,body_proof);
    // These operations own additional allocation/initialization recipes. The
    // O0 scalar proof does not inspect or demand those recipes.
    if (body_proof && (ast[n].kind == Kind::New || ast[n].kind == Kind::Delete ||
        sem.class_initialization(n,sem.facts[n].type).source)) result = true;
    cache[n] = result ? 2 : 1; return result;
}
void Procedural::begin_full_expression(NodeId n, bool omit_result)
{
    if (full_expression.enabled) throw std::logic_error("nested full-expression owner");
    full_expression.enabled = cleanup_expression(n,omit_result) || (unwind_live() && unwind_expression(n));
    if (full_expression.enabled) full_expression.proven_nonthrowing = !unwind_expression(n,true) && !cleanup_expression(n,omit_result);
    if (full_expression.enabled && !omit_result) {
        auto result = n;
        while (ast[result].kind == Kind::Parenthesized || ast[result].kind == Kind::Initializer || ast[result].kind == Kind::ParenInitializer)
            result = ast[result].first;
        auto incoming = sem.expression_fact(result).incoming;
        if (!incoming || sem.conversion_fact(incoming).kind == semantic::Conversion::Kind::Discarded)
            full_expression.result_temporary = sem.object_fact(result).temporary;
    }
    auto root = n;
    while (root && (ast[root].kind == Kind::Parenthesized || ast[root].kind == Kind::Initializer || ast[root].kind == Kind::ParenInitializer)) root = ast[root].first;
    full_expression.root = full_expression.terminal_value = root;
    // A selected constructor/conversion can observe argument temporaries after
    // the logical result is computed. That result is not the terminal consumer.
    auto incoming = sem.conversion_fact(sem.expression_fact(root).incoming);
    if (incoming.kind == semantic::Conversion::Kind::Construction || incoming.kind == semantic::Conversion::Kind::User ||
        incoming.kind == semantic::Conversion::Kind::List || sem.class_initialization(n,sem.facts[n].type).source)
        full_expression.terminal_value = 0;
    if (full_expression.enabled) guard_expression(n);
}
void Procedural::guard_expression(NodeId n, bool storage_ready)
{
    if (!full_expression.enabled || full_expression.open || emitting_cleanup || full_expression.suppress_guard) return;
    if (ast[n].kind == Kind::Lambda && !unwind_expression(n) && !cleanup_expression(n)) return;
    while (ast[n].kind == Kind::Parenthesized || ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenInitializer)
        n = ast[n].first;
    // Guarded initialization establishes temporary storage on its run edge.
    // Storage cannot throw; argument construction/calls establish the region.
    if (full_expression.storage_boundary && (!storage_ready || n == full_expression.root)) return;
    if (ast[n].kind == Kind::Conditional || (ast[n].kind == Kind::Binary && (ast[n].op == OP_LAND || ast[n].op == OP_LOR))) return;
    if (full_expression.terminal_branch && !storage_ready &&
        (ast[n].kind == Kind::Member || sem.expression_fact(n).form == semantic::ExpressionForm::Construction)) return;
    // Without a live prefix or published handler, scalar consumption waits
    // for the first resource-owning expression to establish the region.
    if (full_expression.scalar_terminal && !live && !resume_terminal &&
        !sem.temporary_cleanup(sem.object_fact(n).temporary)) return;
    // O0 retains the region for an observable intermediate cleanup, even
    // when its calls are nonthrowing. Empty destructors retain their existing
    // call presentation without introducing a new empty unwind boundary.
    if ((unwind_expression(n) && (unwind_live() || n == full_expression.root)) ||
        cleanup_expression(n,false,!live)) open_expression_region();
}
void Procedural::open_expression_region()
{
    if (full_expression.open || emitting_cleanup) return;
    full_expression.storage_boundary = false;
    ++full_expression_regions;
    BlockId cleanup;
    if (full_expression.proven_nonthrowing && !exception_context) {
        // Retain the O0 protected-region view, but no destruction actions are
        // reachable on an edge whose complete expression cannot unwind.
        if (!resume_terminal) resume_terminal = block();
        cleanup = resume_terminal;
    } else if (full_expression.lexical && live) {
        // A condition declaration already owns its complete lexical prefix;
        // there are no intermediate temporary suffixes needing a resume join.
        cleanup = block(); cleanup_blocks.push_back({live,BlockId(),cleanup});
    } else {
        cleanup = unwind_target();
    }
    full_expression.open = true;
    emit(Opcode::EhTry,IRType(),{Operand::label(cleanup)});
}
void Procedural::close_expression_region()
{
    if (!full_expression.open) return;
    full_expression.open = false;
    emit(Opcode::EhEnd,IRType(),{});
    if ((resume_terminal && !resume_emitted) || cleanup_cursor < cleanup_blocks.size() || unwind_cursor < unwind_continuations.size()) {
        auto continuation = block(); jump(continuation); flush_cleanups(); start(continuation);
    }
}
void Procedural::finish_full_expression(std::uint32_t stop)
{
    if (ended) { full_expression = FullExpression(); return; }
    clean_inline(live,stop); close_expression_region(); full_expression = FullExpression();
}
} }
