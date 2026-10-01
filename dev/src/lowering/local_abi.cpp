#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
abi_mangle::Id Procedural::abi_template_head(EntityId entity)
{
    const auto head = sem.template_head(entity);
    std::vector<abi_mangle::Id> declarations;
    for (unsigned i = 0; i < head.count; ++i) {
        auto p = sem.template_parameter(head.offset+i);
        auto e = sem.entities[p];
        auto kind = e.kind == semantic::EntityKind::Parameter ? 1u : e.template_info ? 2u : 0u;
        auto value = kind == 1 ? abi_type(e.type) : kind == 2 ? abi_template_head(p) : 0;
        auto decl = abi.make(abi_mangle::Kind::TemplateParameterDeclaration,kind,value);
        if (e.parameter_pack) decl = abi.make(abi_mangle::Kind::TemplateParameterDeclaration,3,decl);
        declarations.push_back(decl);
    }
    return abi.make(abi_mangle::Kind::TemplateHead,0,0,0,0,declarations);
}
abi_mangle::Id Procedural::abi_tagged_name(EntityId e, abi_mangle::Id name)
{
    auto head = sem.abi_tag_heads.get(e);
    if (!head) return name;
    std::vector<abi_mangle::Id> tags;
    for (auto t = head; t; t = sem.abi_tags[t].next)
        tags.push_back(abi.string(spelling(sem.abi_tags[t].name)));
    return abi.make(abi_mangle::Kind::Tagged,name,0,0,0,tags);
}
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
    // Implicit NTTP conversions establish semantic type/value identity, but
    // are not source expressions in a dependent mangling.
    while (sem.type_query(q).kind == semantic::QueryKind::Cast && sem.type_query(q).op == TOK_INVALID)
        q = sem.type_query_child(q,0);
    auto query = sem.type_query(q);
    if (query.kind == semantic::QueryKind::Value && query.value &&
        (sem.types[query.type].kind == TypeKind::Pointer || sem.types[query.type].kind == TypeKind::LRef || sem.types[query.type].kind == TypeKind::MemberPointer)) {
        auto address = sem.constant_static_value(semantic::Constant(query.type,query.value));
        auto e = sem.types[query.type].kind == TypeKind::MemberPointer ?
            sem.member_constant_value(semantic::Constant(query.type,query.value)).member : address.entity;
        auto entity = sem.entities[e].kind == semantic::EntityKind::Function ? abi_function_context(e) :
            abi.make(Kind::VariableEntity,abi_entity_name(e),internal_entity(e));
        return abi.make(Kind::EntityArgument,entity,sem.types[query.type].kind != TypeKind::LRef);
    }
    auto value = abi_query(q);
    return sem.type_query(q).kind == semantic::QueryKind::Value ? value : abi.make(Kind::ExpressionArgument,value);
}
abi_mangle::Id Procedural::abi_entity_name(EntityId e)
{
    auto entity = sem.entities[e];
    auto linkage_name = sem.type_linkage_names.get(e);
    auto name = abi.name(abi_scope(entity.owner),spelling(linkage_name ? linkage_name : entity.name));
    name = abi_tagged_name(e,name);
    if (!entity.specialization) return name;
    auto pattern = sem.specialization_pattern(e);
    if (sem.entities[pattern].template_parameter) name = abi_type(sem.entities[pattern].type);
    auto pack = sem.specialization_arguments(e);
    std::vector<abi_mangle::Id> arguments;
    bool dependent = sem.dependent_type(entity.type);
    for (unsigned j = 0; j < pack.count; ++j) {
        auto arg = sem.template_argument(pack.offset+j);
        // A dependent template-id retains its argument list, before matching
        // a class head groups trailing arguments into a semantic pack.
        if (dependent && sem.argument_pack(arg)) {
            auto values = sem.pack_arguments(arg);
            for (unsigned k = 0; k < values.count; ++k)
                arguments.push_back(abi_argument(sem.template_argument(values.offset+k)));
        } else arguments.push_back(abi_argument(arg));
    }
    return entity.kind == semantic::EntityKind::Type ? abi_template_type(name,arguments) :
        abi.make(abi_mangle::Kind::Template,name,0,0,0,arguments);
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
bool Procedural::internal_entity(EntityId id)
{
    if (internal_entities.size() <= id) internal_entities.resize(sem.entities.size());
    if (internal_entities[id]) return internal_entities[id] == 2;
    auto e = sem.entities[id];
    bool local = (!e.c_linkage && internal_scope(e.owner)) || local_abi_scope(e.owner) ||
        (sem.scopes[e.owner].kind != semantic::ScopeKind::Class &&
         (e.is_static || (e.kind == semantic::EntityKind::Variable && (sem.types[e.type].cv & 1) && !e.external_decl)));
    if (!local && e.specialization) {
        auto args = sem.specialization_arguments(id);
        for (unsigned j = 0; j < args.count; ++j) local |= local_abi_argument(sem.template_argument(args.offset+j));
    }
    internal_entities[id] = local ? 2 : 1; return local;
}
bool Procedural::local_abi_argument(semantic::ArgumentId arg)
{
    if (semantic::value_argument(arg)) {
        auto q = sem.type_query(semantic::argument_query(arg));
        if (q.kind != semantic::QueryKind::Value || !q.value ||
            (sem.types[q.type].kind != TypeKind::Pointer && sem.types[q.type].kind != TypeKind::LRef && sem.types[q.type].kind != TypeKind::MemberPointer)) return false;
        auto e = sem.types[q.type].kind == TypeKind::MemberPointer ? sem.member_constant_value(semantic::Constant(q.type,q.value)).member :
            sem.constant_static_value(semantic::Constant(q.type,q.value)).entity;
        return e && internal_entity(e);
    }
    return local_abi_type(arg);
}
bool Procedural::local_abi_type(TypeId t)
{
    if (!t) return false;
    if (local_abi_types.size() <= t) local_abi_types.resize(sem.types.records.size());
    if (local_abi_types[t]) return local_abi_types[t] == 2;
    auto type = sem.types[t];
    bool local = local_abi_type(type.child);
    if (type.kind == TypeKind::ArgumentPack) {
        auto args = sem.pack_arguments(t);
        for (unsigned j = 0; j < args.count; ++j)
            local |= local_abi_argument(sem.template_argument(args.offset+j));
    } else if (type.kind == TypeKind::Named) {
        auto e = sem.entities[type.entity];
        auto closure = sem.closure(type.entity);
        local |= closure.function && (!linkage.host || !closure.enclosing);
        local |= internal_scope(e.owner) || local_abi_scope(e.owner);
        auto args = sem.specialization_arguments(type.entity);
        for (unsigned j = 0; j < args.count; ++j) local |= local_abi_argument(sem.template_argument(args.offset+j));
    } else if (type.kind == TypeKind::MemberPointer) local |= local_abi_type(type.member_owner());
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
    bool local = local_abi_scope(scope.parent);
    if (scope.kind == semantic::ScopeKind::Function) {
        auto fn = sem.entities[scope.entity];
        local |= !linkage.host || (!fn.inline_function && !fn.specialization && !fn.template_member) || internal_entity(scope.entity);
    }
    if (scope.kind == semantic::ScopeKind::Class) {
        auto closure = sem.closure(scope.entity);
        local |= closure.function && (!linkage.host || !closure.enclosing);
    }
    if (scope.kind == semantic::ScopeKind::Class && sem.entities[scope.entity].specialization)
        local |= local_abi_type(sem.entities[scope.entity].type);
    local_abi_scopes[s] = local ? 2 : 1; return local;
}
abi_mangle::Id Procedural::abi_function_context(EntityId e)
{
    auto entity = sem.entities[e]; auto t = sem.types[entity.type];
    abi_mangle::Function function;
    function.name = abi_tagged_name(e,abi.name(abi_scope(entity.owner),spelling(entity.name)));
    function.category = entity.member_info ? abi_mangle::FunctionCategory::Member : abi_mangle::FunctionCategory::Nonmember;
    function.terminal = operator_terminal(e); function.qualifiers = t.cv;
    if (auto suffix = sem.literal_suffix(e)) {
        function.terminal = abi_mangle::ABI_TERMINAL_LITERAL;
        function.literal_suffix = abi.string(spelling(suffix));
    }
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
    for (unsigned j = 0; j < pack.count; ++j) {
        auto argument = abi_argument(sem.template_argument(pack.offset+j));
        auto parameter_type = sem.dependent_function_template_parameter_type(e,j);
        if (parameter_type &&
            (abi[argument].kind == abi_mangle::Kind::Value || abi[argument].kind == abi_mangle::Kind::WideValue ||
             abi[argument].kind == abi_mangle::Kind::NegativeWideValue))
            argument = abi.make(abi_mangle::Kind::DependentValue,abi_type(parameter_type),argument);
        target.arguments.push_back(argument);
    }
    auto t = sem.types[sem.entities[sem.specialization_pattern(e)].type];
    // The conversion-type-id belongs to the template declaration, just like
    // its parameter types. The specialization arguments supply concrete types.
    if (sem.member_fact(e).conversion_target) target.conversion = abi_type(t.child);
    target.result = abi_type(t.child); target.parameters.clear();
    for (unsigned j = 0; j < t.count; ++j) target.parameters.push_back(abi_type(sem.types.parameters[t.offset+j]));
}
void Procedural::local_member_abi(EntityId e, abi_mangle::Function& target)
{
    if (!sem.entities[e].member_info) return;
    auto cls = sem.scopes[sem.entities[e].owner].entity;
    if (!sem.local_function(cls)) return;
    auto local = abi_type(sem.entities[cls].type);
    auto component = abi[local].kind == abi_mangle::Kind::Tagged ? abi[local].a : local;
    target.context = abi[component].a; target.local_owner = local;
    target.name = abi_tagged_name(e,abi.name(0,spelling(sem.entities[e].name)));
}
} }
