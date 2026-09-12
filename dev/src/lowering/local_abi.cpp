#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
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
    return abi_mangle::function_entity(abi,function);
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
