#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
QueryId Analyzer::intern_query(TypeQuery q, const std::vector<QueryId>& children)
{
    std::uint64_t hash = 1469598103934665603ULL;
    auto add = [&](std::uint64_t x) { hash = (hash ^ x) * 1099511628211ULL; };
    add(unsigned(q.kind)); add(q.op); add(q.type); add(q.entity); add(q.name);
    add(q.context); add(q.arguments); add(q.value); add(q.null_pointer_constant);
    for (auto c : children) add(c);
    if (query_slots.empty() || type_queries.size()*2 >= query_slots.size()) {
        query_slots.assign(query_slots.empty() ? 32 : query_slots.size()*2,0);
        for (QueryId i = 1; i < type_queries.size(); ++i) {
            auto pos = query_hashes[i] & (query_slots.size()-1);
            while (query_slots[pos]) pos = (pos+1)&(query_slots.size()-1);
            query_slots[pos] = i;
        }
    }
    auto pos = hash & (query_slots.size()-1);
    while (auto id = query_slots[pos]) {
        auto p = type_queries[id];
        bool same = query_hashes[id] == hash && p.kind == q.kind && p.op == q.op && p.type == q.type &&
            p.entity == q.entity && p.name == q.name && p.context == q.context &&
            p.arguments == q.arguments && p.value == q.value && p.count == children.size() &&
            p.null_pointer_constant == q.null_pointer_constant;
        for (unsigned i = 0; same && i < p.count; ++i) same = query_edges[p.offset+i] == children[i];
        if (same) return id;
        pos = (pos+1)&(query_slots.size()-1);
    }
    q.offset = query_edges.size(); q.count = children.size();
    query_edges.insert(query_edges.end(),children.begin(),children.end());
    auto id = type_queries.size(); query_slots[pos] = id;
    type_queries.push_back(q); query_hashes.push_back(hash); query_facts.push_back(TypeQueryFact()); return id;
}
QueryId Analyzer::expression_query(NodeId n, ScopeId s, bool callee)
{
    auto& source_index = callee ? query_callee_sources : query_sources;
    if (auto old = source_index.get(key(s,n))) return old;
    TypeQuery q; std::vector<QueryId> children;
    auto node = ast[n]; auto first = node.first;
    switch (node.kind) {
    case Kind::IdExpression: {
        auto name = node.detail;
        if (callee && (fundamental_cast_type(node.op) || ast[name].kind == Kind::TypeId)) {
            q.kind = QueryKind::TypeValue; q.type = fundamental_cast_type(node.op);
            if (!q.type) q.type = type_id(name,s);
            break;
        }
        auto e = resolve(name,s);
        if (!e && (!callee || ast[name].first != ast[name].last || ast[name].op == OP_COLON2))
            throw std::runtime_error("unbound name in type query");
        auto entity = entities[e];
        if (e && (entity.kind == EntityKind::Alias || entity.kind == EntityKind::Type)) {
            if (!callee) throw std::runtime_error("type name used as value in type query");
            q.kind = QueryKind::TypeValue; q.type = entity.type;
        } else if (auto ordinal = signature_parameters.get(e)) {
            q.kind = QueryKind::Parameter; q.type = entity.type; q.value = ordinal-1;
        } else {
            q.kind = QueryKind::Name; q.entity = e;
            if (!function_binding(e)) q.type = entity.type;
            auto list = child(ast[name].last,Kind::TemplateArguments);
            std::vector<TypeId> arguments;
            for (auto a = ast[list].first; a; a = ast[a].next) {
                if (ast[a].kind != Kind::TypeId) throw std::runtime_error("type argument required in type query");
                arguments.push_back(type_id(a,s));
            }
            if (list) q.arguments = intern_arguments(arguments);
            if (ast[name].first == ast[name].last && ast[name].op != OP_COLON2) q.name = terminal(name);
        }
        break;
    }
    case Kind::Literal: case Kind::KeywordLiteral: {
        auto value = expression(n,s); q.type = value.type;
        auto constant = evaluate(n,s); q.value = constant.valid ? constant.bits : 0;
        q.null_pointer_constant = node.kind == Kind::Literal && constant.valid && !constant.bits && integral(q.type) &&
            ast.literals[node.literal].kind != LiteralKind::character;
        break;
    }
    case Kind::Parenthesized:
        q.kind = QueryKind::Parenthesized; children.push_back(expression_query(first,s,callee)); break;
    case Kind::Unary: case Kind::Binary: case Kind::Subscript:
        q.kind = node.kind == Kind::Unary ? QueryKind::Unary : QueryKind::Binary; q.op = node.op;
        if (node.kind == Kind::Subscript) q.op = OP_LSQUARE;
        q.name = operator_name(q.op); q.context = s;
        while (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block)
            q.context = scopes[q.context].parent;
        if (q.op != OP_LSQUARE && q.op != OP_ASS && q.op != OP_ARROW) {
            auto ordinary = lookup(s,q.name);
            if (function_binding(ordinary)) q.entity = ordinary;
        }
        for (auto c = first; c; c = ast[c].next) children.push_back(expression_query(c,s));
        break;
    case Kind::Call:
        q.kind = QueryKind::Call; q.context = s;
        while (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block)
            q.context = scopes[q.context].parent;
        children.push_back(expression_query(first,s,true));
        for (auto a = ast[ast[first].next].first; a; a = ast[a].next) children.push_back(expression_query(a,s));
        break;
    case Kind::Member:
        q.kind = QueryKind::Member; q.op = node.op;
        q.name = terminal(ast[ast[first].next].detail);
        children.push_back(expression_query(first,s)); break;
    case Kind::Sizeof: case Kind::TypeTrait:
        q.kind = QueryKind::Sizeof; q.op = node.op;
        if (ast[first].kind == Kind::TypeId) q.type = type_id(first,s);
        else children.push_back(expression_query(first,s));
        break;
    default: throw std::runtime_error("unsupported dependent type query operation");
    }
    auto id = intern_query(q,children); source_index.put(key(s,n),id); return id;
}
QueryId Analyzer::substitute_query(QueryId id, const Index& bindings, Index& cache)
{
    if (!query_fact(id).dependent) return id;
    auto key = (std::uint64_t(1)<<63)|id;
    if (auto old = cache.get(key)) return old;
    auto q = type_queries[id];
    if (q.type) q.type = substitute_type(q.type,bindings,cache);
    if (q.arguments) {
        auto pack = argument_packs[q.arguments]; std::vector<TypeId> args;
        for (unsigned j = 0; j < pack.count; ++j) args.push_back(substitute_type(argument_types[pack.offset+j],bindings,cache));
        q.arguments = intern_arguments(args);
    }
    std::vector<QueryId> children;
    for (unsigned i = 0; i < q.count; ++i) children.push_back(substitute_query(query_edges[q.offset+i],bindings,cache));
    auto result = intern_query(q,children); cache.put(key,result); return result;
}
TypeQueryFact Analyzer::query_fact(QueryId id)
{
    if (query_facts[id].state == FactState::Success) return query_facts[id];
    if (query_facts[id].state == FactState::Failure) throw std::runtime_error("failed type query");
    if (query_facts[id].state == FactState::Active) throw std::runtime_error("recursive type query");
    query_facts[id].state = FactState::Active;
    try {
    ++query_work;
    auto q = type_queries[id]; TypeQueryFact r;
    std::vector<TypeQueryFact> children;
    for (unsigned i = 0; i < q.count; ++i) {
        children.push_back(query_fact(query_edges[q.offset+i])); r.dependent |= children.back().dependent;
    }
    r.dependent |= q.type && dependent_type(q.type);
    if (q.arguments) {
        auto pack = argument_packs[q.arguments];
        for (unsigned i = 0; i < pack.count; ++i) r.dependent |= dependent_type(argument_types[pack.offset+i]);
    }
    auto& x = r.expression;
    if (!r.dependent) switch (q.kind) {
    case QueryKind::Value: x.type = q.type; x.null_pointer_constant = q.null_pointer_constant; break;
    case QueryKind::TypeValue: x.type = q.type; r.declared_type = q.type; break;
    case QueryKind::Parameter: case QueryKind::Name:
        x.type = value_type(q.type); r.declared_type = q.type;
        x.category = ValueCategory::Lvalue; x.entity = q.entity;
        if (q.entity && function_binding(q.entity)) x.form = ExpressionForm::Overload;
        if (q.entity && entities[q.entity].kind == EntityKind::Function && !entities[q.entity].template_info)
            r.declared_type = entities[q.entity].type;
        break;
    case QueryKind::Parenthesized: r = children[0]; r.declared_type = 0; break;
    case QueryKind::Member: {
        auto object = children[0].expression; auto type = object.type;
        if (q.op == OP_ARROW) { if (!pointer(type)) throw std::runtime_error("type query arrow needs pointer"); type = types[type].child; }
        if (!class_value(type)) throw std::runtime_error("type query member needs class");
        auto cls = types[type].entity; complete_class(cls);
        auto e = lookup(entities[cls].scope,q.name,Lookup::Ordinary,true);
        if (!e) throw std::runtime_error("type query member not found");
        x.entity = e;
        if (function_binding(e)) { x.form = ExpressionForm::Overload; x.category = ValueCategory::Lvalue; }
        else {
            r.declared_type = entities[e].type; x.type = value_type(r.declared_type);
            if (!entities[e].is_static && x.type == r.declared_type) x.type = types.qualify(x.type,types[type].cv & (entities[e].mutable_field ? 2 : 3));
            x.category = q.op == OP_ARROW || types[r.declared_type].kind == TypeKind::LRef || entities[e].is_static ? ValueCategory::Lvalue : object.category;
        }
        break;
    }
    case QueryKind::Unary: case QueryKind::Binary: r = query_operator(q,children); break;
    case QueryKind::Call: r = query_call(q,children); break;
    case QueryKind::Sizeof: x.type = types.fundamental(FT_UNSIGNED_LONG_INT); break;
    }
    r.state = FactState::Success; query_facts[id] = r; return r;
    } catch (...) { query_facts[id].state = FactState::Failure; throw; }
}
TypeId Analyzer::query_decltype(QueryId id, bool direct)
{
    auto value = query_fact(id);
    if (value.dependent) return types.decltype_type(id,direct);
    if (direct && value.declared_type) return value.declared_type;
    auto x = value.expression;
    if (!x.type) throw std::runtime_error("unresolved overload in decltype");
    return x.category == ValueCategory::Prvalue ? x.type :
        types.compound(x.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef,x.type);
}
TypeId Analyzer::dependent_decltype(NodeId n, ScopeId s)
{
    return query_decltype(expression_query(n,s),ast[n].kind == Kind::IdExpression || ast[n].kind == Kind::Member);
}
} }
