#include "semantic/analyzer.h"
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
    bool full = !last || last == ast[n].last;
    if (!last) last = ast[n].last;
    auto source = ast.nodes.occurrences[n].source;
    bool pattern = full && !ast.nodes.occurrences[n].context;
    if (pattern) if (auto id = template_binding_index.get(source)) return template_bindings[id];
    ++template_binding_work;
    TemplateBinding r; r.scope = s;
    auto owner = ast[n].op == OP_COLON2 ? global : s;
    bool qualified = ast[n].op == OP_COLON2;
    for (auto p = ast[n].first; p; p = ast[p].next) {
        if (ast[p].detail && ast[ast[p].detail].kind == Kind::Decltype) {
            r.dependent |= bind_template_expression(ast[ast[p].detail].first,s);
            if (r.dependent) break;
            auto t = expression_type(ast[ast[p].detail].first,s,true);
            owner = types[t].kind == TypeKind::Named ? entities[types[t].entity].scope : 0;
            qualified = true; continue;
        }
        auto e = lookup(owner,full && p == last ? terminal(n) : ast[p].text,p == last ? Lookup::Ordinary : Lookup::Qualifier,qualified);
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
            for (auto a = ast[args].first; a; a = ast[a].next) {
                // The parser retains unresolved template arguments as value
                // syntax when the owning class's aliases are not yet visible.
                if (ast[a].kind == Kind::IdExpression) {
                    auto argument = bind_template_name(ast[a].detail,s);
                    if (argument.dependent || (argument.entity && (entities[argument.entity].kind == EntityKind::Type || entities[argument.entity].kind == EntityKind::Alias))) {
                        r.dependent |= argument.dependent; continue;
                    }
                }
                r.dependent |= bind_template_expression(a,s);
            }
            if (r.dependent && entities[e].template_info)
                for (auto a = ast[args].first; a; a = ast[a].next) template_argument_node(a,s);
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
                for (auto next = ast[p].next; next; previous = next, next = ast[next].next) {
                    if (child(next,Kind::TemplateArguments) && !(ast[next].flags & 1)) {
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
                        auto member = current ? lookup(current,ast[missing].text,Lookup::Ordinary,true) : 0;
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
    bool dependent = bind_template_expression_impl(n,s,callee);
    if (n && !ast.nodes.occurrences[n].context) {
        auto kind = ast[n].kind;
        if (dependent && template_body_values) {
            template_value_dependence.put(ast.nodes.occurrences[n].source,1);
            if (kind == Kind::Sizeof || kind == Kind::SizeofPack || kind == Kind::TypeTrait) bind_template_size(n,s);
        }
        // Value dependence alone does not change scalar operand types or the
        // selected built-in conversions. Each checker still requires complete
        // fixed operand facts before publishing its source semantic decision.
        bool scalar = kind == Kind::IdExpression || kind == Kind::Binary || kind == Kind::Assignment || kind == Kind::Conditional ||
            kind == Kind::Unary || kind == Kind::Postfix || kind == Kind::Parenthesized || kind == Kind::Subscript ||
            kind == Kind::Call || kind == Kind::Member || kind == Kind::Cast;
        if (!dependent || (template_body_values && scalar)) check_fixed_expression(n,s);
    }
    return dependent;
}
bool Analyzer::bind_template_expression_impl(NodeId n, ScopeId s, bool callee)
{
    if (!n) return false;
    ++template_binding_work;
    auto node = ast[n];
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
        if (callee && ast[name].kind == Kind::TypeId) return bind_template_expression(name,s);
        auto binding = bind_template_name(name,s);
        auto e = binding.entity;
        if (!binding.dependent && check_template_field(n,s,e)) return false;
        if (!binding.dependent && !e && !callee) throw std::runtime_error("unbound template value name");
        if (e && !callee && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias ||
            entities[e].kind == EntityKind::Namespace || entities[e].kind == EntityKind::NamespaceAlias))
            throw std::runtime_error("template expression requires a value name");
        return binding.dependent;
    }
    if (node.kind == Kind::Call) {
        bool dependent = false;
        auto fn = node.first;
        for (auto a = ast[ast[fn].next].first; a; a = ast[a].next) dependent |= bind_template_expression(a,s);
        dependent |= bind_template_expression(fn,s,true);
        if (ast[fn].kind == Kind::IdExpression && !fundamental_cast_type(ast[fn].op) && ast[ast[fn].detail].kind != Kind::TypeId) {
            auto name = ast[fn].detail; auto binding = bind_template_name(name,s);
            auto id = terminal(name);
            if (!binding.entity && !binding.dependent &&
                (!dependent || ast[name].first != ast[name].last || ast[name].op == OP_COLON2) &&
                id != constant_builtin && id != abort_builtin && id != expect_builtin)
                query_fact(expression_query(n,s));
        }
        return dependent;
    }
    if (node.kind == Kind::Member) {
        bool dependent = bind_template_expression(node.first,s);
        auto name = ast[ast[node.first].next].detail;
        for (auto p = ast[name].first; p; p = ast[p].next)
            if (auto args = child(p,Kind::TemplateArguments)) dependent |= bind_template_expression(args,s);
        auto receiver = node.first;
        while (ast[receiver].kind == Kind::Parenthesized) receiver = ast[receiver].first;
        bool current = node.op == OP_ARROW && ast[receiver].kind == Kind::KeywordLiteral && ast[receiver].op == KW_THIS;
        if (node.op == OP_DOT && ast[receiver].kind == Kind::Unary && ast[receiver].op == OP_STAR) {
            receiver = ast[receiver].first;
            while (ast[receiver].kind == Kind::Parenthesized) receiver = ast[receiver].first;
            current = ast[receiver].kind == Kind::KeywordLiteral && ast[receiver].op == KW_THIS;
        }
        auto object = template_object_context(s);
        if (dependent && !ast.nodes.occurrences[n].context) {
            auto part = ast[name].last;
            if (child(part,Kind::TemplateArguments) && !(ast[part].flags & 1)) {
                TypeId type = 0;
                if (part != ast[name].first) {
                    auto previous = ast[name].first;
                    while (ast[previous].next != part) previous = ast[previous].next;
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
                    auto member = owner ? lookup(owner,ast[part].text,Lookup::Ordinary,true) : 0;
                    bool known = false;
                    for (auto candidate : candidates(member)) known |= entities[candidate].template_info != 0;
                    if (!known) throw std::runtime_error("dependent member template requires template");
                }
            }
        }
        if (current && object.owner && ast[name].first == ast[name].last && !child(ast[name].last,Kind::TemplateArguments)) {
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
    if (node.kind == Kind::Sizeof || node.kind == Kind::TypeTrait) {
        ++unevaluated_depth; bool dependent = false;
        try { for (auto c = node.first; c; c = ast[c].next) dependent |= bind_template_expression(c,s); }
        catch (...) { --unevaluated_depth; throw; }
        --unevaluated_depth; return dependent;
    }
    if (node.kind == Kind::KeywordLiteral && node.op == KW_THIS) {
        auto object = template_object_context(s);
        if (object.owner && !object.available) throw std::runtime_error("this in static template member");
    }
    bool dependent = node.kind == Kind::KeywordLiteral && node.op == KW_THIS;
    if (node.detail) dependent |= bind_template_expression(node.detail,s);
    for (auto c = node.first; c; c = ast[c].next) dependent |= bind_template_expression(c,s);
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
    template_pattern_scopes.put(fs,1);
    auto d = body.declarator; NodeId params = 0;
    while (d) {
        if (auto p = child(d,Kind::Parameters)) params = p;
        auto nested = child(d,Kind::NestedDeclarator); d = nested ? ast[nested].first : 0;
    }
    bind_template_object_context(fs,params);
    unsigned ordinal = 0;
    for (auto p = ast[params].first; p; p = ast[p].next) {
        if (ast[p].kind != Kind::Parameter) continue;
        auto specs = ast[p].first, decl = ast[specs].next;
        bool dependent = bind_template_expression(specs,fs) | bind_template_expression(decl,fs);
        auto declared_type = facts[p].type;
        if (!declared_type) declared_type = bind_template_type(specs,decl,fs);
        auto e = pattern_declaration(EntityKind::Parameter,fs,terminal(decl_name(decl)),p,dependent);
        entities[e].parameter_pack = child(decl,Kind::ParameterPack) != 0;
        // Type queries can refer to an earlier parameter while the signature
        // is being instantiated, before runtime parameter objects exist.
        signature_parameters.put(e,++ordinal);
        if (declared_type) {
            facts.edit(p).type = declared_type;
            entities[e].type = parameter_body_type(declared_type);
        }
    }
    struct ValueBinding {
        bool& mode; bool prior;
        ValueBinding(bool& value) : mode(value), prior(value) { mode = true; }
        ~ValueBinding() { mode = prior; }
    } value_binding(template_body_values);
    auto ctor_initializers = child(body.source,Kind::CtorInitializer);
    for (auto item = ast[ctor_initializers].first; item; item = ast[item].next) {
        auto id = child(item,Kind::MemInitializerId);
        bind_template_expression(ast[id].detail,fs);
        bind_template_expression(ast[id].next,fs);
    }
    bind_template_statement(body.node,fs);
    check_jumps(body.node,true);
    template_bound_bodies.put(source,unsigned(FactState::Success));
    } catch (...) { template_bound_bodies.put(source,unsigned(FactState::Failure)); throw; }
}
} }
