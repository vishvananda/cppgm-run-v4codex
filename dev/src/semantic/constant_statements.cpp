#include "semantic/analyzer.h"

namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::constant_step()
{
    if (!constant_remaining) { constant_limited = true; return false; }
    --constant_remaining; ++constant_steps; return true;
}
bool Analyzer::constant_local(EntityId e, ScopeId s)
{
    if (!e || !constant_frame || entities[e].is_static || entities[e].external_decl ||
        (!integral(entities[e].type) && !floating_type(entities[e].type)) || types[entities[e].type].kind == TypeKind::LRef ||
        types[entities[e].type].kind == TypeKind::RRef || (types[entities[e].type].cv & 2)) return false;
    auto init = entities[e].initializer;
    if (!init) return false;
    auto slot = constant_frame->bindings.get(e);
    if (!slot) {
        slot = constant_frame->values.size(); constant_frame->values.push_back(Constant());
        constant_frame->bindings.put(e,slot);
    }
    // A loop declaration begins a new lifetime; its initializer cannot read
    // the value left by the previous iteration (including self-initialization).
    constant_frame->values[slot] = Constant();
    auto n = init;
    while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::BracedInit) {
        if (!ast[n].first) { constant_frame->values[slot] = convert(Constant(types.fundamental(FT_INT),0),entities[e].type); return true; }
        n = ast[n].first;
    }
    auto c = conversions[expressions[n].incoming];
    auto value = c.target ? constant_node_conversion(n,c,s) : convert(evaluate(n,s),entities[e].type);
    constant_frame->values[slot] = value;
    return value.valid;
}
Constant Analyzer::execute_constant_condition(NodeId n, ScopeId s)
{
    auto first = ast[n].first;
    if (!first) return Constant(types.fundamental(FT_BOOL),1);
    if (ast[first].kind == Kind::ConditionDeclaration) {
        auto e = facts[n].entity;
        if (!constant_local(e,s)) return Constant();
        return convert(constant_frame->values[constant_frame->bindings.get(e)],facts[n].type,true);
    }
    return constant_node_conversion(first,conversions[expressions[n].conversions],s);
}
Analyzer::ConstantStatement Analyzer::execute_constant_statement(NodeId n, ScopeId s)
{
    const ConstantStatement next{ConstantFlow::Next,Constant()}, failure{ConstantFlow::Failure,Constant()};
    if (!n) return next;
    if (!constant_step()) return failure;
    if (facts[n].scope) s = facts[n].scope;
    auto first = ast[n].first;
    switch (ast[n].kind) {
    case Kind::Compound: case Kind::Then: case Kind::Else:
        for (auto c = first; c; c = ast[c].next) {
            auto result = execute_constant_statement(c,s);
            if (result.flow != ConstantFlow::Next) return result;
        }
        return next;
    case Kind::SimpleDeclaration: {
        if (spec_has(first,KW_TYPEDEF)) return next;
        auto list = ast[first].next;
        for (auto c = ast[list].first; c; c = ast[c].next)
            if (!constant_local(facts[ast[c].first].entity,s)) return failure;
        return next;
    }
    case Kind::Return: {
        auto c = conversions[expressions[first].incoming];
        auto value = c.target ? constant_node_conversion(first,c,s) :
            ast[first].kind == Kind::BracedInit && !ast[first].first ? evaluate(first,s) : Constant();
        return {value.valid ? ConstantFlow::Return : ConstantFlow::Failure,value};
    }
    case Kind::If: {
        auto condition = execute_constant_condition(first,s);
        if (!condition.valid) return failure;
        auto yes = ast[first].next;
        return execute_constant_statement(condition.bits ? yes : ast[yes].next,s);
    }
    case Kind::For: case Kind::While: case Kind::Do: {
        auto condition = first, body = ast[first].next;
        NodeId iteration = 0;
        if (ast[n].kind == Kind::For) {
            auto init = execute_constant_statement(first,s);
            if (init.flow != ConstantFlow::Next) return init;
            condition = ast[first].next; iteration = ast[condition].next; body = ast[iteration].next;
        } else if (ast[n].kind == Kind::Do) { body = first; condition = ast[body].next; }
        bool initial = ast[n].kind == Kind::Do;
        for (;;) {
            if (!constant_step()) return failure;
            if (!initial) {
                auto test = execute_constant_condition(condition,s);
                if (!test.valid) return failure;
                if (!test.bits) return next;
            }
            initial = false;
            auto result = execute_constant_statement(body,s);
            if (result.flow == ConstantFlow::Break) return next;
            if (result.flow == ConstantFlow::Return || result.flow == ConstantFlow::Failure) return result;
            result = execute_constant_statement(iteration,s);
            if (result.flow != ConstantFlow::Next) return result;
        }
    }
    case Kind::ExpressionStatement: case Kind::ForInit: case Kind::Iteration:
        for (auto c = first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::SimpleDeclaration) {
                auto result = execute_constant_statement(c,s);
                if (result.flow != ConstantFlow::Next) return result;
            } else if (!evaluate(c,s).valid) return failure;
        }
        return next;
    case Kind::Break: return {ConstantFlow::Break,Constant()};
    case Kind::Continue: return {ConstantFlow::Continue,Constant()};
    case Kind::Alias: case Kind::UsingDeclaration: case Kind::UsingDirective:
    case Kind::StaticAssert: case Kind::EmptyDeclaration: case Kind::Class:
    case Kind::ClassForward: case Kind::Enum: return next;
    default: return failure;
    }
}
Constant Analyzer::constant_mutation(NodeId n, ScopeId s)
{
    if (!active_constant || !constant_frame) return Constant();
    auto target = ast[n].first;
    auto source = ast[target].next;
    while (ast[target].kind == Kind::Parenthesized) target = ast[target].first;
    if (ast[target].kind != Kind::IdExpression) return Constant();
    auto e = expressions[target].entity;
    auto slot = constant_frame->bindings.get(e);
    if (!slot || (types[entities[e].type].cv & 3)) return Constant();
    auto op = ast[n].op;
    auto old = constant_frame->values[slot];
    auto x = expressions[n];
    Constant value;
    if (op == OP_ASS) value = constant_node_conversion(source,conversions[x.conversions+1],s);
    else {
        ETokenType binary_op = TOK_INVALID;
        switch (op) {
        case OP_INC: case OP_PLUSASS: binary_op = OP_PLUS; break;
        case OP_DEC: case OP_MINUSASS: binary_op = OP_MINUS; break;
        case OP_STARASS: binary_op = OP_STAR; break;
        case OP_DIVASS: binary_op = OP_DIV; break;
        case OP_MODASS: binary_op = OP_MOD; break;
        case OP_BANDASS: binary_op = OP_AMP; break;
        case OP_BORASS: binary_op = OP_BOR; break;
        case OP_XORASS: binary_op = OP_XOR; break;
        case OP_LSHIFTASS: binary_op = OP_LSHIFT; break;
        case OP_RSHIFTASS: binary_op = OP_RSHIFT; break;
        default: return Constant();
        }
        auto left = convert(old,conversions[x.conversions].target,true);
        auto right = source ? constant_node_conversion(source,conversions[x.conversions+1],s) :
            convert(Constant(types.fundamental(FT_INT),1),left.type,true);
        value = convert(binary(binary_op,left,right,true),entities[e].type,true);
    }
    if (!value.valid) return value;
    constant_frame->values[slot] = value;
    return ast[n].kind == Kind::Postfix ? old : value;
}
} }
