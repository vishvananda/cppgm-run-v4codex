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
            for (auto c = ast[ast[p].first].first; c; c = ast[c].next)
                if (ast[c].op == KW_AUTO) throw std::runtime_error("generic lambda is not C++11");
        }
    }
}
}
void Analyzer::bind_lambda_body(NodeId n, ScopeId s)
{
    auto body = child(n,Kind::Compound), d = child(n,Kind::LambdaDeclarator);
    auto source = ast.nodes.occurrences[body].source;
    auto state = template_bound_bodies.get(source);
    if (state == unsigned(FactState::Success)) return;
    if (state) throw std::runtime_error("recursive or failed lambda body binding");
    auto head = child(n,Kind::TemplateParameters);
    if (head) {
        if (!ast[ast[head].first].first) throw std::runtime_error("lambda template head cannot be empty");
        s = make_scope(ScopeKind::Template,s);
        declare_template_parameters(head,s);
        template_source_heads.put(ast.nodes.occurrences[n].source,retain_template_head(s));
        check_template_parameters(d,s);
        check_template_parameters(body,s);
    }
    check_declarator(ast,d);
    auto signature = d ? declarator(d,types.fundamental(FT_VOID),s) : types.function(types.fundamental(FT_VOID),{},false);
    if (d) {
        auto source = ast.nodes.occurrences[d].source;
        template_signature_sources.put(source,d); template_type_sources.put(source,signature+1);
    }
    auto f = types[signature];
    std::vector<TypeId> params(types.parameters.begin()+f.offset,types.parameters.begin()+f.offset+f.count);
    auto trailing = child(d,Kind::TrailingReturn);
    TypeId result = !trailing ? placeholder_type() : f.child;
    auto fn = make_entity(EntityKind::Function,s,0,body);
    entities[fn].type = types.function(result,params,f.variadic);
    entities[fn].template_pattern = true;
    if (head) {
        auto index = template_source_heads.get(ast.nodes.occurrences[n].source);
        entities[fn].template_info = index;
        templates[index].source = n; templates[index].declarator = d; templates[index].body = body;
        template_declaration_sources.put(ast.nodes.occurrences[n].source,fn);
        if (d) template_declaration_sources.put(ast.nodes.occurrences[d].source,fn);
        exception_specification(fn,d,s);
    }
    unsigned mode = 0;
    for (auto c = ast[child(n,Kind::LambdaIntroducer)].first; c; c = ast[c].next) {
        auto op = ast[c].op;
        if ((op == OP_AMP && !ast[c].detail) || op == OP_ASS) { mode = op == OP_AMP ? 1 : 2; continue; }
        if (op == KW_THIS) continue;
        auto name = op == OP_AMP ? ast[ast[c].detail].text : ast[c].text;
        if (auto object = lookup(s,name,Lookup::Ordinary)) closure_pattern_captures.put(key(fn,object),op == OP_AMP ? 1 : 2);
    }
    closure_patterns.put(fn,1 | (mode << 1) | (child(d,Kind::LambdaSpecifier) ? 0 : 8));
    bind_template_defaults(d,s,head ? s : 0);
    bind_template_body({body,d,s,fn,body});
    bind_template_captures(n,s,fn);
}
Expression Analyzer::lambda_expression(NodeId n, ScopeId s)
{
    if (unevaluated_depth != body_evaluation_depth) throw std::runtime_error("lambda in unevaluated operand");
    if (auto known = closure_occurrences.get(n)) {
        Expression result; result.type = entities[closures[known].entity].type; return result;
    }
    auto occurrence = n;
    auto head = child(n,Kind::TemplateParameters);
    if (head) bind_lambda_body(n,s);
    auto d = child(n,Kind::LambdaDeclarator), body = child(n,Kind::Compound);
    // The template head is a lexical overlay. Rebind only its own parameters;
    // the immutable enclosing frame keeps captures and outer template facts.
    auto source_head = head ? template_source_heads.get(ast.nodes.occurrences[n].source) : 0;
    auto enclosing_frame = template_type_contexts.get(ast.nodes.occurrences[n].context);
    ScopeId head_scope = 0;
    if (head) {
        head_scope = make_scope(ScopeKind::Template,s);
        declare_template_parameters(head,head_scope,source_head,enclosing_frame);
        auto source = templates[source_head];
        std::vector<ArgumentId> arguments;
        for (auto decl = scopes[head_scope].first_decl; decl; decl = declarations[decl].next) {
            auto p = declarations[decl].entity;
            if (!entities[p].template_parameter) continue;
            auto arg = parameter_argument(p);
            arguments.push_back(entities[p].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
        }
        auto frame = substitution_frame(0,source.offset,source.count,enclosing_frame,intern_arguments(arguments));
        auto context = ast.new_context();
        n = ast.instantiate(n,context); attach_template_context(context,frame);
        facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
        d = child(n,Kind::LambdaDeclarator); body = child(n,Kind::Compound);
        for (auto decl = scopes[head_scope].first_decl; decl; decl = declarations[decl].next) {
            auto p = declarations[decl].entity;
            if (entities[p].initializer) entities[p].initializer = ast.projected(entities[p].initializer,context);
        }
    }
    if (!head) demand_region(body);
    check_declarator(ast,d);
    auto signature = d ? declarator(d,types.fundamental(FT_VOID),head ? head_scope : s) : types.function(types.fundamental(FT_VOID),{},false);
    // Keep raw parameter facts for the body, but publish the adjusted callable
    // signature just as an ordinary function declaration does.
    signature = types.signature(signature);
    auto f = types[signature];
    std::vector<TypeId> params(types.parameters.begin()+f.offset,types.parameters.begin()+f.offset+f.count);
    bool variadic = f.variadic;
    unsigned cv = child(d,Kind::LambdaSpecifier) ? 0 : 1;
    auto trailing = child(d,Kind::TrailingReturn);
    auto result_type = trailing ? f.child : placeholder_type();
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
    if (head) {
        template_facts(fn,head_scope);
        auto index = entities[fn].template_info;
        templates[index].source = n; templates[index].body = body; templates[index].declarator = d;
        templates[index].source_parameters = templates[source_head].offset;
        templates[index].source_count = templates[source_head].count;
        templates[index].parent_frame = enclosing_frame;
    }
    entities[fn].key = OP_LPAREN; entities[fn].inline_function = true;
    declaration_attributes(fn,0,n,d);
    member_facts(fn);
    auto m = entities[fn].member_info;
    members[m].body = body; members[m].source = body; members[m].declarator = d; members[m].in_class_body = true;
    Closure closure; closure.entity = cls; closure.function = fn; closure.source = n;
    closure.signature = types.function(types.fundamental(FT_VOID),params,variadic);
    closure.signature_group = intern_arguments({unsigned(bool(head)),head ?
        template_declaration_shape(closure.signature,head_scope) : closure.signature});
    for (auto scope = s; scope; scope = scopes[scope].parent)
        if (scopes[scope].kind == ScopeKind::Function) { closure.enclosing = scopes[scope].entity; break; }
    closure.parent = closure_functions.get(closure.enclosing);
    closure.this_type = implicit_object_type(s);
    class_facts[info].local_function = closure.enclosing;
    auto id = closures.size(); closures.push_back(closure);
    closure_entities.put(cls,id); closure_functions.put(fn,id); closure_occurrences.put(occurrence,id);
    prepare_captures(id,s);
    function_defaults(fn,d,head ? head_scope : s,body);
    if (head) {
        prepare_template_captures(id,s);
        prepare_capture_initializers(id,s);
        exception_specification(fn,d,head_scope);
        if (!closures[id].has_introducer) prepare_closure_conversion(id);
        Expression result; result.type = entities[cls].type; return result;
    }
    // Check the retained body immediately, but keep its odr-use edges dormant
    // until the call operator is selected. This is a body scope, never a copy
    // of syntax disguised as an ordinary class declaration.
    auto saved_depth = body_evaluation_depth;
    ++unevaluated_depth;
    try { function_body({body,d,entities[cls].scope,fn,body}); }
    catch (...) { --unevaluated_depth; body_evaluation_depth = saved_depth; throw; }
    --unevaluated_depth; body_evaluation_depth = saved_depth;
    prepare_capture_initializers(id,s);
    // The checked operator scope owns the actual parameter identities (and
    // expanded packs). Exception expressions use that scope and the ordinary
    // contextual-bool/constant rules, independently of runtime body demand.
    exception_specification(fn,d,entities[fn].scope);
    demand_exception_specification(fn);
    if (closures[id].has_introducer) {
        Expression result; result.type = entities[cls].type; return result;
    }
    prepare_closure_conversion(id);
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
    Index counts, local_counts;
    for (auto i : order) {
        auto& closure = closures[i]; auto identity = key(closure.enclosing,closure.signature_group);
        closure.ordinal = counts.get(identity); counts.put(identity,closure.ordinal+1);
        closure.local_ordinal = local_counts.get(closure.enclosing);
        local_counts.put(closure.enclosing,closure.local_ordinal+1);
    }
}
} }
