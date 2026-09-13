#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::check_fixed_call(NodeId n, ScopeId s)
{
    auto callee = ast[n].first, designator = callee;
    while (ast[designator].kind == Kind::Parenthesized) designator = ast[designator].first;
    bool member = ast[designator].kind == Kind::Member;
    if (!member && ast[designator].kind != Kind::IdExpression) return false;
    if (member && !template_fixed_expressions.get(ast.nodes.occurrences[designator].source)) return false;
    auto name = member ? ast[ast[ast[designator].first].next].detail : ast[designator].detail;
    if (ast[name].kind != Kind::Name) return false;
    auto binding = member ? TemplateBinding() : template_bindings[template_binding_index.get(ast.nodes.occurrences[name].source)];
    if (member) binding.entity = expressions[designator].entity;
    if (binding.dependent) return false;
    bool direct = !binding.entity || function_binding(binding.entity);
    if (!direct && class_value(expressions[designator].type)) return false; // Callable-object/operator owner.
    bool adl = direct && !member && callee == designator && ast[name].first == ast[name].last && ast[name].op != OP_COLON2;
    if (!binding.entity && !adl) return false;
    if (direct) {
        for (auto e : candidates(binding.entity)) {
            if (entities[e].template_pattern || (!member && entities[e].member_info && !entities[e].is_static)) return false;
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
    Expression fn, result;
    auto associated = adl ? associated_lookup(terminal(name),args) : 0;
    if (!binding.entity && !associated) { --unevaluated_depth; return false; } // Intrinsic call owner.
    if (associated) {
        fn.entity = explicit_template(name,merge_lookup(binding.entity,associated),s);
        fn.form = ExpressionForm::Overload; fn.category = ValueCategory::Lvalue; fn.ready = true;
        expressions.set(callee,fn); facts.edit(callee).entity = fn.entity; facts.edit(callee).scope = s;
    } else fn = expression(callee,s);
    NodeId object_node = member ? ast[designator].first : 0;
    TypeId object_type = member ? expressions[object_node].type : 0;
    auto category = member ? expressions[object_node].category : ValueCategory::Lvalue;
    if (member && ast[designator].op == OP_ARROW) { object_type = types[decay(object_type)].child; category = ValueCategory::Lvalue; }
    auto naming = object_uses[expressions[designator].object_use].naming_scope;
    TypeId ft = 0; EntityId selected = 0; std::vector<Conversion> chosen;
    if (direct) {
        auto choice = select_call(fn.entity,{},&args,object_type,category,naming,0,chosen);
        if (choice.failure == CallFailure::NoViable) throw std::runtime_error("no viable fixed template call");
        if (choice.failure == CallFailure::Ambiguous) throw std::runtime_error("ambiguous fixed template call");
        selected = choice.entity; ft = entities[selected].type;
        if (deleted_transfer(selected)) throw std::runtime_error("deleted fixed template callee");
        check_access(selected,s,naming,object_type);
        if (member) {
            bool nonstatic = entities[selected].member_info && !entities[selected].is_static;
            if (nonstatic) {
                Expression object; object.type = object_type; object.category = category;
                check_fixed_conversion(object,object_node,chosen[0],s);
            }
            chosen.erase(chosen.begin());
            record_object(result,object_node,nonstatic ? types.parameters[types[call_type(selected)].offset] : 0,
                nonstatic ? base_steps(object_type,scopes[entities[selected].owner].entity) : 0);
            auto& use = object_uses[result.object_use]; use.source_owned = true;
            if (nonstatic && ast[name].first == ast[name].last) use.virtual_slot = members[entities[selected].member_info].virtual_slot;
        }
        auto f = types[ft];
        for (unsigned i = args.size(); i < f.count; ++i) {
            auto a = default_argument(selected,i);
            args.push_back(a); chosen.push_back(conversion(a,types.parameters[f.offset+i]));
        }
        for (auto c = callee;; c = ast[c].first) {
            auto value = expressions[c]; value.entity = selected; value.form = ExpressionForm::Ordinary; value.type = ft;
            expressions.set(c,value); facts.edit(c).entity = selected; facts.edit(c).type = call_type(selected);
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
    result.type = value_type(f.child);
    result.category = types[f.child].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[f.child].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    if (class_value(result.type) && result.category == ValueCategory::Prvalue) {
        complete_class(types[result.type].entity); reject_abstract(result.type);
        default_destructor(result.type,s,false);
    }
    // No argument application or result temporary belongs to the definition.
    store_call(result,args,chosen); result.ready = true; result.inputs = CallInputs::Source;
    expressions.set(n,result); facts.edit(n).type = f.child; facts.edit(n).scope = s; facts.edit(n).entity = selected;
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
        auto receiver = object_uses[result.object_use];
        if (receiver.source_owned) receiver = project_object_use(receiver,n);
        if (receiver.node) expression(receiver.node,s);
        auto pattern = ast[source].first;
        for (auto c = callee;; c = ast[c].first, pattern = ast[pattern].first) {
            expressions.inherit(c,pattern);
            expressions.set(c,expressions[pattern]); expressions.evaluated(c,!unevaluated_depth);
            facts.edit(c).type = facts[pattern].type; facts.edit(c).entity = selected; facts.edit(c).scope = s;
            if (ast[c].kind != Kind::Parenthesized) break;
        }
        use_selected_function(selected,!receiver.virtual_slot);
    } else {
        expression(callee,s);
        auto incoming = expressions[ast[source].first].incoming;
        auto conversion = conversions[incoming]; apply_conversion(callee,conversion);
        expressions.incoming(callee,incoming);
    }
    std::vector<NodeId> args; std::vector<Conversion> chosen;
    bool materialize = false;
    for (unsigned i = 0; i < result.argument_count; ++i) {
        auto original = call_argument(result,i);
        auto a = ast.projected(original,context); if (!a) a = original; // Declaration-owned default.
        expression(a,s); args.push_back(a);
        auto c = copy_conversion_recipe(conversions[result.conversions+i]);
        materialize |= c.kind == Conversion::Kind::Construction || c.kind == Conversion::Kind::User || c.kind == Conversion::Kind::ListPlan;
        chosen.push_back(c);
    }
    if (materialize) record_call(result,args,chosen);
    else {
        for (unsigned i = 0; i < args.size(); ++i) {
            apply_conversion(args[i],chosen[i]); expressions.incoming(args[i],result.conversions+i);
        }
        result.arguments = expressions.argument_slice(source); result.inputs = CallInputs::Source;
    }
    facts.edit(n).type = facts[source].type; facts.edit(n).entity = selected;
}
NodeId Analyzer::call_argument(const Expression& call, unsigned i) const
{
    if (call.inputs != CallInputs::Context) return call_arguments[call.arguments+i];
    auto node = call.arguments;
    auto source = call_arguments[expressions.argument_slice(node)+i];
    auto context = ast.nodes.occurrences[node].context;
    if (context) if (auto use = ast.projected(source,context)) return use;
    return source; // A declaration-owned default can lie outside the caller.
}
} }
