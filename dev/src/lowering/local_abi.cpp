#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
abi_mangle::Id Procedural::abi_argument(semantic::ArgumentId argument)
{
    using abi_mangle::Kind;
    if (sem.argument_pack(argument)) {
        std::vector<abi_mangle::Id> args; auto pack = sem.pack_arguments(argument);
        for (unsigned j = 0; j < pack.count; ++j) args.push_back(abi_argument(sem.template_argument(pack.offset+j)));
        return abi.make(Kind::ArgumentPack,0,0,0,0,args);
    }
    if (!semantic::value_argument(argument) && sem.types[argument].kind == TypeKind::PackExpansion &&
        semantic::value_argument(sem.types[argument].bound))
        return abi.make(Kind::ExpressionArgument,abi.make(Kind::ExprPack,abi_query(semantic::argument_query(sem.types[argument].bound))));
    if (!semantic::value_argument(argument)) return abi.make(Kind::TypeArgument,abi_type(argument));
    auto q = semantic::argument_query(argument);
    auto value = abi_query(q);
    return sem.type_query(q).kind == semantic::QueryKind::Value ? value : abi.make(Kind::ExpressionArgument,value);
}
abi_mangle::Id Procedural::abi_entity_name(EntityId e)
{
    auto entity = sem.entities[e];
    auto name = abi.name(abi_scope(entity.owner),spelling(entity.name));
    if (!entity.specialization) return name;
    auto pack = sem.specialization_arguments(e);
    std::vector<abi_mangle::Id> arguments;
    for (unsigned j = 0; j < pack.count; ++j)
        arguments.push_back(abi_argument(sem.template_argument(pack.offset+j)));
    return abi.make(abi_mangle::Kind::Template,name,0,0,0,arguments);
}
bool Procedural::internal_scope(semantic::ScopeId s)
{
    if (!s || s == sem.global) return false;
    if (internal_scopes.size() <= s) internal_scopes.resize(sem.scopes.size());
    if (internal_scopes[s]) return internal_scopes[s] == 2;
    auto scope = sem.scopes[s];
    bool local = (scope.kind == semantic::ScopeKind::Namespace && !scope.name) ||
        (scope.kind == semantic::ScopeKind::Function && sem.entities[scope.entity].is_static &&
         sem.scopes[sem.entities[scope.entity].owner].kind != semantic::ScopeKind::Class) || internal_scope(scope.parent);
    internal_scopes[s] = local ? 2 : 1;
    return local;
}
bool Procedural::local_abi_type(TypeId t)
{
    if (!t) return false;
    if (local_abi_types.size() <= t) local_abi_types.resize(sem.types.records.size());
    if (local_abi_types[t]) return local_abi_types[t] == 2;
    auto type = sem.types[t];
    bool local = local_abi_type(type.child);
    if (type.kind == TypeKind::Named) {
        auto e = sem.entities[type.entity];
        local |= local_abi_scope(e.owner);
        auto args = sem.specialization_arguments(type.entity);
        for (unsigned j = 0; j < args.count; ++j) local |= local_abi_type(sem.argument_type(sem.template_argument(args.offset+j)));
    } else if (type.kind == TypeKind::MemberPointer) local |= local_abi_type(sem.entities[type.entity].type);
    if (type.kind == TypeKind::Function)
        for (unsigned j = 0; j < type.count; ++j) local |= local_abi_type(sem.types.parameters[type.offset+j]);
    local_abi_types[t] = local ? 2 : 1; return local;
}
bool Procedural::local_abi_scope(semantic::ScopeId s)
{
    if (!s || s == sem.global) return false;
    if (local_abi_scopes.size() <= s) local_abi_scopes.resize(sem.scopes.size());
    if (local_abi_scopes[s]) return local_abi_scopes[s] == 2;
    auto scope = sem.scopes[s];
    bool local = scope.kind == semantic::ScopeKind::Function || local_abi_scope(scope.parent);
    if (scope.kind == semantic::ScopeKind::Class && sem.entities[scope.entity].specialization)
        local |= local_abi_type(sem.entities[scope.entity].type);
    local_abi_scopes[s] = local ? 2 : 1; return local;
}
abi_mangle::Id Procedural::abi_function_context(EntityId e)
{
    auto entity = sem.entities[e]; auto t = sem.types[entity.type];
    abi_mangle::Function function;
    function.name = abi.name(abi_scope(entity.owner),spelling(entity.name));
    function.category = entity.member_info ? abi_mangle::FunctionCategory::Member : abi_mangle::FunctionCategory::Nonmember;
    function.terminal = operator_terminal(e); function.qualifiers = t.cv;
    if (t.ref != semantic::RefQualifier::None) function.qualifiers |= t.ref == semantic::RefQualifier::Lvalue ? 4 : 8;
    function.variadic = t.variadic; function.c_linkage = entity.c_linkage;
    for (unsigned j = 0; j < t.count; ++j) function.parameters.push_back(abi_type(sem.types.parameters[t.offset+j]));
    if (sem.constructor_member(e)) function.terminal = abi_mangle::ABI_TERMINAL_CONSTRUCTOR_COMPLETE;
    if (sem.destructor_member(e)) function.terminal = abi_mangle::ABI_TERMINAL_DESTRUCTOR_COMPLETE;
    if (auto conversion = sem.member_fact(e).conversion_target) function.conversion = abi_type(conversion);
    local_member_abi(e,function);
    template_function_abi(e,function);
    return abi_mangle::function_entity(abi,function);
}
void Procedural::template_function_abi(EntityId e, abi_mangle::Function& target)
{
    if (!sem.entities[e].specialization) return;
    auto pack = sem.specialization_arguments(e);
    target.template_prefix = true;
    for (unsigned j = 0; j < pack.count; ++j)
        target.arguments.push_back(abi_argument(sem.template_argument(pack.offset+j)));
    auto t = sem.types[sem.entities[sem.specialization_pattern(e)].type];
    target.result = abi_type(t.child); target.parameters.clear();
    for (unsigned j = 0; j < t.count; ++j) target.parameters.push_back(abi_type(sem.types.parameters[t.offset+j]));
}
void Procedural::local_member_abi(EntityId e, abi_mangle::Function& target)
{
    if (!sem.entities[e].member_info) return;
    auto cls = sem.scopes[sem.entities[e].owner].entity;
    if (!sem.local_function(cls)) return;
    auto local = abi_type(sem.entities[cls].type);
    target.context = abi[local].a; target.local_owner = local;
    target.name = abi.name(0,spelling(sem.entities[e].name));
}
} }
