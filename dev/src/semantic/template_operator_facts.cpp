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
        auto kind = ast.kind(node);
        if (kind == Kind::Parenthesized) node = ast.first(node);
        else if (kind == Kind::Binary && ast.op(node) == OP_COMMA) node = ast.next(ast.first(node));
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
        auto callee = node.first, first = ast.first(ast.next(callee));
        if (invoke_expression(n,s)) { callee = first; first = ast.next(first); }
        if (!expressions[callee].ready || !class_value(expressions[callee].type)) return false;
        args.push_back(callee); op = OP_LPAREN;
        for (auto a = first; a; a = ast.next(a)) args.push_back(a);
    } else {
        if (node.kind == Kind::Subscript) op = OP_LSQUARE;
        for (auto a = node.first; a; a = ast.next(a)) args.push_back(a);
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
    record_discard_form(n,result); expressions.set(n,result);
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
    if (receiver.member_pointer && receiver.callee) {
        expression(receiver.member_pointer,s);
        if (receiver.invoke_dereference) {
            auto op = invoke_dereferences[receiver.invoke_dereference];
            prepare_range_operation(op,{expressions[receiver.node]},s,!unevaluated_depth);
            receiver.invoke_dereference = invoke_dereferences.size(); invoke_dereferences.push_back(op);
        }
        result.object_use = object_uses.size(); object_uses.push_back(receiver);
    }
    if (receiver.callee_conversion) {
        auto c = copy_conversion_recipe(conversions[receiver.callee_conversion]);
        apply_conversion(receiver.node,c);
        receiver.node = 0;
        receiver.callee_conversion = conversions.size(); conversions.push_back(c);
        result.object_use = object_uses.size(); object_uses.push_back(receiver);
    } else if (selected && result.form != ExpressionForm::TypeinfoEqual && result.form != ExpressionForm::TypeinfoUnequal)
        use_selected_function(receiver.callable_entry ? receiver.callable_entry : selected,!receiver.virtual_slot);
    std::vector<NodeId> args; std::vector<Conversion> chosen;
    unsigned supplied = result.argument_count;
    if (ast.kind(source) == Kind::Call) {
        supplied = 0;
        for (auto a = ast.first(ast.next(ast.first(source))); a; a = ast.next(a)) ++supplied;
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
    auto callee = ast.first(n);
    if (ast.kind(callee) != Kind::IdExpression) return false;
    TypeId type = fundamental_cast_type(ast.op(callee));
    auto name = ast.detail(callee);
    if (!type) {
        if (ast.kind(name) != Kind::Name) return false;
        auto binding = template_bindings[template_binding_index.get(ast.nodes.occurrences[name].source)];
        auto e = binding.entity;
        if (!e || binding.dependent || (entities[e].kind != EntityKind::Type && entities[e].kind != EntityKind::Alias)) return false;
        type = entities[e].type;
    }
    if (!type || dependent_type(type)) return false;
    auto list = ast.next(callee);
    // A braced class or array value shares the ordinary list-plan owner;
    // the concrete use owns the plan's one materialized temporary.
    if (ast.kind(list) == Kind::BracedInit && (class_value(type) || types[type].kind == TypeKind::Array || vector_kind(types[type].kind))) {
        if (!fixed_initializer_operands(list)) return false;
        RecipeScope guard(unevaluated_depth);
        expression(list,s);
        auto conversion = list_initialization(list,type,s,true);
        check_fixed_conversion(Expression(),list,conversion,s);
        Expression result; result.type = type; result.form = ExpressionForm::ListValue; result.ready = true;
        store_call(result,{list},{conversion}); result.inputs = CallInputs::Source;
        record_discard_form(n,result); expressions.set(n,result); facts.edit(n).type = type; facts.edit(n).scope = s;
        template_operator_expressions.put(ast.nodes.occurrences[n].source,n); ++template_fixed_call_work; return true;
    }
    for (auto a = ast.first(list); a; a = ast.next(a))
        if (!template_fixed_expressions.get(ast.nodes.occurrences[a].source)) return false;
    if (!class_value(type)) {
        if (ast.first(list) != ast.last(list)) throw std::runtime_error("fixed scalar cast arity");
        if (!check_fixed_cast(n,s,type,ast.first(list))) return false;
        if (ast.kind(list) == Kind::BracedInit && ast.first(list)) {
            auto c = conversions[expressions[n].conversions];
            list_conversion_from(ast.first(list),expressions[ast.first(list)].type,value_type(type),&c);
        }
        return true;
    }
    RecipeScope guard(unevaluated_depth);
    complete_class(types[type].entity); reject_abstract(type);
    if (!check_template_constructor(list,type,s,InitializationMode::Direct)) return false;
    auto result = expressions[list]; result.form = ExpressionForm::Construction;
    record_discard_form(n,result); expressions.set(n,result);
    { auto& f = facts.edit(n); f.entity = facts[list].entity; f.type = type; f.scope = s; }
    return true;
}
bool Analyzer::check_fixed_cast(NodeId n, ScopeId s, TypeId type, NodeId operand)
{
    if (dependent_type(type)) return false;
    if (ast.op(n) == OP_LPAREN && ast.kind(operand) == Kind::BracedInit) {
        if (!fixed_initializer_operands(operand)) return false;
        RecipeScope guard(unevaluated_depth);
        auto result = cast_expression(n,s,type,operand,true);
        result.ready = true; record_discard_form(n,result); expressions.set(n,result); facts.edit(n).scope = s;
        template_operator_expressions.put(ast.nodes.occurrences[n].source,n); ++template_fixed_call_work; return true;
    }
    if (operand && !template_fixed_expressions.get(ast.nodes.occurrences[operand].source)) return false;
    RecipeScope guard(unevaluated_depth);
    if (class_value(type)) {
        if (ast.op(n) != KW_STATIC_CAST && ast.op(n) != OP_LPAREN) return false;
        complete_class(types[type].entity); reject_abstract(type);
        std::vector<NodeId> args(1,operand);
        if (!check_template_constructor(n,type,s,InitializationMode::Direct,&args)) return false;
        auto result = expressions[n]; result.form = ExpressionForm::Construction;
        record_discard_form(n,result); expressions.set(n,result); return true;
    }
    auto result = cast_expression(n,s,type,operand,true);
    result.ready = true; record_discard_form(n,result); expressions.set(n,result); facts.edit(n).scope = s;
    template_operator_expressions.put(ast.nodes.occurrences[n].source,n); ++template_fixed_call_work; return true;
}
void Analyzer::reuse_fixed_construction(NodeId n, NodeId source, ScopeId s, Expression& result)
{
    auto list = ast.next(ast.first(n));
    std::vector<NodeId> args;
    if (ast.kind(n) == Kind::Cast) { args.push_back(list); expression(list,s); list = n; }
    else for (auto a = ast.first(list); a; a = ast.next(a)) { expression(a,s); args.push_back(a); }
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
