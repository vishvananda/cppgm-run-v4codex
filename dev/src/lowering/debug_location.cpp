#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
lowir_model::DebugLocation Procedural::debug_location(NodeId node)
{
    lowir_model::DebugLocation result;
    if (!linkage.debug || !node) return result;
    // An arithmetic expression starts at its left operand; the operator token
    // retained by the parser is not the beginning of the source expression.
    while (ast.kind(node) == syntax::Kind::Binary && ast.first(node)) node = ast.first(node);
    const auto& location = static_cast<const syntax::Ast&>(ast).locations[ast.location(node)];
    if (!location.presumed_file || !location.line) return result;
    result.file = p.intern(spelling(location.presumed_file));
    result.line = location.line; result.column = location.column ? location.column : 1;
    // A named counter initialized and updated on the same line shares its
    // value's originating column in line-table mode. Across lines keep the
    // actual update location; no line is moved to a declaration elsewhere.
    if ((ast.kind(node) == syntax::Kind::Unary || ast.kind(node) == syntax::Kind::Postfix) &&
        (ast.op(node) == OP_INC || ast.op(node) == OP_DEC)) {
        auto operand = ast.first(node);
        auto entity = sem.expression_fact(operand).entity;
        if (entity && ast.kind(operand) == syntax::Kind::IdExpression) {
            auto init = sem.entities[entity].initializer;
            while (init && ast.kind(init) == syntax::Kind::Initializer) init = ast.first(init);
            if (init && init != node) {
                const auto& origin = static_cast<const syntax::Ast&>(ast).locations[ast.location(init)];
                if (origin.presumed_file == location.presumed_file && origin.line == location.line && origin.column)
                    result.column = origin.column;
            }
        }
    }
    return result;
}
} }
