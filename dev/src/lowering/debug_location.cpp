#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
lowir_model::DebugLocation Procedural::debug_location(NodeId node)
{
    lowir_model::DebugLocation result;
    if (!linkage.debug || !node) return result;
    // An arithmetic expression starts at its left operand; the operator token
    // retained by the parser is not the beginning of the source expression.
    while (ast[node].kind == syntax::Kind::Binary && ast[node].first) node = ast[node].first;
    const auto& location = static_cast<const syntax::Ast&>(ast).locations[ast[node].location];
    if (!location.presumed_file || !location.line) return result;
    result.file = p.intern(spelling(location.presumed_file));
    result.line = location.line; result.column = location.column ? location.column : 1;
    // A named counter initialized and updated on the same line shares its
    // value's originating column in line-table mode. Across lines keep the
    // actual update location; no line is moved to a declaration elsewhere.
    if ((ast[node].kind == syntax::Kind::Unary || ast[node].kind == syntax::Kind::Postfix) &&
        (ast[node].op == OP_INC || ast[node].op == OP_DEC)) {
        auto operand = ast[node].first;
        auto entity = sem.expression_fact(operand).entity;
        if (entity && ast[operand].kind == syntax::Kind::IdExpression) {
            auto init = sem.entities[entity].initializer;
            while (init && ast[init].kind == syntax::Kind::Initializer) init = ast[init].first;
            if (init && init != node) {
                const auto& origin = static_cast<const syntax::Ast&>(ast).locations[ast[init].location];
                if (origin.presumed_file == location.presumed_file && origin.line == location.line && origin.column)
                    result.column = origin.column;
            }
        }
    }
    return result;
}
} }
