#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::invoke_expression(NodeId n, ScopeId s)
{
    auto callee = ast[n].first;
    if (ast[callee].kind != Kind::IdExpression) return false;
    auto name = ast[callee].detail;
    return ast[name].kind == Kind::Name && ast[name].first == ast[name].last &&
        terminal(name) == invoke_builtin && !resolve(name,s);
}
Expression Analyzer::callable_expression(NodeId n, ScopeId s, NodeId callee,
    std::vector<NodeId> args, Expression fn)
{
    Expression result;
    if (fn.type && types[fn.type].kind == TypeKind::Named && entities[types[fn.type].entity].class_info) {
        std::vector<NodeId> operands(1, callee); operands.insert(operands.end(), args.begin(), args.end());
        if (operator_expression(n, s, OP_LPAREN, operands, result)) return result;
    }
    if (fn.form == ExpressionForm::PseudoDestructor) {
        if (!args.empty()) throw std::runtime_error("pseudo-destructor takes no arguments");
        result.type = types.fundamental(FT_VOID); result.form = ExpressionForm::PseudoDestructor; return result;
    }
    TypeId ft = 0;
    NodeId designator = callee;
    while (ast[designator].kind == Kind::Parenthesized) designator = ast[designator].first;
    bool direct_name = ast[designator].kind == Kind::IdExpression || ast[designator].kind == Kind::Member;
    TypeId object_type = 0;
    NodeId object_node = 0;
    ValueCategory object_category = ValueCategory::Lvalue;
    if (ast[designator].kind == Kind::Member) {
        object_node = ast[designator].first;
        object_type = expressions[object_node].type;
        if (ast[designator].op != OP_ARROW) object_category = expressions[object_node].category;
        if (ast[designator].op == OP_ARROW) {
            auto arrow = object_uses[fn.object_use].arrow;
            object_type = types[arrow ? arrow_chains[arrow].type : object_type].child;
        }
    } else {
        TypeId implicit = implicit_object_type(s);
        if (implicit) object_type = types[implicit].child;
    }
    if (fn.form == ExpressionForm::Overload && !direct_name) throw std::runtime_error("unresolved indirect callee");
    if (direct_name && (fn.form == ExpressionForm::Overload || (fn.entity && entities[fn.entity].kind == EntityKind::Function))) {
        std::vector<Conversion> chosen;
        auto choice = select_call(fn.entity,{},&args,object_type,object_category,
            object_uses[fn.object_use].naming_scope,0,chosen);
        if (choice.failure == CallFailure::NoViable) throw std::runtime_error("no viable function");
        if (choice.failure == CallFailure::Ambiguous) throw std::runtime_error("ambiguous overload");
        EntityId selected = choice.entity;
        if (object_type) chosen.erase(chosen.begin());
        facts.edit(n).entity = selected;
        if (entities[selected].member_info && !entities[selected].is_static)
        {
            NodeId direct = callee;
            while (ast[direct].kind == Kind::Parenthesized) direct = ast[direct].first;
            NodeId name = ast[direct].kind == Kind::Member ? ast[ast[ast[direct].first].next].detail : ast[direct].detail;
            record_member_receiver(result,object_node,object_type,selected,object_uses[fn.object_use].naming_scope,
                name && ast[name].first != ast[name].last,s);
        }
        ft = entities[selected].type;
        if (object_node && !result.object_use) record_object(result, object_node, 0, 0);
        if (result.object_use) object_uses[result.object_use].arrow = object_uses[fn.object_use].arrow;
        Type selected_type = types[ft];
        for (unsigned j = 0; j < selected_type.count; ++j) reject_abstract(types.parameters[selected_type.offset+j]);
        for (std::size_t i = args.size(); i < selected_type.count; ++i) {
            Conversion c; NodeId a = default_argument(selected,i,&c);
            args.push_back(a); chosen.push_back(c);
        }
        record_call(result, args, chosen);
        select_function(callee, selected, !result.object_use || !object_uses[result.object_use].virtual_slot);
    } else {
        ft = fn.type;
        if (pointer(ft)) ft = types[ft].child;
        if (types[ft].kind != TypeKind::Function) throw std::runtime_error("called object is not a function");
        Type f = types[ft];
        if (args.size() < f.count || (!f.variadic && args.size() != f.count)) throw std::runtime_error("indirect call arity");
        for (unsigned i = 0; i < f.count; ++i) reject_abstract(types.parameters[f.offset+i]);
        if (fn.form == ExpressionForm::BoundMember) {
            auto bound = object_uses[fn.object_use];
            record_object(result,bound.node,bound.type,bound.adjustment);
            object_uses[result.object_use].member_pointer = bound.member_pointer;
        } else require_conversion(callee, decay(fn.type));
        std::vector<Conversion> chosen;
        for (std::size_t i = 0; i < args.size(); ++i) {
            Conversion c;
            if (i < f.count) c = conversion(args[i], types.parameters[f.offset + i]);
            else c = ellipsis_conversion(args[i]);
            if (!c.valid()) throw std::runtime_error("indirect argument conversion");
            chosen.push_back(c);
        }
        record_call(result, args, chosen);
    }
    TypeId returned = types[ft].child;
    facts.edit(n).type = returned;
    result.type = value_type(returned);
    result.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    return result;
}
} }
