#include "lowering/procedural.h"
#include "lowering/debug_location.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::expression(NodeId n, bool location)
{
    DebugScope debug(current_debug,debug_location(n));
    SourceInvocationScope invocation(source_invocation,sem.source_site(n));
    if (!n) throw std::logic_error("missing expression node");
    guard_expression(n);
    // Recursive call chains do not need the aggregate/vector/construction
    // dispatcher's large frame. Keep the same guards and invocation lifetime
    // around either path, and retain the original call and argument lowering.
    using Form = semantic::ExpressionForm;
    auto form = sem.expression_fact(n).form;
    if (form == Form::OperatorCall || form == Form::LiteralCall ||
        (ast.kind(n) == syntax::Kind::Call &&
         (form == Form::Ordinary || form == Form::Abort || form == Form::Unreachable ||
          form == Form::PseudoDestructor || form == Form::Expect || form == Form::InvokeMemberData)))
        return call(n);
    return expression_value(n,location);
}
} }
