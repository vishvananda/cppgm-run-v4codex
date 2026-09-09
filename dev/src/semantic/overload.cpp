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
EntityId Analyzer::declare_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type, bool constructor)
{
    Type t = types[type];
    std::vector<TypeId> params(types.parameters.begin() + t.offset, types.parameters.begin() + t.offset + t.count);
    TypeId shape = types.function(types.fundamental(FT_VOID), params, t.variadic, t.cv);
    EntityId family = function_families.get(key(owner, name));
    EntityId e = family ? function_signatures.get(key(family, shape)) : 0;
    if (e) {
        if (entities[e].type != type) throw std::runtime_error("conflicting function return type");
        if (!constructor) bind(owner, name, e);
        return e;
    }
    e = make_entity(EntityKind::Function, owner, name, source);
    entities[e].type = type;
    if (calls && scopes[owner].kind == ScopeKind::Template) template_facts(e);
    if (!family) { family = e; function_families.put(key(owner, name), family); }
    function_signatures.put(key(family, shape), e);
    if (!constructor) bind(owner, name, e);
    return e;
}
bool Analyzer::better(const Conversion* a, const Conversion* b, std::size_t count)
{
    bool strict = false;
    for (std::size_t i = 0; i < count; ++i) {
        if (a[i].rank > b[i].rank) return false;
        if (a[i].rank < b[i].rank) { strict = true; continue; }
        TypeId at = a[i].target, bt = b[i].target;
        if (a[i].reference) at = types[at].child;
        if (b[i].reference) bt = types[bt].child;
        if (!a[i].reference && !b[i].reference && pointer(at) && pointer(bt)) {
            at = types[at].child; bt = types[bt].child;
        }
        // Compare the complete qualification signatures, not an OR of cv bits
        // that loses which indirection level acquired the qualifier.
        unsigned added = 0;
        if (at != bt && (a[i].derived || b[i].derived)) {
            if (derived_from(bt, at) || (fundamental(at, FT_VOID) && types[bt].kind == TypeKind::Named)) return false;
            if (derived_from(at, bt) || (fundamental(bt, FT_VOID) && types[at].kind == TypeKind::Named)) { strict = true; continue; }
        }
        if (at != bt && similar_type(at, bt)) {
            bool ab = qualification(at, bt, added), ba = qualification(bt, at, added);
            if (ba && !ab) return false;
            if (ab && !ba) { strict = true; continue; }
            if (!ab && !ba) return false;
        }
        if (a[i].reference && b[i].reference) {
            if (a[i].preference > b[i].preference) return false;
            if (a[i].preference < b[i].preference) strict = true;
        }
    }
    return strict;
}
Conversion Analyzer::ellipsis_conversion(NodeId n)
{
    Conversion c; c.rank = 4;
    c.target = promote(decay(expressions[n].type));
    if (fundamental(c.target, FT_FLOAT)) c.target = types.fundamental(FT_DOUBLE);
    if (fundamental(c.target, FT_NULLPTR_T)) c.target = types.compound(TypeKind::Pointer, types.fundamental(FT_VOID));
    return c;
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
        bool builtin_name = ast[detail].kind == Kind::Name && ast[detail].first == ast[detail].last;
        if (!e && builtin_name && name == constant_builtin) {
            if (args.size() != 1) throw std::runtime_error("constant query arity");
            result.type = types.fundamental(FT_INT); result.form = ExpressionForm::ConstantQuery;
            Constant v(result.type, evaluate(args[0], s).valid);
            facts[n].value = constants.size(); constants.push_back(v);
            return result;
        }
        if (!e && builtin_name && name && ids.spelling(name).equals("__builtin_unreachable")) {
            if (!args.empty()) throw std::runtime_error("unreachable takes no arguments");
            result.type = types.fundamental(FT_VOID); result.form = ExpressionForm::Unreachable;
            return result;
        }
        if (!e && builtin_name && name == abort_builtin) {
            if (!args.empty()) throw std::runtime_error("abort takes no arguments");
            result.type = types.fundamental(FT_VOID); result.form = ExpressionForm::Abort;
            return result;
        }
        if (!e && builtin_name && name) {
            TextView spelling = ids.spelling(name);
            bool copy = spelling.equals("__builtin_memcpy");
            bool move = spelling.equals("__builtin_memmove");
            if (copy || move) {
                TypeId v = types.fundamental(FT_VOID), ptr = types.compound(TypeKind::Pointer, v);
                TypeId ft = types.function(ptr, {ptr, types.compound(TypeKind::Pointer, types.qualify(v, 1)), types.fundamental(FT_UNSIGNED_LONG_INT)}, false);
                e = declare_function(global, name, 0, ft);
                entities[e].builtin = copy ? Entity::Memcpy : Entity::Memmove;
            }
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
            if (types[cast_type].kind == TypeKind::Named && entities[types[cast_type].entity].class_info) {
                EntityId ctor = choose_constructor(cast_type, args, &result, s);
                members[entities[ctor].member_info].complete_entry = true;
                facts[n].entity = ctor; facts[n].type = cast_type;
                result.type = cast_type; result.form = ExpressionForm::Construction;
                EntityId temporary = make_entity(EntityKind::Variable, make_scope(ScopeKind::Block, s), 0, n);
                entities[temporary].type = cast_type; register_destruction(temporary);
                record_object(result, 0, 0, 0); object_uses[result.object_use].temporary = temporary;
                object_uses[result.object_use].value_initialize = args.empty() && members[entities[ctor].member_info].synthetic;
                return result;
            }
            if (args.size() > 1) throw std::runtime_error("scalar cast arity");
            return cast_expression(n, s, cast_type, args.empty() ? 0 : args[0]);
        }
    }
    Expression fn;
    bool unqualified = ast[callee].kind == Kind::IdExpression && ast[ast[callee].detail].kind == Kind::Name &&
        ast[ast[callee].detail].first == ast[ast[callee].detail].last && ast[ast[callee].detail].op != OP_COLON2;
    bool adl = unqualified;
    EntityId ordinary = unqualified ? resolve(ast[callee].detail, s) : 0;
    if (ordinary) {
        if (!function_binding(ordinary)) adl = false;
        else for (EntityId candidate : candidates(ordinary)) {
            ScopeKind owner = scopes[entities[candidate].owner].kind;
            if (owner == ScopeKind::Class || owner == ScopeKind::Block || owner == ScopeKind::Function) adl = false;
        }
    }
    EntityId associated = adl ? associated_lookup(terminal(ast[callee].detail), args) : 0;
    if (associated) {
        EntityId selected = merge_lookup(ordinary, associated);
        fn.entity = selected; fn.form = ExpressionForm::Overload; fn.category = ValueCategory::Lvalue;
        expressions[callee] = fn; expressions[callee].ready = true; expressions[callee].evaluated = !unevaluated_depth;
        facts[callee].entity = selected; facts[callee].scope = s;
    } else fn = expression(callee, s);
    if (fn.type && types[fn.type].kind == TypeKind::Named && entities[types[fn.type].entity].class_info) {
        std::vector<NodeId> operands(1, callee); operands.insert(operands.end(), args.begin(), args.end());
        if (operator_expression(n, s, OP_LPAREN, operands, result)) return result;
    }
    if (fn.form == ExpressionForm::PseudoDestructor) {
        if (!args.empty()) throw std::runtime_error("pseudo-destructor takes no arguments");
        result.type = types.fundamental(FT_VOID); result.form = ExpressionForm::PseudoDestructor; return result;
    }
    TypeId ft = 0;
    NodeId designator = callee;
    while (ast[designator].kind == Kind::Parenthesized) designator = ast[designator].first;
    bool direct_name = ast[designator].kind == Kind::IdExpression || ast[designator].kind == Kind::Member;
    TypeId object_type = 0;
    NodeId object_node = 0;
    if (ast[designator].kind == Kind::Member) {
        object_node = ast[designator].first;
        object_type = expressions[object_node].type;
        if (ast[designator].op == OP_ARROW) object_type = types[object_type].child;
    } else {
        TypeId implicit = implicit_object_type(s);
        if (implicit) object_type = types[implicit].child;
    }
    if (fn.form == ExpressionForm::Overload && !direct_name) throw std::runtime_error("unresolved indirect callee");
    if (direct_name && (fn.form == ExpressionForm::Overload || (fn.entity && entities[fn.entity].kind == EntityKind::Function))) {
        struct Candidate { EntityId entity; std::size_t offset; };
        std::vector<Candidate> viable;
        bool object_ranking = object_type != 0;
        std::vector<Conversion> sequences;
        for (EntityId e : candidates(fn.entity)) {
            ++candidate_work;
            if (entities[e].template_info) e = deduce_function(e, args);
            if (!e) continue;
            Type function = types[entities[e].type];
            if ((!function.variadic && args.size() > function.count) ||
                (args.size() < function.count && (!entities[e].defaults ||
                 !default_arguments[entities[e].defaults + args.size()]))) continue;
            std::size_t begin = sequences.size();
            bool valid = true;
            if (object_ranking) {
                Conversion c; c.rank = 0; c.target = object_type;
                if (entities[e].member_info && !entities[e].is_static) {
                    TypeId wanted = types[types.parameters[types[call_type(e)].offset]].child;
                    c.target = types.compound(TypeKind::LRef, wanted); c.reference = true;
                    valid = (destructor_member(e) || !(types[object_type].cv & ~types[wanted].cv)) &&
                        (types.unqualified(object_type) == types.unqualified(wanted) || derived_from(object_type, wanted));
                }
                sequences.push_back(c);
            } else if (entities[e].member_info && !entities[e].is_static) valid = false;
            for (std::size_t i = 0; valid && i < args.size(); ++i) {
                Conversion c;
                if (i < function.count) c = conversion(args[i], types.parameters[function.offset + i]);
                else c = ellipsis_conversion(args[i]);
                valid = c.valid(); sequences.push_back(c);
            }
            if (valid) viable.push_back({e, begin});
            else sequences.resize(begin);
        }
        if (viable.empty()) throw std::runtime_error("no viable function");
        std::size_t best = 0;
        for (std::size_t i = 1; i < viable.size(); ++i)
            if (better(sequences.data() + viable[i].offset, sequences.data() + viable[best].offset, args.size() + object_ranking)) best = i;
        for (std::size_t i = 0; i < viable.size(); ++i)
            if (i != best && !better(sequences.data() + viable[best].offset, sequences.data() + viable[i].offset, args.size() + object_ranking))
                throw std::runtime_error("ambiguous overload");
        EntityId selected = viable[best].entity;
        facts[n].entity = selected;
        if (entities[selected].member_info && !entities[selected].is_static)
        {
            record_object(result, object_node, types.parameters[types[call_type(selected)].offset],
                base_steps(object_type, scopes[entities[selected].owner].entity));
        }
        ft = entities[selected].type;
        if (object_node && !result.object_use) record_object(result, object_node, 0, 0);
        Type selected_type = types[ft];
        std::vector<Conversion> chosen(sequences.begin() + viable[best].offset + object_ranking,
            sequences.begin() + viable[best].offset + object_ranking + args.size());
        for (std::size_t i = args.size(); i < selected_type.count; ++i) {
            NodeId a = default_arguments[entities[selected].defaults + i];
            args.push_back(a);
            chosen.push_back(conversion(a, types.parameters[selected_type.offset+i]));
        }
        result.conversions = conversions.size(); result.count = args.size();
        for (std::size_t i = 0; i < args.size(); ++i) {
            Conversion c = chosen[i];
            expressions[args[i]].incoming = conversions.size();
            conversions.push_back(c);
            apply_conversion(args[i], c);
        }
        select_function(callee, selected);
    } else {
        ft = fn.type;
        if (pointer(ft)) ft = types[ft].child;
        if (types[ft].kind != TypeKind::Function) throw std::runtime_error("called object is not a function");
        Type f = types[ft];
        if (args.size() < f.count || (!f.variadic && args.size() != f.count)) throw std::runtime_error("indirect call arity");
        require_conversion(callee, decay(fn.type));
        result.conversions = conversions.size(); result.count = args.size();
        for (std::size_t i = 0; i < args.size(); ++i) {
            Conversion c;
            if (i < f.count) c = conversion(args[i], types.parameters[f.offset + i]);
            else c = ellipsis_conversion(args[i]);
            if (!c.valid()) throw std::runtime_error("indirect argument conversion");
            expressions[args[i]].incoming = conversions.size();
            conversions.push_back(c);
            apply_conversion(args[i], c);
        }
    }
    result.arguments = call_arguments.size(); result.argument_count = args.size();
    call_arguments.insert(call_arguments.end(), args.begin(), args.end());
    TypeId returned = types[ft].child;
    facts[n].type = returned;
    result.type = value_type(returned);
    result.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    return result;
}
} }
