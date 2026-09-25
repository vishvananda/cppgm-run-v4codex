#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
struct RecipeScope {
    unsigned& depth;
    RecipeScope(unsigned& d) : depth(d) { ++depth; }
    ~RecipeScope() { --depth; }
};
}
bool Analyzer::decltype_call_result(NodeId n) const
{
    for (auto node = template_decltype_operand; node;) {
        if (node == n) return true;
        auto kind = ast[node].kind;
        if (kind == Kind::Parenthesized) node = ast[node].first;
        else if (kind == Kind::Binary && ast[node].op == OP_COMMA) node = ast[ast[node].first].next;
        else break;
    }
    return false;
}
bool Analyzer::check_fixed_operator(NodeId n, ScopeId s)
{
    auto node = ast[n];
    std::vector<NodeId> args;
    auto op = node.op;
    if (node.kind == Kind::Call) {
        auto callee = node.first, first = ast[ast[callee].next].first;
        if (invoke_expression(n,s)) { callee = first; first = ast[first].next; }
        if (!expressions[callee].ready || !class_value(expressions[callee].type)) return false;
        args.push_back(callee); op = OP_LPAREN;
        for (auto a = first; a; a = ast[a].next) args.push_back(a);
    } else {
        if (node.kind == Kind::Subscript) op = OP_LSQUARE;
        for (auto a = node.first; a; a = ast[a].next) args.push_back(a);
    }
    bool named = false;
    for (auto a : args) {
        if (!template_fixed_expressions.get(ast.nodes.occurrences[a].source)) return false;
        named |= class_value(expressions[a].type);
    }
    if (!named) return false;
    if (node.kind == Kind::Postfix) args.push_back(0);
    RecipeScope guard(unevaluated_depth);
    Expression result;
    if (node.kind == Kind::Conditional) {
        TypeQuery query; query.context = s;
        std::vector<TypeQueryFact> children;
        for (auto a : args) { TypeQueryFact fact; fact.expression = expressions[a]; children.push_back(fact); }
        result = query_conditional(query,children).expression;
        std::vector<Conversion> selected(conversions.begin()+result.conversions,conversions.begin()+result.conversions+result.count);
        store_call(result,args,selected); result.inputs = CallInputs::Source;
    } else if (!operator_expression(n,s,op,args,result,true)) return false;
    if (!decltype_call_result(n) && class_value(result.type) && result.category == ValueCategory::Prvalue) {
        complete_class(types[result.type].entity); reject_abstract(result.type);
        default_destructor(result.type,s,false);
    }
    if (node.kind == Kind::Call && invoke_expression(n,s)) {
        if (!result.object_use) record_object(result,0,0,0);
        object_uses[result.object_use].callee = args[0];
        object_uses[result.object_use].source_owned = true;
    }
    result.ready = true;
    expressions.set(n,result);
    { auto& f = facts.edit(n); f.scope = s; if (!f.type) f.type = result.type; }
    template_operator_expressions.put(ast.nodes.occurrences[n].source,n);
    ++template_fixed_call_work;
    return true;
}
void Analyzer::reuse_fixed_operator(NodeId n, NodeId source, ScopeId s, Expression& result)
{
    ++template_fixed_call_uses;
    result = expressions[source]; result.incoming = 0;
    auto context = ast.nodes.occurrences[n].context;
    auto selected = facts[source].entity;
    auto receiver = object_uses[result.object_use];
    if (receiver.source_owned) receiver = project_object_use(receiver,n);
    if (receiver.node) expression(receiver.node,s);
    if (receiver.callee_conversion) {
        auto c = copy_conversion_recipe(conversions[receiver.callee_conversion]);
        apply_conversion(receiver.node,c);
        receiver.node = 0;
        receiver.callee_conversion = conversions.size(); conversions.push_back(c);
        result.object_use = object_uses.size(); object_uses.push_back(receiver);
    } else if (selected) use_selected_function(selected,!receiver.virtual_slot);
    std::vector<NodeId> args; std::vector<Conversion> chosen;
    unsigned supplied = result.argument_count;
    if (ast[source].kind == Kind::Call) {
        supplied = 0;
        for (auto a = ast[ast[ast[source].first].next].first; a; a = ast[a].next) ++supplied;
        if (object_uses[expressions[source].object_use].callee) --supplied;
    }
    for (unsigned i = 0; i < result.argument_count; ++i) {
        if (i >= supplied && selected) default_argument(selected,i);
        auto source_arg = call_argument(result,i);
        auto arg = ast.projected(source_arg,context); if (!arg) arg = source_arg;
        if (arg) expression(arg,s);
        args.push_back(arg); chosen.push_back(copy_conversion_recipe(conversions[result.conversions+i]));
    }
    // Builtin compound assignment retains its arithmetic type after the two
    // operand conversions. It has no additional evaluated argument.
    for (unsigned i = result.argument_count; i < result.count; ++i)
        chosen.push_back(conversions[result.conversions+i]);
    record_call(result,args,chosen); result.count = chosen.size();
    if (result.form == ExpressionForm::ListValue) {
        record_object(result,0,0,0);
        object_uses[result.object_use].temporary = converted_temporary(conversions[result.conversions]);
    }
    auto& f = facts.edit(n); f.entity = selected; f.type = facts[source].type;
}
bool Analyzer::check_fixed_construction(NodeId n, ScopeId s)
{
    auto callee = ast[n].first;
    if (ast[callee].kind != Kind::IdExpression) return false;
    TypeId type = fundamental_cast_type(ast[callee].op);
    auto name = ast[callee].detail;
    if (!type) {
        if (ast[name].kind != Kind::Name) return false;
        auto binding = template_bindings[template_binding_index.get(ast.nodes.occurrences[name].source)];
        auto e = binding.entity;
        if (!e || binding.dependent || (entities[e].kind != EntityKind::Type && entities[e].kind != EntityKind::Alias)) return false;
        type = entities[e].type;
    }
    if (!type || dependent_type(type)) return false;
    auto list = ast[callee].next;
    // A braced class or array value shares the ordinary list-plan owner;
    // the concrete use owns the plan's one materialized temporary.
    if (ast[list].kind == Kind::BracedInit && (class_value(type) || types[type].kind == TypeKind::Array)) {
        if (!fixed_initializer_operands(list)) return false;
        RecipeScope guard(unevaluated_depth);
        expression(list,s);
        auto conversion = list_initialization(list,type,s,true);
        check_fixed_conversion(Expression(),list,conversion,s);
        Expression result; result.type = type; result.form = ExpressionForm::ListValue; result.ready = true;
        store_call(result,{list},{conversion}); result.inputs = CallInputs::Source;
        expressions.set(n,result); facts.edit(n).type = type; facts.edit(n).scope = s;
        template_operator_expressions.put(ast.nodes.occurrences[n].source,n); ++template_fixed_call_work; return true;
    }
    for (auto a = ast[list].first; a; a = ast[a].next)
        if (!template_fixed_expressions.get(ast.nodes.occurrences[a].source)) return false;
    if (!class_value(type)) {
        if (ast[list].first != ast[list].last) throw std::runtime_error("fixed scalar cast arity");
        if (ast[list].kind == Kind::BracedInit && ast[list].first)
            list_conversion_from(ast[list].first,expressions[ast[list].first].type,value_type(type));
        return check_fixed_cast(n,s,type,ast[list].first);
    }
    RecipeScope guard(unevaluated_depth);
    complete_class(types[type].entity); reject_abstract(type);
    if (!check_template_constructor(list,type,s,InitializationMode::Direct)) return false;
    auto result = expressions[list]; result.form = ExpressionForm::Construction;
    expressions.set(n,result);
    { auto& f = facts.edit(n); f.entity = facts[list].entity; f.type = type; f.scope = s; }
    return true;
}
bool Analyzer::check_fixed_cast(NodeId n, ScopeId s, TypeId type, NodeId operand)
{
    if (dependent_type(type)) return false;
    if (operand && !template_fixed_expressions.get(ast.nodes.occurrences[operand].source)) return false;
    RecipeScope guard(unevaluated_depth);
    if (class_value(type)) {
        if (ast[n].op != KW_STATIC_CAST && ast[n].op != OP_LPAREN) return false;
        complete_class(types[type].entity); reject_abstract(type);
        std::vector<NodeId> args(1,operand);
        if (!check_template_constructor(n,type,s,InitializationMode::Direct,&args)) return false;
        auto result = expressions[n]; result.form = ExpressionForm::Construction;
        expressions.set(n,result); return true;
    }
    auto result = cast_expression(n,s,type,operand,true);
    result.ready = true; expressions.set(n,result); facts.edit(n).scope = s;
    template_operator_expressions.put(ast.nodes.occurrences[n].source,n); ++template_fixed_call_work; return true;
}
void Analyzer::reuse_fixed_construction(NodeId n, NodeId source, ScopeId s, Expression& result)
{
    auto list = ast[ast[n].first].next;
    std::vector<NodeId> args;
    if (ast[n].kind == Kind::Cast) { args.push_back(list); expression(list,s); list = n; }
    else for (auto a = ast[list].first; a; a = ast[a].next) { expression(a,s); args.push_back(a); }
    auto type = expressions[source].type;
    EntityId ctor = 0;
    if (!reuse_template_constructor(list,type,args,result,s,ctor)) throw std::logic_error("missing fixed construction recipe");
    if (converting_transfer(ctor,result)) {
        auto c = result_conversion(ctor,result,type);
        result = Expression(); result.type = type; result.form = ExpressionForm::Cast;
        record_conversion(result,args[0],c); facts.edit(n).type = type; return;
    }
    members[entities[ctor].member_info].complete_entry = true;
    result.type = type; result.form = ExpressionForm::Construction;
    record_object(result,0,0,0);
    object_uses[result.object_use].value_initialize = args.empty() && members[entities[ctor].member_info].synthetic && !members[entities[ctor].member_info].defaulted_late;
    if (object_uses[result.object_use].value_initialize) prepare_zero_initialization(type);
    auto& f = facts.edit(n); f.entity = ctor; f.type = type;
}
} }
