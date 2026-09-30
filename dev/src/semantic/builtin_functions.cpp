#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
EntityId Analyzer::builtin_function(IdentifierId name)
{
    auto text = ids.spelling(name);
    auto kind = text.equals("__builtin_va_start") ? Intrinsic::VaStart :
        text.equals("__builtin_va_end") ? Intrinsic::VaEnd :
        text.equals("__builtin_va_copy") ? Intrinsic::VaCopy :
        text.equals("__builtin_alloca") ? Intrinsic::StackAlloc : Intrinsic::None;
    bool bounded = text.equals("__builtin_vsnprintf"), print = text.equals("__builtin_vsprintf");
    auto floating = text.equals("__builtin_fabs") ? FT_DOUBLE : text.equals("__builtin_fabsf") ? FT_FLOAT :
        text.equals("__builtin_fabsl") ? FT_LONG_DOUBLE : FT_VOID;
    if (floating != FT_VOID) {
        auto t = types.fundamental(floating);
        auto e = declare_function(global,name,0,types.function(t,{t},false));
        entities[e].c_linkage = true; entities[e].exception_spec = 129;
        assembler_names.put(e,ids.intern(TextView(text.data+10,text.size-10))); return e;
    }
    if (kind == Intrinsic::None && !bounded && !print) return 0;
    auto v = types.fundamental(FT_VOID), size = types.fundamental(FT_UNSIGNED_LONG_INT);
    auto va = types.adjusted(variadic_list_type);
    auto ret = v; std::vector<TypeId> args;
    if (kind == Intrinsic::StackAlloc) { args.push_back(size); ret = types.compound(TypeKind::Pointer,v); }
    else if (kind != Intrinsic::None) {
        args.push_back(va);
        if (kind == Intrinsic::VaCopy) args.push_back(va);
    } else {
        auto c = types.fundamental(FT_CHAR);
        args.push_back(types.compound(TypeKind::Pointer,c));
        if (bounded) args.push_back(size);
        args.push_back(types.compound(TypeKind::Pointer,types.qualify(c,1))); args.push_back(va);
        ret = types.fundamental(FT_INT);
    }
    auto e = declare_function(global,name,0,types.function(ret,args,kind == Intrinsic::VaStart));
    entities[e].c_linkage = true; entities[e].exception_spec = 129;
    if (kind != Intrinsic::None) intrinsic_functions.put(e,unsigned(kind));
    else assembler_names.put(e,ids.intern(bounded ? TextView("vsnprintf",9) : TextView("vsprintf",8)));
    return e;
}
void Analyzer::validate_intrinsic(EntityId e, const std::vector<NodeId>& args, ScopeId s)
{
    if (Intrinsic(intrinsic_functions.get(e)) != Intrinsic::VaStart) return;
    if (args.size() != 2) throw std::runtime_error("va_start takes two arguments");
    EntityId fn = 0;
    for (auto scope = s; scope; scope = scopes[scope].parent)
        if (scopes[scope].kind == ScopeKind::Function) { fn = scopes[scope].entity; break; }
    if (!fn || !types[entities[fn].type].variadic)
        throw std::runtime_error("va_start requires a variadic function");
    auto last = args[1];
    while (ast[last].kind == syntax::Kind::Parenthesized) last = ast[last].first;
    auto parameter = expressions[last].entity;
    if (ast[last].kind != syntax::Kind::IdExpression || entities[parameter].kind != EntityKind::Parameter)
        throw std::runtime_error("va_start requires a named parameter");
    auto owner = entities[parameter].owner;
    if (scopes[owner].kind != ScopeKind::Function || scopes[owner].entity != fn)
        throw std::runtime_error("va_start parameter belongs to another function");
    auto ordinal = signature_parameters.get(parameter);
    if (ordinal) {
        if (ordinal != types[entities[fn].type].count) throw std::runtime_error("va_start requires the last parameter");
    } else {
        EntityId final = 0;
        for (auto d = scopes[owner].first_decl; d; d = declarations[d].next)
            if (entities[declarations[d].entity].kind == EntityKind::Parameter) final = declarations[d].entity;
        if (parameter != final) throw std::runtime_error("va_start requires the last parameter");
    }
}
TypeQueryFact Analyzer::query_builtin_operand(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    TypeQueryFact result;
    if (q.kind == QueryKind::Typeof) {
        result.expression.type = children[0].expression.type;
        if (!result.expression.type && children[0].declared_type)
            result.expression.type = value_type(children[0].declared_type);
    } else {
        result.expression.type = q.type;
        if (!standard_conversion(children[0].expression,types.adjusted(variadic_list_type)).valid())
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    }
    return result;
}
EntityId Analyzer::predefined_function_name(NodeId n, ScopeId s)
{
    while (s && scopes[s].kind != ScopeKind::Function) s = scopes[s].parent;
    if (!s) throw std::runtime_error("predefined function name outside function");
    auto name = ast[n].text;
    if (auto old = local(s,name)) return old;
    auto fn = scopes[s].entity;
    auto content = entities[fn].name;
    auto text = ids.spelling(content);
    auto e = make_entity(EntityKind::Variable,s,name,n);
    entities[e].type = types.compound(TypeKind::Array,types.qualify(types.fundamental(FT_CHAR),1),text.size+1);
    entities[e].is_static = true; entities[e].definition = n;
    entities[e].template_pattern = pattern_scope(s);
    predefined_strings.put(e,content); bind(s,name,e);
    return e;
}
Expression Analyzer::va_arg_expression(NodeId n, ScopeId s)
{
    auto arg = ast[n].first;
    expression(arg,s);
    auto c = conversion(arg,types.adjusted(variadic_list_type));
    if (!c.valid()) throw std::runtime_error("va_arg requires va_list");
    Expression result; result.type = type_id(ast[arg].next,s);
    auto type = types[result.type];
    if (class_value(result.type) || type.kind == TypeKind::Array || type.kind == TypeKind::Function ||
        type.kind == TypeKind::LRef || type.kind == TypeKind::RRef || fundamental(result.type,FT_VOID))
        throw std::runtime_error("unsupported va_arg result type");
    record_conversion(result,arg,c); return result;
}
} }
