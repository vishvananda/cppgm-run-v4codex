#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::condition_target(Expression value, bool is_switch)
{
    auto t = value.type;
    if (!t) throw std::runtime_error("unresolved condition");
    if (class_value(value_type(t))) {
        if (!is_switch) return types.fundamental(FT_BOOL);
        TypeId converted = 0;
        for (EntityId e : conversion_candidates(value_type(t))) {
            TypeId result = value_type(types[entities[e].type].child);
            if (members[entities[e].member_info].explicit_constructor || !integral(result)) continue;
            if (!object_conversion(e,value_type(t),value.category).valid()) continue;
            result = types.unqualified(result);
            if (converted && result != converted) throw std::runtime_error("ambiguous contextual integral conversion");
            converted = result;
        }
        if (!converted) throw std::runtime_error("missing contextual integral conversion");
        return promote(converted);
    }
    if (is_switch) {
        if (!integral(t)) throw std::runtime_error("switch requires integral or enum condition");
    } else if (scoped_enum(t) || (!arithmetic(t) && !pointer(decay(t)) && types[decay(t)].kind != TypeKind::MemberPointer && !fundamental(t, FT_NULLPTR_T)))
        throw std::runtime_error("invalid boolean condition");
    return is_switch ? promote(t) : types.fundamental(FT_BOOL);
}
void Analyzer::resolve_condition(NodeId n, ScopeId s, bool is_switch)
{
    facts.edit(n).scope = s;
    NodeId c = ast[n].first;
    if (!c) return;
    TypeId t;
    if (ast[c].kind == Kind::ConditionDeclaration) {
        NodeId specs = ast[c].first, d = ast[specs].next;
        TypeId deduction = 0;
        t = spec_has(specs,KW_AUTO) ? deduced_object_type(specs,d,ast[d].next,s,deduction) :
            declarator(d, specifiers(specs, s), s);
        facts.edit(n).entity = declare_object(d, ast[d].next, t, specs, s, c);
    } else t = expression(c, s).type;
    Expression value; value.type = t;
    value.category = facts[n].entity ? ValueCategory::Lvalue : expressions[c].category;
    auto occurrence = ast.nodes.occurrences[n];
    auto retained = occurrence.context ? template_statement_conversions.get(occurrence.source) : 0;
    auto target = retained ? conversions[retained].target : condition_target(value,is_switch);
    if (retained) ++statement_conversion_uses;
    if (class_value(value_type(t))) {
        Conversion conversion = retained ? copy_conversion_recipe(conversions[retained]) :
            conversion_function(facts[n].entity ? n : c,target,!is_switch,ReferenceBinding::Any,facts[n].entity);
        if (retained && facts[n].entity && conversion.kind == Conversion::Kind::User)
            user_conversions[conversion.materialization].object_entity = facts[n].entity;
        Expression result; result.type = target; result.ready = true;
        record_conversion(result,facts[n].entity ? n : c,conversion);
        facts.edit(n).type = target; expressions.set(n,result);
        if (is_switch) switches.back().type = target;
        return;
    }
    facts.edit(n).type = target;
    if (is_switch) switches.back().type = facts[n].type;
    Expression result; result.type = facts[n].type; result.ready = true;
    if (ast[c].kind != Kind::ConditionDeclaration)
        record_conversion(result, c, retained ? copy_conversion_recipe(conversions[retained]) :
            is_switch ? conversion(c, result.type) : boolean_conversion(c));
    else {
        NodeId d = ast[ast[c].first].next;
        facts.edit(n).entity = facts[d].entity;
        Conversion conversion; conversion.target = result.type;
        conversion.rank = types.unqualified(t) == result.type ? 0 : 2;
        conversion.kind = Conversion::Kind::Contextual;
        record_conversion(result, 0, conversion);
    }
    expressions.set(n,result);
}
void Analyzer::resolve_statement(NodeId n, ScopeId s)
{
    if (!n) return;
    if (definitions) resolve_angle_statement(n,s);
    switch (ast[n].kind) {
    case Kind::Assembly: resolve_assembly(n,s); return;
    case Kind::Throw: expression(n,s); return;
    case Kind::Try: case Kind::FunctionTry: {
        auto body = child(n,Kind::Compound);
        auto scope = make_scope(ScopeKind::Block,s);
        facts.edit(body).scope = scope; facts.edit(n).scope = scope; try_scopes.put(scope,1);
        for (auto c = ast[body].first; c; c = ast[c].next) resolve_statement(c,scope);
        for (auto h = ast[body].next; h; h = ast[h].next) resolve_handler(h,s,false,ast[n].kind == Kind::FunctionTry);
        return;
    }
    case Kind::Handler: resolve_handler(n,s); return;
    case Kind::RangeFor: resolve_range(n,s); return;
    case Kind::Compound: {
        ScopeId bs = make_scope(ScopeKind::Block, s);
        facts.edit(n).scope = bs;
        for (NodeId c = ast[n].first; c; c = ast[c].next) resolve_statement(c, bs);
        return;
    }
    case Kind::If: case Kind::Switch: case Kind::While: case Kind::Do: case Kind::For: {
        bool loop = ast[n].kind == Kind::While || ast[n].kind == Kind::Do || ast[n].kind == Kind::For;
        bool sw = ast[n].kind == Kind::Switch;
        ScopeId control = make_scope(ScopeKind::Control, s);
        facts.edit(n).scope = control;
        if (ast[n].kind == Kind::If && (ast[n].flags & 1)) {
            auto cond = child(n,Kind::Condition); resolve_condition(cond,control,false);
            if (facts[cond].entity) throw std::runtime_error("constexpr if requires an expression condition");
            auto value = execute_constant_condition(cond,control);
            if (!value.valid) throw std::runtime_error("constexpr if condition is not constant");
            facts.edit(n).value = constants.size(); constants.push_back(value);
            auto selected = constant_truth(value) ? Kind::Then : Kind::Else;
            for (auto c = ast[cond].next; c; c = ast[c].next)
                if (!ast.nodes.occurrences[n].context || ast[c].kind == selected) resolve_statement(c,control);
            return;
        }
        if (loop) ++loop_depth;
        if (sw) { ++switch_depth; switches.emplace_back(); }
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            bool header = ast[c].kind == Kind::Condition || ast[c].kind == Kind::ForInit || ast[c].kind == Kind::Iteration;
            if (header && loop) --loop_depth;
            if (header && sw) --switch_depth;
            if (ast[c].kind == Kind::Condition) resolve_condition(c, control, sw);
            else if (ast[c].kind == Kind::ForInit || ast[c].kind == Kind::Iteration || ast[c].kind == Kind::Then || ast[c].kind == Kind::Else)
                resolve_statement(c, control);
            else resolve_statement(c, ast[c].kind == Kind::Compound ? control : make_scope(ScopeKind::Block, control));
            if (header && loop) ++loop_depth;
            if (header && sw) ++switch_depth;
        }
        if (sw) { --switch_depth; switches.pop_back(); }
        if (loop) --loop_depth;
        return;
    }
    case Kind::Then: case Kind::Else: {
        NodeId c = ast[n].first;
        resolve_statement(c, ast[c].kind == Kind::Compound ? s : make_scope(ScopeKind::Block, s)); return;
    }
    case Kind::SimpleDeclaration: case Kind::Alias: case Kind::UsingDirective: case Kind::UsingDeclaration:
    case Kind::NamespaceAlias: case Kind::StaticAssert: case Kind::Class: case Kind::ClassForward: case Kind::Enum:
        declaration(n, s); return;
    case Kind::Return:
        for (auto scope=s; scope && scopes[scope].kind != ScopeKind::Function; scope=scopes[scope].parent)
            if (constructor_handler_scopes.get(scope)) throw std::runtime_error("return in constructor function-try handler");
        if (placeholder_returns.get(current_function)) deduce_return(n,s);
        if (class_value(return_type)) { record_class_return(n,s); return; }
        if (ast[n].first) {
            NodeId value_node = ast[n].first;
            if (ast[value_node].kind == Kind::BracedInit && !ast[value_node].first) {
                initialize(value_node, return_type, s); return;
            }
            Expression value = expression(ast[n].first, s);
            if (fundamental(return_type, FT_VOID) && !fundamental(value.type, FT_VOID)) throw std::runtime_error("value returned from void");
            require_conversion(ast[n].first, return_type);
        } else if (!fundamental(return_type, FT_VOID)) throw std::runtime_error("missing return value");
        return;
    case Kind::Label:
        facts.edit(n).scope = s; resolve_statement(ast[n].first, s); return;
    case Kind::Goto: facts.edit(n).scope = s; return;
    case Kind::Break:
        if (!loop_depth && !switch_depth) throw std::runtime_error("break outside loop/switch");
        return;
    case Kind::Continue:
        if (!loop_depth) throw std::runtime_error("continue outside loop");
        return;
    case Kind::Case: {
        if (!switch_depth) throw std::runtime_error("case outside switch");
        NodeId c = ast[n].first;
        Expression x = expression(c, s);
        if (!integral(x.type) || !evaluate(c, s).valid) throw std::runtime_error("nonconstant case label");
        require_conversion(c, switches.back().type);
        Constant value = convert(evaluate(c, s), switches.back().type);
        if (switches.back().labels.get(value.bits)) throw std::runtime_error("duplicate case label");
        switches.back().labels.put(value.bits, n);
        resolve_statement(ast[c].next, s); return;
    }
    case Kind::Default:
        if (!switch_depth) throw std::runtime_error("default outside switch");
        if (switches.back().has_default) throw std::runtime_error("duplicate default label");
        switches.back().has_default = true;
        resolve_statement(ast[n].first, s); return;
    case Kind::ExpressionStatement: case Kind::ForInit: case Kind::Iteration:
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::SimpleDeclaration) resolve_statement(c, s);
            else {
                if (defer_reserved_statement(c,s)) continue;
                if (expression(c, s).form == ExpressionForm::Overload) throw std::runtime_error("unresolved discarded overload");
                prepare_discarded(c);
            }
        }
        return;
    default: throw std::runtime_error("unsupported statement");
    }
}
} }
