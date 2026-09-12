#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::check_fixed_call(NodeId n, ScopeId s)
{
    auto callee = ast[n].first, designator = callee;
    while (ast[designator].kind == Kind::Parenthesized) designator = ast[designator].first;
    if (ast[designator].kind != Kind::IdExpression || ast[ast[designator].detail].kind != Kind::Name) return false;
    auto name = ast[designator].detail;
    auto binding = template_bindings[template_binding_index.get(ast.nodes.occurrences[name].source)];
    if (binding.dependent) return false;
    bool direct = !binding.entity || function_binding(binding.entity);
    bool adl = direct && callee == designator && ast[name].first == ast[name].last && ast[name].op != OP_COLON2;
    if (!binding.entity && !adl) return false;
    if (direct) {
        for (auto e : candidates(binding.entity)) {
            if (entities[e].template_pattern || (entities[e].member_info && !entities[e].is_static)) return false;
            auto owner = scopes[entities[e].owner].kind;
            if (owner == ScopeKind::Class || owner == ScopeKind::Block || owner == ScopeKind::Function) adl = false;
        }
    } else if (!template_fixed_expressions.get(ast.nodes.occurrences[designator].source)) return false;
    std::vector<NodeId> args;
    for (auto a = ast[ast[callee].next].first; a; a = ast[a].next) {
        if (!template_fixed_expressions.get(ast.nodes.occurrences[a].source)) return false;
        args.push_back(a);
    }
    ++unevaluated_depth;
    try {
    Expression fn;
    auto associated = adl ? associated_lookup(terminal(name),args) : 0;
    if (!binding.entity && !associated) { --unevaluated_depth; return false; } // Intrinsic call owner.
    if (associated) {
        fn.entity = explicit_template(name,merge_lookup(binding.entity,associated),s);
        fn.form = ExpressionForm::Overload; fn.category = ValueCategory::Lvalue; fn.ready = true;
        expressions[callee] = fn; facts[callee].entity = fn.entity; facts[callee].scope = s;
    } else fn = expression(callee,s);
    TypeId ft = 0; EntityId selected = 0; std::vector<Conversion> chosen;
    if (direct) {
        auto choice = select_call(fn.entity,expressions,&args,0,ValueCategory::Lvalue,
            object_uses[fn.object_use].naming_scope,0,chosen);
        if (choice.failure == CallFailure::NoViable) throw std::runtime_error("no viable fixed template call");
        if (choice.failure == CallFailure::Ambiguous) throw std::runtime_error("ambiguous fixed template call");
        selected = choice.entity; ft = entities[selected].type;
        if (deleted_transfer(selected)) throw std::runtime_error("deleted fixed template callee");
        check_access(selected,s,object_uses[fn.object_use].naming_scope);
        auto f = types[ft];
        for (unsigned i = args.size(); i < f.count; ++i) {
            auto a = entities[selected].specialization ? instantiate_default(selected,i) : default_arguments[entities[selected].defaults+i];
            args.push_back(a); chosen.push_back(conversion(a,types.parameters[f.offset+i]));
        }
        for (auto c = callee;; c = ast[c].first) {
            expressions[c].entity = selected; expressions[c].form = ExpressionForm::Ordinary;
            expressions[c].type = ft; facts[c].entity = selected; facts[c].type = call_type(selected);
            if (ast[c].kind != Kind::Parenthesized) break;
        }
    } else {
        ft = decay(fn.type); if (pointer(ft)) ft = types[ft].child;
        auto f = types[ft];
        if (f.kind != TypeKind::Function || args.size() < f.count || (!f.variadic && args.size() != f.count))
            throw std::runtime_error("invalid fixed indirect call");
        for (unsigned i = 0; i < args.size(); ++i)
            chosen.push_back(i < f.count ? conversion(args[i],types.parameters[f.offset+i]) : ellipsis_conversion(args[i]));
        require_conversion(callee,decay(fn.type));
    }
    auto f = types[ft];
    for (unsigned i = 0; i < f.count; ++i) reject_abstract(types.parameters[f.offset+i]);
    for (unsigned i = 0; i < chosen.size(); ++i)
        check_fixed_conversion(expressions[args[i]],args[i],chosen[i],s);
    Expression result; result.type = value_type(f.child);
    result.category = types[f.child].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[f.child].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    if (class_value(result.type) && result.category == ValueCategory::Prvalue) {
        complete_class(types[result.type].entity); reject_abstract(result.type);
        default_destructor(result.type,s,false);
    }
    // No argument application or result temporary belongs to the definition.
    store_call(result,args,chosen); result.ready = true;
    expressions[n] = result; facts[n].type = f.child; facts[n].scope = s; facts[n].entity = selected;
    ++template_fixed_call_work;
    } catch (...) { --unevaluated_depth; throw; }
    --unevaluated_depth; return true;
}
void Analyzer::reuse_fixed_call(NodeId n, NodeId source, ScopeId s, Expression& result)
{
    ++template_fixed_call_uses;
    auto context = ast.nodes.occurrences[n].context;
    result = expressions[source]; result.incoming = 0;
    auto selected = facts[source].entity;
    auto callee = ast[n].first;
    if (selected) {
        auto pattern = ast[source].first;
        for (auto c = callee;; c = ast[c].first, pattern = ast[pattern].first) {
            expressions[c] = expressions[pattern]; expressions[c].evaluated = !unevaluated_depth;
            facts[c].type = facts[pattern].type; facts[c].entity = selected; facts[c].scope = s;
            if (ast[c].kind != Kind::Parenthesized) break;
        }
        use_selected_function(selected,true);
    } else {
        expression(callee,s);
        auto incoming = expressions[ast[source].first].incoming;
        auto conversion = conversions[incoming]; apply_conversion(callee,conversion);
        expressions[callee].incoming = incoming;
    }
    std::vector<NodeId> args; std::vector<Conversion> chosen;
    bool materialize = false;
    for (unsigned i = 0; i < result.argument_count; ++i) {
        auto original = call_arguments[result.arguments+i];
        auto a = ast.projected(original,context); if (!a) a = original; // Declaration-owned default.
        expression(a,s); args.push_back(a);
        auto c = copy_conversion_recipe(conversions[result.conversions+i]);
        materialize |= c.kind == Conversion::Kind::Construction || c.kind == Conversion::Kind::User || c.kind == Conversion::Kind::ListPlan;
        chosen.push_back(c);
    }
    if (materialize) record_call(result,args,chosen);
    else {
        for (unsigned i = 0; i < args.size(); ++i) {
            apply_conversion(args[i],chosen[i]); expressions[args[i]].incoming = result.conversions+i;
        }
        result.arguments = call_arguments.size(); call_arguments.insert(call_arguments.end(),args.begin(),args.end());
    }
    facts[n].type = facts[source].type; facts[n].entity = selected;
}
} }
