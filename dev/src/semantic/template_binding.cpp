#include "semantic/analyzer.h"
#include "support/type_traits.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
EntityId Analyzer::pattern_declaration(EntityKind kind, ScopeId s, IdentifierId name, NodeId source, bool dependent)
{
    if (kind == EntityKind::Parameter && name && local(s,name)) throw std::runtime_error("duplicate source parameter");
    auto e = make_entity(kind,s,name,source);
    entities[e].template_pattern = true;
    template_pattern_entities.put(e,dependent ? 2 : 1);
    // Binding a declaration in a pattern/prototype scope publishes identity.
    // It must not erase a raw type already established by the source signature.
    bind(s,name,e); record(s,e,source,facts[source].type,kind);
    template_declaration_sources.put(ast.nodes.occurrences[source].source,e);
    ++template_declaration_work;
    return e;
}
TemplateBinding Analyzer::bind_template_name(NodeId n, ScopeId s, NodeId last)
{
    if (!n) return TemplateBinding();
    bool full = !last || last == ast.last(n);
    if (!last) last = ast.last(n);
    auto source = ast.nodes.occurrences[n].source;
    bool pattern = full && !ast.nodes.occurrences[n].context;
    if (pattern) if (auto id = template_binding_index.get(source)) return template_bindings[id];
    ++template_binding_work;
    TemplateBinding r; r.scope = s;
    auto owner = ast.op(n) == OP_COLON2 ? global : s;
    bool qualified = ast.op(n) == OP_COLON2;
    for (auto p = ast.first(n); p; p = ast.next(p)) {
        if (full && p == last && ast.op(p) == KW_OPERATOR && ast.detail(p)) {
            r.entity = resolve_conversion_name(n,s);
            auto target = facts[ast.detail(p)].type;
            r.dependent |= dependent_type(target) || (r.entity && entities[r.entity].template_pattern);
            break;
        }
        if (ast.detail(p) && ast.kind(ast.detail(p)) == Kind::Decltype) {
            r.dependent |= bind_template_expression(ast.first(ast.detail(p)),s);
            if (r.dependent) break;
            auto t = expression_type(ast.first(ast.detail(p)),s,true);
            owner = types[t].kind == TypeKind::Named ? entities[types[t].entity].scope : 0;
            qualified = true; continue;
        }
        auto e = lookup(owner,full && p == last ? terminal(n) : ast.text(p),child(p,Kind::TemplateArguments) ? Lookup::Template : p == last ? Lookup::Ordinary : Lookup::Qualifier,qualified);
        if (!e) { r.entity = 0; break; }
        r.entity = e;
        if (p != last && entities[e].parameter_pack) r.qualifier_pack = e;
        auto args = child(p,Kind::TemplateArguments);
        bool class_id = args && entities[e].class_info && (entities[e].template_info || entities[e].specialization);
        r.dependent |= entities[e].template_parameter || entities[e].template_member || (!class_id && template_pattern_entities.get(e) == 2 &&
            (!entities[e].template_info || encloses(entities[e].scope,s))) ||
            (entities[e].template_pattern && !entities[e].type &&
             (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias)) ||
            (entities[e].type && dependent_type(entities[e].type));
        if (args) {
            for (auto a = ast.first(args); a; a = ast.next(a)) {
                // An argument expansion can expand types as well as values.
                // Its typed argument owner resolves that distinction; an
                // expression-only walk would reject a type pack's identifier.
                if (ast.kind(a) == Kind::PackExpression) {
                    r.dependent |= dependent_argument(template_argument_node(a,s)); continue;
                }
                // The parser retains unresolved template arguments as value
                // syntax when the owning class's aliases are not yet visible.
                if (ast.kind(a) == Kind::IdExpression) {
                    auto argument = bind_template_name(ast.detail(a),s);
                    if (argument.dependent || (argument.entity && (entities[argument.entity].kind == EntityKind::Type || entities[argument.entity].kind == EntityKind::Alias))) {
                        r.dependent |= argument.dependent; continue;
                    }
                }
                if (ast.kind(a) == Kind::Call && ast.kind(ast.first(a)) == Kind::IdExpression) {
                    auto target = bind_template_name(ast.detail(ast.first(a)),s).entity;
                    if (target && (entities[target].kind == EntityKind::Type || entities[target].kind == EntityKind::Alias)) {
                        auto argument = template_argument_node(a,s);
                        if (argument && !value_argument(argument)) {
                            r.dependent |= dependent_argument(argument); continue;
                        }
                    }
                }
                r.dependent |= bind_template_expression(a,s);
            }
            if (r.dependent && entities[e].template_info)
                for (auto a = ast.first(args); a; a = ast.next(a)) template_argument_node(a,s);
            if (!r.dependent && !entities[e].template_pattern) {
                r.entity = e = class_template_name(p,e,s);
                r.entity = e = variable_template_name(p,e,s);
                r.dependent |= entities[e].type && dependent_type(entities[e].type);
            }
        }
        if (p == last) break;
        if (r.dependent) {
            if (!ast.nodes.occurrences[n].context) {
                NodeId previous = p, prefix_part = 0, missing = 0;
                for (auto next = ast.next(p); next; previous = next, next = ast.next(next)) {
                    if (child(next,Kind::TemplateArguments) && !(ast.flags(next) & 1)) {
                        prefix_part = previous; missing = next;
                    }
                    if (next == last) break;
                }
                if (missing) {
                    // One qualifier traversal also checks introducers on any
                    // earlier dependent member template-ids in this name.
                    auto type = type_name(n,s,prefix_part);
                    if (dependent_type(type)) {
                        auto current = current_instantiation_scope(type,s);
                        auto member = current ? lookup(current,ast.text(missing),Lookup::Ordinary,true) : 0;
                        bool known = false;
                        for (auto candidate : candidates(member)) known |= entities[candidate].template_info != 0;
                        if (!known) throw std::runtime_error("dependent qualified template requires template");
                    }
                }
            }
            r.entity = 0; break;
        }
        auto cls = entities[e].class_info ? e : entities[e].kind == EntityKind::Alias ? types[entities[e].type].entity : 0;
        if (cls && entities[cls].class_info) complete_class(cls);
        owner = target(e); qualified = true;
        if (!owner) { r.entity = 0; break; }
    }
    // A retained nested function may first be bound in a concrete enclosing
    // specialization. Those bindings belong to that environment and cannot be
    // published as definition-wide source facts for other specializations.
    if (pattern) { template_binding_index.put(source,template_bindings.size()); template_bindings.push_back(r); }
    return r;
}
bool Analyzer::bind_template_expression(NodeId n, ScopeId s, bool callee)
{
    struct Context {
        bool& active; bool prior;
        Context(bool& a, bool excluded) : active(a), prior(a) { if (excluded) active = false; }
        ~Context() { active = prior; }
    } context(template_coroutine_context,n && (ast.kind(n) == Kind::DefaultArgument ||
        ast.kind(n) == Kind::Decltype || ast.kind(n) == Kind::Sizeof ||
        ast.kind(n) == Kind::TypeTrait || ast.kind(n) == Kind::Noexcept));
    bool dependent = bind_template_expression_impl(n,s,callee);
    if (n && !ast.nodes.occurrences[n].context) {
        auto kind = ast.kind(n);
        if (dependent && template_body_values) {
            template_value_dependence.put(ast.nodes.occurrences[n].source,1);
            if (kind == Kind::Sizeof || kind == Kind::SizeofPack || (kind == Kind::TypeTrait && BuiltinTrait(ast.flags(n)) != BuiltinTrait::Offsetof)) bind_template_size(n,s);
        }
        // Value dependence alone does not change scalar operand types or the
        // selected built-in conversions. Each checker still requires complete
        // fixed operand facts before publishing its source semantic decision.
        bool scalar = kind == Kind::IdExpression || kind == Kind::Binary || kind == Kind::Assignment || kind == Kind::Conditional ||
            kind == Kind::Unary || kind == Kind::Postfix || kind == Kind::Parenthesized || kind == Kind::Subscript ||
            kind == Kind::Call || kind == Kind::Member || kind == Kind::Cast || ast.op(n) == KW_TYPEID;
        if (!dependent || (template_body_values && scalar)) check_fixed_expression(n,s);
    }
    return dependent;
}
void Analyzer::check_coroutine_context(ScopeId s) const
{
    // A local class/default/unevaluated operand is not the enclosing
    // coroutine body. Nested function bodies establish their own context.
    while (s && (scopes[s].kind == ScopeKind::Block || scopes[s].kind == ScopeKind::Control ||
        scopes[s].kind == ScopeKind::Template)) s = scopes[s].parent;
    if (!template_coroutine_context || !s || scopes[s].kind != ScopeKind::Function)
        throw std::runtime_error("coroutine operation outside an evaluated function body");
}
bool Analyzer::bind_template_expression_impl(NodeId n, ScopeId s, bool callee)
{
    if (!n) return false;
    ++template_binding_work;
    auto node = ast[n];
    if (node.kind == Kind::Await || node.kind == Kind::Yield) {
        check_coroutine_context(s);
        for (auto c = node.first; c; c = ast.next(c)) bind_template_expression(c,s);
        // The promise/awaiter protocol is an obligation of the demanded
        // coroutine, even when an operand itself has a nondependent type.
        return true;
    }
    if (node.kind == Kind::Fold) {
        for (auto c = node.first; c; c = ast.next(c)) bind_template_expression(c,s);
        fold_query(n,s); return true;
    }
    if (node.kind == Kind::FunctionName) return true;
    if (node.kind == Kind::StatementExpression) { bind_template_statement(node.first,s); return true; }
    if (node.kind == Kind::Lambda) { bind_lambda_body(n,s); return true; }
    if (node.kind == Kind::ClassForward && !(node.flags & 2)) {
        if (bind_template_name(node.detail,s).dependent) return true;
        class_type(n,s,0,false); return false;
    }
    if (node.kind == Kind::Decltype) {
        struct Operand {
            NodeId& active; NodeId prior;
            Operand(NodeId& a, NodeId n) : active(a), prior(a) { active = n; }
            ~Operand() { active = prior; }
        } operand(template_decltype_operand,node.first);
        return bind_template_expression(node.first,s);
    }
    if (node.kind == Kind::IdExpression) {
        auto name = node.detail;
        if (callee && fundamental_cast_type(node.op)) return false;
        if (callee && ast.kind(name) == Kind::TypeId) return bind_template_expression(name,s);
        auto binding = bind_template_name(name,s);
        auto e = binding.entity;
        if (!binding.dependent && check_template_field(n,s,e)) return false;
        if (!binding.dependent && !e && !callee) {
            auto text = ids.spelling(terminal(name));
            auto location = static_cast<const syntax::Ast&>(ast).locations[node.location];
            auto path = ids.spelling(location.presumed_file);
            throw std::runtime_error("unbound template value name: " + std::string(text.data,text.size) +
                " at " + std::string(path.data,path.size) + ":" + std::to_string(location.line));
        }
        if (e && !callee && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias ||
            entities[e].kind == EntityKind::Namespace || entities[e].kind == EntityKind::NamespaceAlias))
            throw std::runtime_error("template expression requires a value name");
        return binding.dependent;
    }
    if (node.kind == Kind::Call) {
        bool dependent = false;
        auto fn = node.first;
        for (auto a = ast.first(ast.next(fn)); a; a = ast.next(a)) dependent |= bind_template_expression(a,s);
        dependent |= bind_template_expression(fn,s,true);
        if (ast.kind(fn) == Kind::IdExpression && !fundamental_cast_type(ast.op(fn)) && ast.kind(ast.detail(fn)) != Kind::TypeId) {
            auto name = ast.detail(fn); auto binding = bind_template_name(name,s);
            auto id = terminal(name);
            if (!binding.entity && !binding.dependent &&
                (!dependent || ast.first(name) != ast.last(name) || ast.op(name) == OP_COLON2) &&
                id != constant_builtin && id != abort_builtin && id != expect_builtin)
                query_fact(expression_query(n,s));
        }
        return dependent;
    }
    if (node.kind == Kind::Member) {
        bool dependent = bind_template_expression(node.first,s);
        auto name = ast.detail(ast.next(node.first));
        for (auto p = ast.first(name); p; p = ast.next(p))
            if (auto args = child(p,Kind::TemplateArguments))
                for (auto a = ast.first(args); a; a = ast.next(a))
                    dependent |= dependent_argument(template_argument_node(a,s));
        auto receiver = node.first;
        while (ast.kind(receiver) == Kind::Parenthesized) receiver = ast.first(receiver);
        bool current = node.op == OP_ARROW && ast.kind(receiver) == Kind::KeywordLiteral && ast.op(receiver) == KW_THIS;
        if (node.op == OP_DOT && ast.kind(receiver) == Kind::Unary && ast.op(receiver) == OP_STAR) {
            receiver = ast.first(receiver);
            while (ast.kind(receiver) == Kind::Parenthesized) receiver = ast.first(receiver);
            current = ast.kind(receiver) == Kind::KeywordLiteral && ast.op(receiver) == KW_THIS;
        }
        auto object = template_object_context(s);
        if (dependent && !ast.nodes.occurrences[n].context) {
            auto part = ast.last(name);
            if (child(part,Kind::TemplateArguments) && ast.op(part) != OP_COMPL && !(ast.flags(part) & 1)) {
                TypeId type = 0;
                if (part != ast.first(name)) {
                    auto previous = ast.first(name);
                    while (ast.next(previous) != part) previous = ast.next(previous);
                    auto qualifier = bind_template_name(name,s,previous);
                    if (qualifier.entity || qualifier.dependent) type = type_name(name,s,previous);
                    else throw std::runtime_error("unknown dependent qualifier requires template");
                } else if (current && object.owner) type = injected_template_type(object.owner,s);
                if (!type) {
                    auto query = expression_query(node.first,s);
                    type = value_type(query_fact(query).expression.type);
                    if (node.op == OP_ARROW && types[type].kind == TypeKind::Pointer) type = types[type].child;
                }
                if (dependent_type(type)) {
                    auto owner = current_instantiation_scope(type,s);
                    auto member = owner ? lookup(owner,ast.text(part),Lookup::Ordinary,true) : 0;
                    bool known = false;
                    for (auto candidate : candidates(member)) known |= entities[candidate].template_info != 0;
                    if (!known) throw std::runtime_error("dependent member template requires template");
                }
            }
        }
        if (current && object.owner && ast.first(name) == ast.last(name) && !child(ast.last(name),Kind::TemplateArguments)) {
            auto field = lookup(entities[object.owner].scope,terminal(name),Lookup::Ordinary,true);
            if (check_template_field(n,s,field,true)) return false;
        }
        return dependent;
    }
    if (node.kind == Kind::Name) return bind_template_name(n,s).dependent;
    if (node.kind == Kind::SizeofPack) {
        auto e = lookup(s,node.text);
        if (!e || !entities[e].parameter_pack) throw std::runtime_error("sizeof... requires a parameter pack");
        pack_size_entities.put(ast.nodes.occurrences[n].source,e);
        return true;
    }
    if (node.kind == Kind::Identifier) return false; // A declaration's own name.
    if (node.kind == Kind::TypeTrait && BuiltinTrait(node.flags) == BuiltinTrait::Offsetof) {
        bool dependent = dependent_type(type_id(node.first,s));
        for (auto c = ast.next(node.first); c; c = ast.next(c))
            if (ast.kind(c) == Kind::Subscript) dependent |= bind_template_expression(ast.first(c),s);
        return dependent;
    }
    if (node.kind == Kind::TypeTrait && node.flags) {
        bool dependent = false;
        for (auto c = node.first; c; c = ast.next(c))
            dependent |= dependent_argument(template_argument_node(c,s));
        return dependent;
    }
    if (node.kind == Kind::Sizeof || node.kind == Kind::TypeTrait) {
        ++unevaluated_depth; bool dependent = false;
        try { for (auto c = node.first; c; c = ast.next(c)) dependent |= bind_template_expression(c,s); }
        catch (...) { --unevaluated_depth; throw; }
        --unevaluated_depth; return dependent;
    }
    if (node.kind == Kind::KeywordLiteral && node.op == KW_THIS) {
        auto object = template_object_context(s);
        if (object.owner && !object.available) throw std::runtime_error("this in static template member");
    }
    bool dependent = node.kind == Kind::KeywordLiteral && node.op == KW_THIS;
    if (node.detail) dependent |= bind_template_expression(node.detail,s);
    for (auto c = node.first; c; c = ast.next(c)) dependent |= bind_template_expression(c,s);
    return dependent;
}
void Analyzer::bind_template_body(const Body& body)
{
    auto source = ast.nodes.occurrences[body.node].source;
    auto state = template_bound_bodies.get(source);
    if (state == unsigned(FactState::Success)) return;
    if (state == unsigned(FactState::Failure)) throw std::runtime_error("failed template body binding");
    if (state == unsigned(FactState::Active)) throw std::runtime_error("recursive template body binding");
    template_bound_bodies.put(source,unsigned(FactState::Active));
    struct ControlBinding {
        Analyzer& sem; TypeId result; unsigned loops, switches;
        std::vector<SwitchContext> contexts;
        ControlBinding(Analyzer& a, TypeId type) : sem(a), result(a.return_type),
            loops(a.loop_depth), switches(a.switch_depth) {
            contexts.swap(sem.switches); sem.loop_depth = sem.switch_depth = 0;
            sem.return_type = type;
        }
        ~ControlBinding() {
            contexts.swap(sem.switches); sem.loop_depth = loops; sem.switch_depth = switches;
            sem.return_type = result;
        }
    } control(*this,types[entities[body.entity].type].child);
    try {
    check_constexpr_signature(body.entity);
    auto owner = entities[body.entity].template_info ? templates[entities[body.entity].template_info].environment : body.owner;
    auto fs = make_scope(ScopeKind::Function,owner,entities[body.entity].name,body.entity,false);
    if (closure_patterns.get(body.entity)) entities[body.entity].scope = fs;
    template_pattern_scopes.put(fs,1);
    auto d = body.declarator; NodeId params = 0;
    while (d) {
        if (auto p = child(d,Kind::Parameters)) params = p;
        auto nested = child(d,Kind::NestedDeclarator); d = nested ? ast.first(nested) : 0;
    }
    bind_template_object_context(fs,params);
    unsigned ordinal = 0;
    for (auto p = ast.first(params); p; p = ast.next(p)) {
        if (ast.kind(p) != Kind::Parameter) continue;
        auto specs = ast.first(p), decl = ast.next(specs);
        bool dependent = bind_template_expression(specs,fs) | bind_template_expression(decl,fs);
        auto declared_type = facts[p].type;
        if (!declared_type) declared_type = bind_template_type(specs,decl,fs);
        auto e = pattern_declaration(EntityKind::Parameter,fs,terminal(decl_name(decl)),p,dependent);
        entities[e].parameter_pack = declarator_pack(decl) != 0;
        // Type queries can refer to an earlier parameter while the signature
        // is being instantiated, before runtime parameter objects exist.
        signature_parameters.put(e,++ordinal);
        if (declared_type) {
            facts.edit(p).type = declared_type;
            entities[e].type = parameter_body_type(declared_type);
        }
    }
    struct ValueBinding {
        bool& mode; bool prior; bool& coroutine; bool prior_coroutine;
        ValueBinding(bool& value, bool& c) : mode(value), prior(value), coroutine(c), prior_coroutine(c) {
            mode = coroutine = true;
        }
        ~ValueBinding() { mode = prior; coroutine = prior_coroutine; }
    } value_binding(template_body_values,template_coroutine_context);
    // A lambda's declarator sees its parameter names. Retain fixed lookup
    // here, before later declarations can affect an enclosing instantiation.
    if (ast.kind(body.declarator) == Kind::LambdaDeclarator)
        for (auto q = ast.first(body.declarator); q; q = ast.next(q))
            if ((ast.kind(q) == Kind::FunctionQualifier || ast.kind(q) == Kind::Noexcept) && ast.op(q) == KW_NOEXCEPT && ast.first(q))
                bind_template_expression(ast.first(q),fs);
    auto ctor_initializers = child(body.source,Kind::CtorInitializer);
    if (!ctor_initializers) ctor_initializers = child(body.node,Kind::CtorInitializer);
    for (auto item = ast.first(ctor_initializers); item; item = ast.next(item)) {
        auto id = child(item,Kind::MemInitializerId);
        bind_template_expression(ast.detail(id),fs);
        bind_template_expression(ast.next(id),fs);
    }
    bind_template_statement(body.node,fs);
    check_jumps(body.node,true);
    template_bound_bodies.put(source,unsigned(FactState::Success));
    } catch (...) { template_bound_bodies.put(source,unsigned(FactState::Failure)); throw; }
}
} }
