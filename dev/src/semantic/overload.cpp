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
EntityId Analyzer::declare_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type, bool constructor, TypeId conversion)
{
    if (definitions && active_template_scope &&
        (owner == active_template_scope || scopes[owner].kind == ScopeKind::Namespace ||
         (source && scopes[owner].kind == ScopeKind::Class)))
        return declare_template_function(owner,name,source,type,constructor,conversion != 0);
    // A block-scope function declaration has namespace linkage identity but
    // introduces its name only into the declaring block ([basic.link]). The
    // namespace signature index can later merge its namespace redeclaration.
    auto binding_scope = owner;
    while (scopes[owner].kind != ScopeKind::Namespace && scopes[owner].kind != ScopeKind::Class &&
        scopes[owner].kind != ScopeKind::Template) owner = scopes[owner].parent;
    Type t = types[type];
    std::vector<TypeId> params(types.parameters.begin() + t.offset, types.parameters.begin() + t.offset + t.count);
    TypeId shape = types.function(types.fundamental(FT_VOID), params, t.variadic, t.cv, t.ref);
    EntityId family = conversion ? conversion_families.get(key(owner,conversion)) : function_families.get(key(owner, name));
    TypeId ref_shape = 0;
    unsigned ref_mode = t.ref == RefQualifier::None ? 1 : 2;
    if (calls && scopes[owner].kind == ScopeKind::Class) {
        ref_shape = types.function(types.fundamental(FT_VOID), params, t.variadic);
        unsigned prior = family ? function_ref_modes.get(key(family, ref_shape)) : 0;
        if (prior && prior != ref_mode) throw std::runtime_error("mixed qualified and unqualified member overloads");
    }
    EntityId e = family ? function_signatures.get(key(family, shape)) : 0;
    if (e) {
        if (entities[e].type != type) throw std::runtime_error("conflicting function return type");
        if (!constructor && !conversion) bind(binding_scope, name, e);
        return e;
    }
    e = make_entity(EntityKind::Function, owner, name, source);
    entities[e].type = type;
    if (c_linkage && !constructor && !conversion && !t.variadic && t.count == 1 &&
        scopes[owner].kind == ScopeKind::Namespace) {
        auto ptr = types.compound(TypeKind::Pointer,types.fundamental(FT_VOID));
        if (ids.spelling(name).equals("malloc") && t.child == ptr && params[0] == types.fundamental(FT_UNSIGNED_LONG_INT))
            entities[e].builtin = Entity::Malloc;
        if (ids.spelling(name).equals("free") && t.child == types.fundamental(FT_VOID) && params[0] == ptr)
            entities[e].builtin = Entity::Free;
    }
    if (calls && scopes[owner].kind == ScopeKind::Template) template_facts(e);
    if (!family) { family = e; (conversion ? conversion_families : function_families).put(key(owner, conversion ? conversion : name), family); }
    if (ref_shape) function_ref_modes.put(key(family, ref_shape), ref_mode);
    function_signatures.put(key(family, shape), e);
    if (!constructor && !conversion) bind(binding_scope, name, e);
    return e;
}
bool Analyzer::better(const Conversion* a, const Conversion* b, std::size_t count)
{
    bool strict = false;
    for (std::size_t i = 0; i < count; ++i) {
        auto list = [&](const Conversion& c) {
            auto id = c.kind == Conversion::Kind::List ? list_objects[c.materialization].plan :
                c.kind == Conversion::Kind::ListPlan || c.kind == Conversion::Kind::QueryList ? c.materialization : 0;
            return list_plans[id].backing_element != 0;
        };
        bool alist = list(a[i]), blist = list(b[i]);
        if (alist != blist) { if (!alist) return false; strict = true; continue; }
        if (a[i].rank > b[i].rank) return false;
        if (a[i].rank < b[i].rank) { strict = true; continue; }
        if (a[i].rank == 5) {
            bool al = a[i].kind == Conversion::Kind::ListPlan || a[i].kind == Conversion::Kind::List || a[i].kind == Conversion::Kind::QueryList;
            bool bl = b[i].kind == Conversion::Kind::ListPlan || b[i].kind == Conversion::Kind::List || b[i].kind == Conversion::Kind::QueryList;
            if (al && bl && types.unqualified(value_type(a[i].target)) == types.unqualified(value_type(b[i].target)) &&
                a[i].reference && b[i].reference) {
                if (a[i].preference > b[i].preference) return false;
                strict |= a[i].preference < b[i].preference;
            }
            if (a[i].kind == Conversion::Kind::Construction && b[i].kind == Conversion::Kind::Construction &&
                a[i].function == b[i].function && a[i].reference && b[i].reference) {
                if (a[i].preference > b[i].preference) return false;
                strict |= a[i].preference < b[i].preference;
            }
            if (a[i].kind == Conversion::Kind::User && b[i].kind == Conversion::Kind::User && a[i].function == b[i].function) {
                auto x = user_conversions[a[i].materialization].result, y = user_conversions[b[i].materialization].result;
                if (better(&y,&x,1)) return false;
                strict |= better(&x,&y,1);
            }
            continue;
        }
        if (a[i].reference && b[i].reference && a[i].preference != b[i].preference) {
            if (a[i].preference > b[i].preference) return false;
            strict = true; continue;
        }
        TypeId at = a[i].target, bt = b[i].target;
        if (a[i].reference) at = types[at].child;
        if (b[i].reference) bt = types[bt].child;
        if (!a[i].reference && !b[i].reference && pointer(at) && pointer(bt)) {
            at = types[at].child; bt = types[bt].child;
        }
        // Compare the complete qualification signatures, not an OR of cv bits
        // that loses which indirection level acquired the qualifier.
        unsigned added = 0;
        // A class-value transfer has a standard derived-to-base rank even
        // though its construction record owns the actual subobject binding.
        bool transfer = a[i].rank == 2 &&
            (a[i].kind == Conversion::Kind::Construction || b[i].kind == Conversion::Kind::Construction);
        if (at != bt && (a[i].derived || b[i].derived || transfer)) {
            // [over.ics.rank]/4: A::* -> B::* beats A::* -> C::* when
            // C derives from B. Member conversions run opposite to pointers.
            if (types[at].kind == TypeKind::MemberPointer && types[bt].kind == TypeKind::MemberPointer) {
                auto ac = types[at].member_owner(), bc = types[bt].member_owner();
                if (derived_from(ac,bc)) return false;
                if (derived_from(bc,ac)) { strict = true; continue; }
            }
            if (derived_from(bt, at) || (fundamental(at, FT_VOID) && types[bt].kind == TypeKind::Named)) return false;
            if (derived_from(at, bt) || (fundamental(bt, FT_VOID) && types[at].kind == TypeKind::Named)) { strict = true; continue; }
        }
        if (a[i].reference == b[i].reference && at != bt && similar_type(at, bt)) {
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
    return ellipsis_conversion_value(expressions[n]);
}
Conversion Analyzer::ellipsis_conversion_value(Expression source)
{
    Conversion c; if (!source.type) return c; c.rank = 6;
    c.target = promote(decay(source.type));
    if (class_value(c.target)) {
        if (source.category == ValueCategory::Prvalue) {
            // A prvalue already owns its result object. [expr.call]/7 applies
            // lvalue-to-rvalue conversion only to glvalues, so this boundary
            // must not require an additional accessible copy/move constructor.
            c.ellipsis_object = true; return c;
        }
        // Ellipsis still passes a value: select its transfer in semantics,
        // retaining the source-language type for queries and constant evaluation.
        // LowIR's scalar variadic lane carries its private object's address.
        c = transfer_initialization(source,c.target,InitializationMode::Copy);
        // [conv.lval]/2 suppresses the copy in an unevaluated operand. The
        // ellipsis candidate stays viable; an evaluated application diagnoses
        // a missing/deleted/inaccessible transfer after selecting the call.
        c.rank = 6;
        c.ellipsis_object = true;
        return c;
    }
    if (fundamental(c.target, FT_FLOAT)) c.target = types.fundamental(FT_DOUBLE);
    if (fundamental(c.target, FT_NULLPTR_T)) c.target = types.compound(TypeKind::Pointer, types.fundamental(FT_VOID));
    return c;
}
TypeId Analyzer::fundamental_cast_type(ETokenType op)
{
            switch (op) {
            case KW_INT: return types.fundamental(FT_INT);
            case KW_INT128: return types.fundamental(FT_INT128);
            case KW_UINT128: return types.fundamental(FT_UINT128);
            case KW_BOOL: return types.fundamental(FT_BOOL);
            case KW_CHAR: return types.fundamental(FT_CHAR);
            case KW_SHORT: return types.fundamental(FT_SHORT_INT);
            case KW_SIGNED: return types.fundamental(FT_INT);
            case KW_UNSIGNED: return types.fundamental(FT_UNSIGNED_INT);
            case KW_WCHAR_T: return types.fundamental(FT_WCHAR_T);
            case KW_CHAR16_T: return types.fundamental(FT_CHAR16_T);
            case KW_CHAR32_T: return types.fundamental(FT_CHAR32_T);
            case KW_LONG: return types.fundamental(FT_LONG_INT);
            case KW_FLOAT: return types.fundamental(FT_FLOAT);
            case KW_DOUBLE: return types.fundamental(FT_DOUBLE);
            case KW_VOID: return types.fundamental(FT_VOID);
            default: return 0;
            }
}
Expression Analyzer::call_expression(NodeId n, ScopeId s)
{
    NodeId callee = ast[n].first, args_node = ast[callee].next;
    expand_expression_list(args_node,s);
    std::vector<NodeId> args;
    for (NodeId a = ast[args_node].first; a; a = ast[a].next) { expression(a, s); args.push_back(a); }
    Expression result;
    if (ast[callee].kind == Kind::IdExpression) {
        IdentifierId name = terminal(ast[callee].detail);
        NodeId detail = ast[callee].detail;
        if (operator_token(detail) == KW_NEW || operator_token(detail) == KW_DELETE)
            global_allocation(operator_token(detail),array_operator(detail));
        EntityId e = detail && ast[detail].kind == Kind::Name && !ast[ast[detail].first].detail ? resolve(detail, s) : 0;
        bool builtin_name = ast[detail].kind == Kind::Name && ast[detail].first == ast[detail].last;
        if (!e && builtin_name && name == invoke_builtin) {
            if (args.empty()) throw std::runtime_error("invoke requires a callable");
            auto callable = args.front(); args.erase(args.begin());
            result = callable_expression(n,s,callable,std::move(args),expressions[callable]);
            if (!result.object_use) record_object(result,0,0,0);
            object_uses[result.object_use].callee = callable;
            return result;
        }
        if (!e && builtin_name && name == expect_builtin) {
            if (args.size() != 2) throw std::runtime_error("expect takes two arguments");
            result.type = types.fundamental(FT_LONG_INT); result.form = ExpressionForm::Expect;
            std::vector<Conversion> chosen;
            for (auto arg : args) {
                auto c = conversion(arg,result.type);
                if (!c.valid()) throw std::runtime_error("invalid expect operand");
                chosen.push_back(c);
            }
            record_call(result,args,chosen); return result;
        }
        if (!e && builtin_name && name == constant_builtin) {
            if (args.size() != 1) throw std::runtime_error("constant query arity");
            result.type = types.fundamental(FT_INT); result.form = ExpressionForm::ConstantQuery;
            Constant v(result.type, evaluate(args[0], s).valid);
            facts.edit(n).value = constants.size(); constants.push_back(v);
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
            bool length = spelling.equals("__builtin_strlen");
            const char* library = spelling.equals("__builtin_memcmp") ? "memcmp" :
                spelling.equals("__builtin_memchr") ? "memchr" :
                spelling.equals("__builtin_memset") ? "memset" : nullptr;
            if (library) {
                auto v = types.fundamental(FT_VOID), i = types.fundamental(FT_INT);
                auto ptr = types.compound(TypeKind::Pointer,v);
                auto cp = types.compound(TypeKind::Pointer,types.qualify(v,1));
                auto size = types.fundamental(FT_UNSIGNED_LONG_INT);
                bool compare = library[3] == 'c' && library[4] == 'm';
                bool set = library[3] == 's';
                e = declare_function(global,name,0,types.function(compare ? i : ptr,{set ? ptr : cp,compare ? cp : i,size},false));
                entities[e].c_linkage = true; entities[e].exception_spec = 129;
                assembler_names.put(e,ids.intern(TextView(library,std::char_traits<char>::length(library))));
            }
            if (length) {
                TypeId cp = types.compound(TypeKind::Pointer, types.qualify(types.fundamental(FT_CHAR), 1));
                e = declare_function(global, name, 0, types.function(types.fundamental(FT_UNSIGNED_LONG_INT), {cp}, false));
                entities[e].builtin = Entity::Strlen;
            }
            if (floating_builtin(n,s,name,args,result)) return result;
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
        if (e && (entities[e].kind == EntityKind::Alias || entities[e].kind == EntityKind::Type)) {
            bool applied_alias = entities[e].kind == EntityKind::Alias && entities[e].template_info &&
                child(ast[detail].last,Kind::TemplateArguments);
            cast_type = applied_alias ? facts[ast[detail].last].type : entities[e].type;
            if (!cast_type) throw std::logic_error("alias call has no applied type");
        }
        if (auto fundamental = fundamental_cast_type(ast[callee].op)) cast_type = fundamental;
        if (cast_type) {
            if (class_value(cast_type)) complete_class(types[cast_type].entity);
            auto list_value = [&]() {
                auto c = list_initialization(args_node,cast_type,s,true);
                cast_type = c.target;
                result.type = cast_type; result.form = ExpressionForm::ListValue;
                record_conversion(result,args_node,c); facts.edit(n).type = cast_type;
                record_object(result,0,0,0);
                object_uses[result.object_use].temporary = list_objects[conversions[result.conversions].materialization].temporary;
                return result;
            };
            if ((ast[args_node].kind == Kind::BracedInit && class_value(cast_type)) ||
                types[cast_type].kind == TypeKind::Array) return list_value();
            if (types[cast_type].kind == TypeKind::Named && entities[types[cast_type].entity].class_info) {
                EntityId ctor = choose_constructor(cast_type,args,&result,s,true,false,0,aggregate_type(cast_type));
                if (!ctor) return list_value();
                if (converting_transfer(ctor,result)) {
                    auto c = result_conversion(ctor,result,cast_type);
                    result = Expression(); result.type = cast_type; result.form = ExpressionForm::Cast;
                    record_conversion(result,args[0],c); facts.edit(n).type = cast_type;
                    return result;
                }
                members[entities[ctor].member_info].complete_entry = true;
                { auto& published = facts.edit(n); published.entity = ctor; published.type = cast_type; }
                result.type = cast_type; result.form = ExpressionForm::Construction;
                EntityId temporary = make_entity(EntityKind::Variable, make_scope(ScopeKind::Block, s), 0, n);
                entities[temporary].type = cast_type; register_destruction(temporary);
                record_object(result, 0, 0, 0); object_uses[result.object_use].temporary = temporary;
                object_uses[result.object_use].value_initialize = args.empty() && members[entities[ctor].member_info].synthetic && !members[entities[ctor].member_info].defaulted_late;
                if (object_uses[result.object_use].value_initialize) prepare_zero_initialization(entities[scopes[entities[ctor].owner].entity].type);
                return result;
            }
            if (args.size() > 1) throw std::runtime_error("scalar cast arity");
            result = cast_expression(n, s, cast_type, args.empty() ? 0 : args[0]);
            if (ast[args_node].kind == Kind::BracedInit && !args.empty()) {
                auto c = conversions[result.conversions];
                list_conversion_from(args[0],expressions[args[0]].type,value_type(cast_type),&c);
            }
            return result;
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
        if (definitions) selected = explicit_template(ast[callee].detail,selected,s);
        fn.entity = selected; fn.form = ExpressionForm::Overload; fn.category = ValueCategory::Lvalue;
        expressions.set(callee,fn); expressions.ready(callee,true); expressions.evaluated(callee,!unevaluated_depth);
        { auto& published = facts.edit(callee); published.entity = selected; published.scope = s; }
    } else fn = expression(callee, s);
    return callable_expression(n,s,callee,std::move(args),fn);
}
} }
