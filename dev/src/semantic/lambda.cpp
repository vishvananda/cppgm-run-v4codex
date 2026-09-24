#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
void check_declarator(const syntax::AstView& ast, NodeId d)
{
    for (auto q = ast[d].first; q; q = ast[q].next) {
        auto k = ast[q].kind;
        if (k == Kind::CvQualifier || k == Kind::VirtSpecifier ||
            (k == Kind::FunctionQualifier && (ast[q].op == OP_AMP || ast[q].op == OP_LAND)))
            throw std::runtime_error("invalid lambda qualifier");
        if (k != Kind::Parameters) continue;
        for (auto p = ast[q].first; p; p = ast[p].next) {
            if (ast[p].kind != Kind::Parameter) continue;
            for (auto c = ast[p].first; c; c = ast[c].next)
                if (ast[c].kind == Kind::DefaultArgument) throw std::runtime_error("lambda parameter default");
            for (auto c = ast[ast[p].first].first; c; c = ast[c].next)
                if (ast[c].op == KW_AUTO) throw std::runtime_error("generic lambda is not C++11");
        }
    }
}
NodeId inferred_return(const syntax::AstView& ast, NodeId body)
{
    auto first = ast[body].first;
    return first == ast[body].last && ast[first].kind == Kind::Return && ast[first].first &&
        ast[ast[first].first].kind != Kind::BracedInit ? first : 0;
}
}
void Analyzer::bind_lambda_body(NodeId n, ScopeId s)
{
    auto body = child(n,Kind::Compound), d = child(n,Kind::LambdaDeclarator);
    auto source = ast.nodes.occurrences[body].source;
    auto state = template_bound_bodies.get(source);
    if (state == unsigned(FactState::Success)) return;
    if (state) throw std::runtime_error("recursive or failed lambda body binding");
    check_declarator(ast,d);
    auto signature = d ? declarator(d,types.fundamental(FT_VOID),s) : types.function(types.fundamental(FT_VOID),{},false);
    if (d) {
        auto source = ast.nodes.occurrences[d].source;
        template_signature_sources.put(source,d); template_type_sources.put(source,signature+1);
    }
    auto f = types[signature];
    std::vector<TypeId> params(types.parameters.begin()+f.offset,types.parameters.begin()+f.offset+f.count);
    auto trailing = child(d,Kind::TrailingReturn);
    TypeId result = !trailing && inferred_return(ast,body) ? 0 : f.child;
    auto fn = make_entity(EntityKind::Function,s,0,body);
    entities[fn].type = types.function(result,params,f.variadic);
    entities[fn].template_pattern = true;
    bind_template_body({body,d,s,fn,body});
}
Expression Analyzer::lambda_expression(NodeId n, ScopeId s)
{
    if (unevaluated_depth != body_evaluation_depth) throw std::runtime_error("lambda in unevaluated operand");
    if (auto known = closure_occurrences.get(n)) {
        Expression result; result.type = entities[closures[known].entity].type; return result;
    }
    if (ast[child(n,Kind::LambdaIntroducer)].first) throw std::runtime_error("capturing lambda outside implemented surface");
    auto d = child(n,Kind::LambdaDeclarator), body = child(n,Kind::Compound);
    demand_region(body);
    check_declarator(ast,d);
    auto signature = d ? declarator(d,types.fundamental(FT_VOID),s) : types.function(types.fundamental(FT_VOID),{},false);
    // Keep raw parameter facts for the body, but publish the adjusted callable
    // signature just as an ordinary function declaration does.
    signature = types.signature(signature);
    auto f = types[signature];
    std::vector<TypeId> params(types.parameters.begin()+f.offset,types.parameters.begin()+f.offset+f.count);
    bool variadic = f.variadic;
    unsigned cv = child(d,Kind::LambdaSpecifier) ? 0 : 1;
    auto trailing = child(d,Kind::TrailingReturn);
    auto result_type = f.child;
    auto label = "__lambda_"+std::to_string(n);
    auto name = ids.intern(TextView(label.data(),label.size()));
    auto cls = make_entity(EntityKind::Type,s,name,n);
    entities[cls].key = KW_CLASS;
    entities[cls].type = types.named(cls);
    entities[cls].scope = make_scope(ScopeKind::Class,s,name,cls,false);
    entities[cls].complete = true; entities[cls].definition = n;
    entities[cls].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
    auto info = entities[cls].class_info;
    class_facts[info].aggregate = false;
    auto fn = declare_function(entities[cls].scope,operator_name(OP_LPAREN),body,types.function(result_type,params,variadic,cv));
    entities[fn].key = OP_LPAREN; entities[fn].inline_function = true;
    member_facts(fn);
    auto m = entities[fn].member_info;
    members[m].body = body; members[m].source = body; members[m].declarator = d; members[m].in_class_body = true;
    Closure closure; closure.entity = cls; closure.function = fn; closure.source = n;
    closure.signature = types.function(types.fundamental(FT_VOID),params,variadic);
    for (auto scope = s; scope; scope = scopes[scope].parent)
        if (scopes[scope].kind == ScopeKind::Function) { closure.enclosing = scopes[scope].entity; break; }
    class_facts[info].local_function = closure.enclosing;
    if (!trailing) closure.inferred_return = inferred_return(ast,body);
    auto id = closures.size(); closures.push_back(closure);
    closure_entities.put(cls,id); closure_functions.put(fn,id); closure_occurrences.put(n,id);
    // Check the retained body immediately, but keep its odr-use edges dormant
    // until the call operator is selected. This is a body scope, never a copy
    // of syntax disguised as an ordinary class declaration.
    auto saved_depth = body_evaluation_depth;
    ++unevaluated_depth;
    try { function_body({body,d,entities[cls].scope,fn,body}); }
    catch (...) { --unevaluated_depth; body_evaluation_depth = saved_depth; throw; }
    --unevaluated_depth; body_evaluation_depth = saved_depth;
    // The checked operator scope owns the actual parameter identities (and
    // expanded packs). Exception expressions use that scope and the ordinary
    // contextual-bool/constant rules, independently of runtime body demand.
    exception_specification(fn,d,entities[fn].scope);
    demand_exception_specification(fn);
    auto call_type = types[entities[fn].type];
    auto pointer = types.compound(TypeKind::Pointer,types.function(call_type.child,params,variadic));
    auto conversion_name = ids.intern(TextView("__closure_conversion",20));
    auto conversion = make_entity(EntityKind::Function,entities[cls].scope,conversion_name,n);
    entities[conversion].type = types.function(pointer,{},false,1);
    entities[conversion].inline_function = true; entities[conversion].exception_spec = 129;
    member_facts(conversion);
    auto cm = entities[conversion].member_info;
    members[cm].conversion_target = pointer;
    members[cm].synthetic = members[cm].in_class_body = true;
    class_facts[info].first_conversion = conversion;
    auto thunk = make_entity(EntityKind::Function,entities[cls].scope,ids.intern(TextView("_FUN",4)),n);
    entities[thunk].type = types[pointer].child;
    entities[thunk].is_static = entities[thunk].inline_function = true;
    entities[thunk].exception_spec = entities[fn].exception_spec;
    member_facts(thunk);
    members[entities[thunk].member_info].synthetic = members[entities[thunk].member_info].in_class_body = true;
    closures[id].conversion = conversion; closures[id].thunk = thunk;
    closure_adapters.put(conversion,id); closure_adapters.put(thunk,id);
    conversion_bindings.put(key(entities[cls].scope,pointer),conversion);
    Expression result; result.type = entities[cls].type; return result;
}
void Analyzer::finish_closures()
{
    // Semantic evaluation order is unrelated to source/ABI numbering. Sort
    // only the closure records once, then number each typed signature group.
    std::vector<unsigned> order;
    for (unsigned i = 1; i < closures.size(); ++i) order.push_back(i);
    std::sort(order.begin(),order.end(),[&](unsigned a,unsigned b) {
        const auto& x = closures[a]; const auto& y = closures[b];
        if (x.enclosing != y.enclosing) return x.enclosing < y.enclosing;
        return ast.nodes.occurrences[x.source].source < ast.nodes.occurrences[y.source].source;
    });
    Index counts;
    for (auto i : order) {
        auto& closure = closures[i]; auto identity = key(closure.enclosing,closure.signature);
        closure.ordinal = counts.get(identity); counts.put(identity,closure.ordinal+1);
    }
}
} }
