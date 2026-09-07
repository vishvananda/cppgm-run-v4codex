#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::function_binding(EntityId e) const
{
    return e && (entities[e].kind == EntityKind::Function || entities[e].kind == EntityKind::Overload);
}
std::vector<EntityId> Analyzer::candidates(EntityId e)
{
    std::vector<EntityId> work(1, e), result;
    Index seen;
    while (!work.empty()) {
        EntityId current = work.back(); work.pop_back();
        if (!current || seen.get(current)) continue;
        seen.put(current, 1);
        if (entities[current].kind == EntityKind::Overload) {
            work.push_back(entities[current].second); work.push_back(entities[current].first);
        } else result.push_back(current);
    }
    return result;
}
EntityId Analyzer::declare_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type)
{
    Type t = types[type];
    std::vector<TypeId> params(types.parameters.begin() + t.offset, types.parameters.begin() + t.offset + t.count);
    TypeId shape = types.function(types.fundamental(FT_VOID), params, t.variadic);
    EntityId family = function_families.get(key(owner, name));
    EntityId e = family ? function_signatures.get(key(family, shape)) : 0;
    if (e) {
        if (entities[e].type != type) throw std::runtime_error("conflicting function return type");
        return e;
    }
    e = make_entity(EntityKind::Function, owner, name, source);
    entities[e].type = type;
    if (!family) { family = e; function_families.put(key(owner, name), family); }
    function_signatures.put(key(family, shape), e);
    bind(owner, name, e);
    return e;
}
namespace {
bool better(const Conversion* a, const Conversion* b, std::size_t count)
{
    bool strict = false;
    for (std::size_t i = 0; i < count; ++i) {
        if (a[i].rank > b[i].rank) return false;
        if (a[i].rank < b[i].rank) { strict = true; continue; }
        if (a[i].qualification & ~b[i].qualification) return false;
        if (b[i].qualification & ~a[i].qualification) { strict = true; continue; }
        if (a[i].reference && b[i].reference) {
            if (a[i].preference > b[i].preference) return false;
            if (a[i].preference < b[i].preference) strict = true;
        }
    }
    return strict;
}
}
Expression Analyzer::call_expression(NodeId n, ScopeId s)
{
    NodeId callee = ast[n].first, args_node = ast[callee].next;
    std::vector<NodeId> args;
    for (NodeId a = ast[args_node].first; a; a = ast[a].next) { expression(a, s); args.push_back(a); }
    Expression result;
    if (ast[callee].kind == Kind::IdExpression) {
        IdentifierId name = terminal(ast[callee].detail);
        NodeId detail = ast[callee].detail;
        EntityId e = detail && ast[detail].kind == Kind::Name && !ast[ast[detail].first].detail ? resolve(detail, s) : 0;
        if (!e && name == constant_builtin) {
            if (args.size() != 1) throw std::runtime_error("constant query arity");
            result.type = types.fundamental(FT_INT); result.form = ExpressionForm::ConstantQuery;
            Constant v(result.type, evaluate(args[0], s).valid);
            facts[n].value = constants.size(); constants.push_back(v);
            return result;
        }
        if (!e && name == abort_builtin) {
            if (!args.empty()) throw std::runtime_error("abort takes no arguments");
            result.type = types.fundamental(FT_VOID); result.form = ExpressionForm::Abort;
            return result;
        }
        TypeId cast_type = 0;
        if (ast[detail].kind == Kind::TypeId) cast_type = type_id(detail, s);
        else if (detail && ast[ast[detail].first].detail && ast[ast[ast[detail].first].detail].kind == Kind::Decltype)
            cast_type = expression_type(ast[ast[ast[detail].first].detail].first, s, true);
        if (e && (entities[e].kind == EntityKind::Alias || entities[e].kind == EntityKind::Type)) cast_type = entities[e].type;
        if (ast[callee].op != TOK_INVALID) {
            switch (ast[callee].op) {
            case KW_INT: cast_type = types.fundamental(FT_INT); break;
            case KW_BOOL: cast_type = types.fundamental(FT_BOOL); break;
            case KW_CHAR: cast_type = types.fundamental(FT_CHAR); break;
            case KW_LONG: cast_type = types.fundamental(FT_LONG_INT); break;
            case KW_FLOAT: cast_type = types.fundamental(FT_FLOAT); break;
            case KW_DOUBLE: cast_type = types.fundamental(FT_DOUBLE); break;
            case KW_VOID: cast_type = types.fundamental(FT_VOID); break;
            default: break;
            }
        }
        if (cast_type) {
            if (args.size() > 1) throw std::runtime_error("scalar cast arity");
            return cast_expression(n, s, cast_type, args.empty() ? 0 : args[0]);
        }
    }
    Expression fn = expression(callee, s);
    TypeId ft = 0;
    if (fn.form == ExpressionForm::Overload || (fn.entity && entities[fn.entity].kind == EntityKind::Function)) {
        struct Candidate { EntityId entity; std::size_t offset; };
        std::vector<Candidate> viable;
        std::vector<Conversion> sequences;
        for (EntityId e : candidates(fn.entity)) {
            ++candidate_work;
            Type function = types[entities[e].type];
            if (args.size() < function.count || (!function.variadic && args.size() != function.count)) continue;
            std::size_t begin = sequences.size();
            bool valid = true;
            for (std::size_t i = 0; valid && i < args.size(); ++i) {
                Conversion c;
                if (i < function.count) c = conversion(args[i], types.parameters[function.offset + i]);
                else { c.rank = 4; c.target = decay(expressions[args[i]].type); }
                valid = c.valid(); sequences.push_back(c);
            }
            if (valid) viable.push_back({e, begin});
            else sequences.resize(begin);
        }
        if (viable.empty()) throw std::runtime_error("no viable function");
        std::size_t best = 0;
        for (std::size_t i = 1; i < viable.size(); ++i)
            if (better(sequences.data() + viable[i].offset, sequences.data() + viable[best].offset, args.size())) best = i;
        for (std::size_t i = 0; i < viable.size(); ++i)
            if (i != best && !better(sequences.data() + viable[best].offset, sequences.data() + viable[i].offset, args.size()))
                throw std::runtime_error("ambiguous overload");
        result.entity = viable[best].entity;
        ft = entities[result.entity].type;
        result.conversions = conversions.size(); result.count = args.size();
        for (std::size_t i = 0; i < args.size(); ++i) {
            Conversion c = sequences[viable[best].offset + i];
            conversions.push_back(c);
            apply_conversion(args[i], c);
        }
        select_function(callee, result.entity);
    } else {
        ft = fn.type;
        if (pointer(ft)) ft = types[ft].child;
        if (types[ft].kind != TypeKind::Function) throw std::runtime_error("called object is not a function");
        Type f = types[ft];
        if (args.size() < f.count || (!f.variadic && args.size() != f.count)) throw std::runtime_error("indirect call arity");
        result.conversions = conversions.size(); result.count = args.size();
        for (std::size_t i = 0; i < args.size(); ++i) {
            Conversion c;
            if (i < f.count) c = conversion(args[i], types.parameters[f.offset + i]);
            else { c.rank = 4; c.target = decay(expressions[args[i]].type); }
            if (!c.valid()) throw std::runtime_error("indirect argument conversion");
            conversions.push_back(c);
            apply_conversion(args[i], c);
        }
    }
    TypeId returned = types[ft].child;
    facts[n].type = returned;
    result.type = value_type(returned);
    result.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    return result;
}
} }
