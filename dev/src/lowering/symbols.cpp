#include "lowering/procedural.h"
#include "support/type_traits.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using namespace lowir_model;
std::string Procedural::spelling(IdentifierId id) const
{
    TextView text = identifiers.spelling(id); return std::string(text.data, text.size);
}
abi_mangle::Id Procedural::abi_scope(semantic::ScopeId s)
{
    if (!s || s == sem.global) return 0;
    if (abi_scopes[s]) return abi_scopes[s];
    auto scope = sem.scopes[s];
    if (scope.kind == semantic::ScopeKind::Class)
        return abi_scopes[s] = abi_entity_name(scope.entity);
    auto parent = abi_scope(scope.parent);
    if (scope.kind == semantic::ScopeKind::Template) return abi_scopes[s] = parent;
    return abi_scopes[s] = abi.name(parent, scope.name ? spelling(scope.name) : "_GLOBAL__N_1");
}
abi_mangle::Id Procedural::abi_type(TypeId id)
{
    using namespace abi_mangle;
    if (id >= abi_types.size()) abi_types.resize(sem.types.records.size());
    if (abi_types[id]) return abi_types[id];
    auto t = sem.types[id];
    abi_mangle::Id result = 0;
    if (t.cv && t.kind != TypeKind::Function) {
        result = abi_type(sem.types.non_atomic(sem.types.unqualified(id)));
        if (t.cv & 4) result = abi.make(Kind::Vendor,result,abi.string("_Atomic"));
        if (t.cv & 3) result = abi.cv(result,t.cv & 3);
    }
    else switch (t.kind) {
    case TypeKind::Fundamental: {
        static const AbiBuiltinTypeKind kinds[] = {ABI_BUILTIN_TYPE_SIGNED_CHAR, ABI_BUILTIN_TYPE_SHORT,
            ABI_BUILTIN_TYPE_INT, ABI_BUILTIN_TYPE_LONG, ABI_BUILTIN_TYPE_LONG_LONG,
            ABI_BUILTIN_TYPE_UNSIGNED_CHAR, ABI_BUILTIN_TYPE_UNSIGNED_SHORT, ABI_BUILTIN_TYPE_UNSIGNED_INT,
            ABI_BUILTIN_TYPE_UNSIGNED_LONG, ABI_BUILTIN_TYPE_UNSIGNED_LONG_LONG, ABI_BUILTIN_TYPE_WCHAR,
            ABI_BUILTIN_TYPE_CHAR, ABI_BUILTIN_TYPE_CHAR16, ABI_BUILTIN_TYPE_CHAR32, ABI_BUILTIN_TYPE_BOOL,
            ABI_BUILTIN_TYPE_FLOAT, ABI_BUILTIN_TYPE_DOUBLE, ABI_BUILTIN_TYPE_LONG_DOUBLE, ABI_BUILTIN_TYPE_VOID, ABI_BUILTIN_TYPE_NULLPTR, ABI_BUILTIN_TYPE_INT128, ABI_BUILTIN_TYPE_UINT128};
        result = abi.builtin(kinds[t.fundamental]); break;
    }
    case TypeKind::Named: {
        auto e = sem.entities[t.entity];
        if (sem.placeholder_entity(t.entity)) result = abi.builtin(ABI_BUILTIN_TYPE_AUTO);
        else if (e.template_parameter) result = abi.make(abi_mangle::Kind::Parameter,0,1,e.parameter_pack,sem.template_ordinal(t.entity));
        else if (sem.closure(t.entity).function && sem.closure(t.entity).enclosing) {
            auto closure = sem.closure(t.entity);
            auto enclosing = sem.entities[closure.enclosing];
            if (linkage.host && !enclosing.inline_function && !enclosing.specialization && !enclosing.template_member) {
                // TU-unique closures have no prescribed external ABI encoding.
                // Use a separate local ordinal, independent of signature groups.
                result = abi.make(abi_mangle::Kind::Local,abi_function_context(closure.enclosing),
                    abi.string("$_"+std::to_string(closure.local_ordinal)));
                break;
            }
            auto f = sem.types[closure.signature];
            std::vector<abi_mangle::Id> params;
            if (sem.entities[closure.function].template_info) params.push_back(abi_template_head(closure.function));
            for (unsigned i = 0; i < f.count; ++i) params.push_back(abi_type(sem.types.parameters[f.offset+i]));
            result = abi.make(abi_mangle::Kind::Lambda,abi_function_context(closure.enclosing),!closure.ordinal,f.variadic,
                closure.ordinal ? closure.ordinal-1 : 0,params);
        } else if (sem.local_function(t.entity)) {
            auto linkage_name = sem.type_linkage_names.get(t.entity);
            auto unnamed = linkage_name ? 0 : sem.local_unnamed_types.get(t.entity);
            result = abi_tagged_name(t.entity,abi.make(abi_mangle::Kind::Local,abi_function_context(sem.local_function(t.entity)),
                unnamed ? 0 : abi.string(spelling(linkage_name ? linkage_name : e.name)),unnamed ? 1 : 0,unnamed ? unnamed-1 : sem.local_ordinal(t.entity)));
        }
        else result = abi_entity_name(t.entity);
        break;
    }
    case TypeKind::AliasApplication: result = abi_type(t.child); break;
    case TypeKind::PackExpansion: result = abi.make(abi_mangle::Kind::Pack,abi_type(t.bound)); break;
    case TypeKind::BlockPointer:
        result = abi.make(abi_mangle::Kind::Vendor,abi_type(t.child),abi.string("block_pointer")); break;
    case TypeKind::Pointer: result = abi.make(abi_mangle::Kind::Pointer, abi_type(t.child)); break;
    case TypeKind::Decltype: {
        auto query = sem.type_query(t.entity);
        if (query.kind == semantic::QueryKind::BuiltinTrait && template_type_transform(BuiltinTrait(query.value))) {
            auto args = sem.query_arguments(query.arguments);
            std::vector<abi_mangle::Id> operands;
            for (unsigned j = 0; j < args.count; ++j) operands.push_back(abi_argument(sem.template_argument(args.offset+j)));
            if (BuiltinTrait(query.value) == BuiltinTrait::TypePackElement) {
                std::vector<abi_mangle::Id> tail(operands.begin()+1,operands.end());
                operands.resize(1);
                operands.push_back(abi.make(abi_mangle::Kind::ArgumentPack,0,0,0,0,tail));
            }
            result = abi.make(abi_mangle::Kind::Template,abi.name(0,spelling(query.name)),0,0,0,operands);
        } else if (query.kind == semantic::QueryKind::BuiltinTrait && type_transform(BuiltinTrait(query.value))) {
            auto args = sem.query_arguments(query.arguments);
            std::vector<abi_mangle::Id> operands;
            for (unsigned j = 0; j < args.count; ++j) operands.push_back(abi_type(sem.template_argument(args.offset+j)));
            result = abi.make(abi_mangle::Kind::Transform,0,abi.string(spelling(query.name)),0,0,operands);
        } else result = abi.make(abi_mangle::Kind::Decltype,abi_query(t.entity),t.bound);
        break;
    }
    case TypeKind::DependentName: {
        result = abi.name(abi_type(t.child),spelling(t.entity));
        if (semantic::DependentNameKind(t.bound) == semantic::DependentNameKind::Application) {
            std::vector<abi_mangle::Id> args;
            for (unsigned j = 0; j < t.count; ++j)
                args.push_back(abi_argument(sem.types.parameters[t.offset+j]));
            result = abi.make(abi_mangle::Kind::Template,result,0,0,0,args);
        }
        break;
    }
    case TypeKind::LRef: result = abi.make(abi_mangle::Kind::Reference, abi_type(t.child)); break;
    case TypeKind::RRef: result = abi.make(abi_mangle::Kind::RvalueReference, abi_type(t.child)); break;
    case TypeKind::Vector: case TypeKind::ExtVector: result = abi.make(abi_mangle::Kind::Vector,abi_type(t.child),0,0,t.kind == TypeKind::ExtVector ? t.bound : t.bound/sem.object_size(t.child)); break;
    case TypeKind::Array: result = abi.make(abi_mangle::Kind::Array, abi_type(t.child), 0, t.unknown_bound, t.bound); break;
    case TypeKind::DependentVector: case TypeKind::DependentExtVector: result = abi.make(abi_mangle::Kind::Vector,abi_type(t.child),abi_query(t.bound)); break;
    case TypeKind::DependentArray: result = abi.make(abi_mangle::Kind::Array,abi_type(t.child),abi_query(t.bound)); break;
    case TypeKind::MemberPointer: result = abi.make(abi_mangle::Kind::MemberPointer, abi_type(t.member_owner()), abi_type(t.child)); break;
    case TypeKind::Function: {
        std::vector<abi_mangle::Id> params;
        for (unsigned j = 0; j < t.count; ++j) params.push_back(abi_type(sem.types.parameters[t.offset+j]));
        unsigned qualifiers = t.cv | (t.ref == semantic::RefQualifier::Lvalue ? 4 : t.ref == semantic::RefQualifier::Rvalue ? 8 : 0);
        result = abi.make(abi_mangle::Kind::FunctionType, abi_type(t.child), qualifiers, t.variadic, 0, params); break;
    }
    default: throw std::runtime_error("unsupported procedural ABI type");
    }
    if (id >= abi_types.size()) abi_types.resize(sem.types.records.size());
    return abi_types[id] = result;
}
bool Procedural::base_only_entry(EntityId id) const
{
    auto e = sem.entities[id];
    if (!e.member_info) return false;
    const auto& m = sem.member_fact(id);
    return m.base_entry && !m.complete_entry && !m.defaulted_late &&
        (m.synthetic || !e.body || e.template_member || m.inherited_constructor);
}
bool Procedural::separate_base(EntityId id) const
{
    auto e = sem.entities[id];
    // One demanded base entry owns one symbol/body. Reserving a complete
    // slot with the base spelling as well would duplicate its ABI identity.
    if (base_only_entry(id)) return false;
    if (e.member_info && sem.member_fact(id).defaulted_late && (sem.constructor_member(id) || sem.destructor_member(id))) return true;
    if (!e.member_info || !sem.member_fact(id).base_entry) return false;
    if (sem.virtual_base_count(sem.scopes[e.owner].entity)) return true;
    if (sem.member_fact(id).virtual_member && sem.destructor_member(id)) return true;
    if (sem.member_fact(id).polymorphic_base_entry && !sem.synthetic_member(id)) return true;
    if (e.template_member && !sem.synthetic_member(id)) return sem.member_fact(id).complete_entry;
    return sem.member_fact(id).complete_entry || (e.body && !e.inline_function);
}
SymbolId Procedural::symbol(EntityId id, bool base, bool deleting)
{
    auto e = sem.entities[id];
    // Source-string support has its own emission identity and never grows the
    // declaration-indexed lifecycle/frame tables after semantic completion.
    if (sem.predefined_string(id) && !e.name) return source_string_symbol(sem.predefined_string(id));
    bool external = sem.emission_suppressed(id) ||
        (e.kind == semantic::EntityKind::Function ? !e.body && !sem.synthetic_member(id) : !e.definition);
    bool separate = separate_base(id);
    bool base_only = base_only_entry(id);
    base = base && separate;
    if (deleting) return deleting_symbol(id);
    if ((base ? base_symbols[id] : symbols[id])) return base ? base_symbols[id] : symbols[id];
    if (sem.local_static(id)) {
        // A local declaration has no namespace variable ABI name. Distinct
        // lexical declarations and specializations already have distinct IDs.
        auto sid = fresh_symbol("@__local_static_"+std::to_string(id));
        symbols[id] = sid;
        auto& metadata = p.symbols[sid.index-1].metadata;
        metadata.binding = SBM_INTERNAL;
        if (e.thread_local_storage) metadata.storage = GSM_THREAD_LOCAL;
        return sid;
    }
    bool internal = internal_entity(id);
    std::string name = spelling(e.name);
    // Source ABI spelling and internal IR identity occupy separate namespaces.
    // In particular a root C++ variable's ABI spelling is the bare source name.
    bool entry = name == "main" && e.owner == sem.global && e.kind == semantic::EntityKind::Function;
    SymbolMetadata metadata;
    metadata.object_root = e.instantiation_definition;
    metadata.binding = internal ? SBM_INTERNAL : !external && (e.inline_function || e.inline_variable || (!e.explicit_specialization && (e.specialization || e.template_member))) ? SBM_WEAK : SBM_STRONG;
    if (sem.weak_symbols.get(id) && !internal) metadata.binding = SBM_WEAK;
    if (auto section = sem.section_names.get(id)) metadata.section = p.intern(spelling(section));
    metadata.inline_hint = e.inline_function; metadata.no_inline = e.no_inline; metadata.force_inline = e.force_inline && !e.no_inline;
    if (e.member_info) metadata.object_root |= base || (!separate && !external && sem.member_fact(id).base_entry);
    if (e.member_info) {
        metadata.object_root |= sem.member_fact(id).retained_root;
        if (local_abi_scope(e.owner) || (internal && e.specialization && local_abi_type(e.type))) {
            metadata.object_root = true; metadata.binding = SBM_INTERNAL;
        }
    }
    if (e.c_linkage) metadata.linkage = LLM_C;
    if (e.thread_local_storage) metadata.storage = GSM_THREAD_LOCAL;
    else if (e.kind == semantic::EntityKind::Variable && (sem.types[e.type].cv & 3) == 1 && type(e.type).scalar() && !reference(e.type)) metadata.storage = GSM_READONLY;
    abi_mangle::Target target;
    auto aname = e.kind == semantic::EntityKind::Variable && e.specialization ? abi_entity_name(id) : abi_tagged_name(id,abi.name(abi_scope(e.owner), name));
    if (e.kind == semantic::EntityKind::Function) {
        target.kind = abi_mangle::TargetKind::Function;
        target.function.name = aname;
        target.function.terminal = operator_terminal(id);
        if (auto conversion = sem.member_fact(id).conversion_target) target.function.conversion = abi_type(conversion);
        if (auto suffix = sem.literal_suffix(id)) {
            target.function.terminal = abi_mangle::ABI_TERMINAL_LITERAL;
            target.function.literal_suffix = abi.string(spelling(suffix));
        }
        target.function.category = e.member_info ? abi_mangle::FunctionCategory::Member : abi_mangle::FunctionCategory::Nonmember;
        target.function.qualifiers = sem.types[e.type].cv;
        if (sem.types[e.type].ref != semantic::RefQualifier::None)
            target.function.qualifiers |= sem.types[e.type].ref == semantic::RefQualifier::Lvalue ? 4 : 8;
        if (sem.constructor_member(id)) target.function.terminal = abi_mangle::ABI_TERMINAL_CONSTRUCTOR_COMPLETE;
        if (sem.destructor_member(id)) target.function.terminal = abi_mangle::ABI_TERMINAL_DESTRUCTOR_COMPLETE;
        if (base || base_only) target.function.terminal = sem.destructor_member(id) ? abi_mangle::ABI_TERMINAL_DESTRUCTOR_BASE : abi_mangle::ABI_TERMINAL_CONSTRUCTOR_BASE;
        target.function.c_linkage = e.c_linkage;
        local_member_abi(id,target.function);
        auto t = sem.types[e.type]; target.function.variadic = t.variadic;
        for (unsigned j = 0; j < t.count; ++j) target.function.parameters.push_back(abi_type(sem.types.parameters[t.offset+j]));
        template_function_abi(id,target.function);
    } else { target.kind = abi_mangle::TargetKind::Variable; target.type = aname; target.internal = internal; }
    std::uint64_t key = 0;
    if (!internal && (linkage.merge || e.c_linkage)) {
        ++linkage.requests;
        if (e.c_linkage || entry || e.builtin != semantic::Entity::NoBuiltin)
            key = (std::uint64_t(3) << 32) | abi.name(0, name);
        else if (e.kind == semantic::EntityKind::Function)
            key = (std::uint64_t(1) << 32) | abi_mangle::function_entity(abi, target.function);
        else key = (std::uint64_t(2) << 32) | aname;
        if (auto previous = linkage.external.get(key)) {
            ++linkage.hits;
            if (e.inline_variable && !external) p.symbols[previous-1].metadata.binding = metadata.binding;
            return (base ? base_symbols[id] : symbols[id]) = SymbolId(previous);
        }
    }
    if (entry) {
        metadata.role = SR_ENTRY; metadata.keep_alias = true;
    } else metadata.object = p.intern(e.c_linkage && !internal && e.kind != semantic::EntityKind::Function ? name : abi_mangle::mangle(abi, target));
    if (auto explicit_name = sem.assembler_names.get(id)) metadata.object = p.intern(spelling(explicit_name));
    // Several source TUs are emitted as one LowIR program/object. Its local
    // object labels need the same isolation as their internal SymbolIds.
    if (internal && linkage.merge && metadata.object)
        metadata.object = p.intern(p.name(metadata.object) + "." + std::to_string(p.symbols.size()+1));
    if ((e.builtin == semantic::Entity::Malloc || e.builtin == semantic::Entity::Free) && !e.definition) {
        metadata.role = e.builtin == semantic::Entity::Malloc ? SR_MALLOC : SR_FREE_MEMORY;
    } else if (e.builtin != semantic::Entity::NoBuiltin && e.builtin != semantic::Entity::Malloc && e.builtin != semantic::Entity::Free) {
        if (e.builtin == semantic::Entity::Strlen) metadata.builtin = lowir_model::SymbolMetadata::Builtin::Strlen;
        metadata.object = p.intern(linkage.host ?
            (e.builtin == semantic::Entity::Memcpy ? "memcpy" : e.builtin == semantic::Entity::Strlen ? "strlen" : "memmove") :
            (e.builtin == semantic::Entity::Memcpy ? "cppgm_builtin_memcpy" : e.builtin == semantic::Entity::Strlen ? "cppgm_builtin_strlen" : "cppgm_builtin_memmove"));
        metadata.linkage = LLM_C;
    }
    SymbolId allocation_runtime; unsigned allocation_role = 2;
    if (e.allocation_runtime && !e.definition) {
        const char* runtime[] = {"", "cppgm_builtin_operator_new", "cppgm_builtin_operator_new_array", "cppgm_builtin_operator_delete", "cppgm_builtin_operator_delete_array"};
        if (!linkage.host) metadata.object = p.intern(runtime[e.allocation_runtime]);
        allocation_role = e.key == KW_NEW ? 0 : 1;
        allocation_runtime = linkage.allocation_roles[allocation_role];
        if (!allocation_runtime) {
            metadata.role = e.key == KW_NEW ? SR_ALLOCATE_MEMORY : SR_FREE_MEMORY;
        }
    }
    // Use source spellings for both declaration kinds. SymbolId and the
    // collision allocator still own identity; native ABI names remain separate.
    std::string display = "@" + name;
    for (char& c : display) if (c != '@' && c != '_' && !(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9')) c = '_';
    if (metadata.object && p.name(metadata.object) != display.substr(1)) linkage.native_names.put(metadata.object, 1);
    SymbolId sid = fresh_symbol(display); (base ? base_symbols[id] : symbols[id]) = sid;
    if (allocation_role < 2) {
        if (allocation_runtime) allocation_adapters.push_back({sid,allocation_runtime});
        else linkage.allocation_roles[allocation_role] = sid;
    }
    // The ordinary LowIR spelling already supplies this object name. Avoid
    // asking a native adapter to publish the same label twice.
    if (metadata.object && p.name(metadata.object) == p.name(p.symbols[sid.index-1].name).substr(1)) metadata.object = 0;
    if (key) linkage.external.put(key, sid.index);
    p.symbols[sid.index-1].metadata = metadata;
    if (!external && !separate && !base_only && (sem.constructor_member(id) || sem.destructor_member(id)) && (e.body || sem.synthetic_member(id))) {
        target.function.terminal = sem.destructor_member(id) ? abi_mangle::ABI_TERMINAL_DESTRUCTOR_BASE : abi_mangle::ABI_TERMINAL_CONSTRUCTOR_BASE;
        if (!internal && linkage.merge) {
            auto base_key = (std::uint64_t(1) << 32) | abi_mangle::function_entity(abi,target.function);
            auto existing = linkage.external.get(base_key);
            if (existing) {
                auto entry = p.symbols[existing-1];
                // Another TU may have emitted this base entry before the
                // complete entry arrived. It already owns that native name.
                if (entry.kind == Symbol::FunctionSymbol && !p.functions[entry.entity-1].declaration) return sid;
            }
            // Publish alias identity too, so later base references share its
            // defining entry instead of producing a second definition.
            linkage.external.put(base_key,sid.index);
        }
        std::string alias_name = abi_mangle::mangle(abi,target);
        if (internal && linkage.merge) alias_name += "." + std::to_string(sid.index);
        ObjectAlias alias; alias.name = p.intern(alias_name); alias.target = sid; p.aliases.push_back(alias);
    }
    return sid;
}
SymbolId Procedural::fresh_symbol(const std::string& preferred)
{
    auto name = p.intern(preferred);
    // Check source and generated spellings alike. The program-wide monotonic
    // suffix makes rejected candidates unique, hence total collision work is
    // bounded by existing symbols rather than a fresh search for each entity.
    while (p.symbol_names.find(name) || linkage.native_names.get(p.intern(p.name(name).substr(1))))
        name = p.intern(preferred + "__" + std::to_string(++linkage.disambiguator));
    linkage.native_names.put(p.intern(p.name(name).substr(1)), 1);
    return p.symbol(name);
}
SignatureId Procedural::signature(TypeId id, FunctionId owner)
{
    if (!owner) {
        if (id >= indirect_signatures.size()) indirect_signatures.resize(sem.types.records.size());
        if (indirect_signatures[id]) return indirect_signatures[id];
    }
    auto t = sem.types[id];
    EntityId member_owner = t.kind == TypeKind::MemberPointer ? t.entity : 0;
    bool block_receiver = t.kind == TypeKind::BlockPointer;
    if (member_owner || block_receiver) t = sem.types[t.child];
    auto return_type = sem.types[t.child];
    bool incomplete_result = return_type.kind == TypeKind::Named && sem.entities[return_type.entity].class_info && !sem.entities[return_type.entity].complete;
    bool incomplete_signature = incomplete_result;
    bool indirect_result = sem.indirect_value(t.child);
    Signature sig; sig.result = incomplete_result || indirect_result ? IRType(IRType::Void) : type(t.child); sig.parameters.begin = p.parameters.size();
    sig.parameters.count = t.count + indirect_result + bool(member_owner || block_receiver);
    if (t.variadic) sig.boundary.arity = CAM_VARIADIC;
    if (indirect_result) {
        Parameter param; param.type = IRType::Ptr; param.passing = PPM_INDIRECT_RESULT; param.object_bytes = sem.object_size(t.child);
        lowir_model::Value v; v.type = param.type; v.owner = owner; v.defined = true;
        if (!owner) v.name = p.intern("%ret");
        p.values.push_back(v); param.value = ValueId(p.values.size()); p.parameters.push_back(param);
    }
    if (member_owner || block_receiver) {
        Parameter param; param.type = IRType::Ptr;
        if (member_owner) param.object_bytes = sem.object_size(sem.entities[member_owner].type);
        lowir_model::Value v; v.type = param.type; v.owner = owner; v.defined = true; v.name = p.intern("%arg0");
        p.values.push_back(v); param.value = ValueId(p.values.size()); p.parameters.push_back(param);
    }
    for (unsigned j = 0; j < t.count; ++j) {
        TypeId pt = sem.types.parameters[t.offset+j];
        bool incomplete = sem.class_value(pt) && !sem.entities[sem.types[pt].entity].complete;
        incomplete_signature |= incomplete;
        // Like an incomplete result above, this declaration has no callable
        // value ABI yet. Keep an opaque pointer in the explicit LowIR view;
        // every actual call/definition requires completeness in semantics.
        Parameter param; param.type = incomplete || sem.indirect_parameter(pt) ? IRType(IRType::Ptr) : type(pt);
        lowir_model::Value v; v.type = param.type; v.owner = owner; v.defined = true;
        if (!owner) v.name = p.intern("%arg" + std::to_string(j + bool(member_owner || block_receiver)));
        p.values.push_back(v); param.value = ValueId(p.values.size());
        if (incomplete) param.passing = PPM_BY_ADDRESS;
        else if (sem.indirect_parameter(pt)) { param.passing = PPM_BY_ADDRESS; param.object_bytes = sem.object_size(pt); }
        else if (reference(pt)) {
            param.passing = PPM_BY_ADDRESS;
            auto referred = sem.types[pt].child;
            if (sem.types[referred].kind != TypeKind::Function &&
                !(sem.types[referred].kind == TypeKind::Named && !sem.entities[sem.types[referred].entity].complete) &&
                !(sem.types[referred].kind == TypeKind::Array && sem.types[referred].unknown_bound)) param.object_bytes = sem.object_size(referred);
        }
        p.parameters.push_back(param);
    }
    Linkage::ParameterAbi plan; plan.visible = sig.parameters.count;
    plan.hidden.begin = linkage.value_base_arguments.size();
    for (unsigned j = 0; j < t.count; ++j) {
        auto pt = sem.types.parameters[t.offset+j];
        if (!sem.class_value(pt)) continue;
        auto cls = sem.types[pt].entity;
        if (!sem.entities[cls].complete) continue;
        for (unsigned k = 0; k < sem.virtual_base_count(cls); ++k) {
            linkage.value_base_arguments.push_back({j+unsigned(indirect_result)+unsigned(bool(member_owner || block_receiver)),
                sem.virtual_base_offset(cls,sem.virtual_base_type(cls,k))});
            Parameter param; param.type = IRType::Ptr;
            lowir_model::Value value; value.type = IRType::Ptr; value.owner = owner; value.defined = true;
            if (!owner) value.name = p.intern("%vbase"+std::to_string(sig.parameters.count));
            p.values.push_back(value); param.value = ValueId(p.values.size()); p.parameters.push_back(param);
            ++sig.parameters.count;
        }
    }
    plan.hidden.count = linkage.value_base_arguments.size()-plan.hidden.begin;
    if (plan.hidden.count) {
        linkage.signature_parameter_abis.put(p.signatures.size()+1,linkage.parameter_abis.size());
        linkage.parameter_abis.push_back(plan);
    }
    p.signatures.push_back(sig);
    if (owner && (incomplete_signature || linkage.incomplete_signatures.get(owner.index)))
        linkage.incomplete_signatures.put(owner.index,incomplete_signature);
    SignatureId result(p.signatures.size());
    if (!owner) indirect_signatures[id] = result;
    return result;
}
Procedural::Procedural(syntax::Ast& a, semantic::Analyzer& s, IdentifierTable& ids, Program& out, Linkage& links)
    : ast(a), sem(s), identifiers(ids), p(out), linkage(links), abi(links.abi), abi_types(s.types.records.size()), abi_scopes(s.scopes.size()),
      symbols(s.entities.size()), strings(a.nodes.size()), base_symbols(s.entities.size()), objects(s.entities.size()), object_addresses(s.entities.size()), labels(a.nodes.size()), control_entries(a.nodes.size()) {
    virtual_signatures.resize(s.member_count()); vtables.resize(s.virtual_class_count());
    deleting_symbols.resize(s.member_count());
}
void Procedural::run()
{
    std::vector<EntityId> reference_objects, deferred_conversions;
    semantic::Index reference_counts;
    for (auto storage : sem.reference_storage) {
        if (storage.reference && sem.entities[storage.reference].inline_variable) {
            auto ordinal = reference_counts.get(storage.reference)+1;
            reference_counts.put(storage.reference,ordinal); reference_ordinals.put(storage.object,ordinal);
        }
        if (!storage.reference || (!sem.local_static(storage.reference) && !sem.entities[storage.reference].inline_variable) || !sem.destructor_needed(sem.object_destructor(storage.object))) continue;
        auto next = local_static_references.get(storage.reference);
        local_static_references.put(storage.reference,local_static_reference_objects.size());
        local_static_reference_objects.push_back({storage.object,next});
    }
    for (EntityId e = 1; e < symbols.size(); ++e) {
        auto entity = sem.entities[e];
        if (sem.predefined_string(e) && !entity.name) continue; // Demand support bytes only through a consumed address.
        if (entity.template_pattern) continue;
        if (sem.dormant_inline_variable(e)) continue;
        if (sem.deferred_inline_function(e) && !(entity.emission & semantic::Entity::Used) &&
            !entity.instantiation_definition) continue;
        if (entity.kind == semantic::EntityKind::Function && entity.specialization && !entity.explicit_specialization && !(entity.emission & semantic::Entity::Used)) continue;
        if (entity.kind == semantic::EntityKind::Variable && (entity.template_info ||
            (entity.specialization && !(entity.emission & semantic::Entity::Used)))) continue;
        if (sem.dormant_hidden_friend(e)) continue;
        if (sem.static_temporary(e).object) {
            symbols[e] = fresh_symbol("@__reference_"+std::to_string(e));
            reference_objects.push_back(e); continue;
        }
        bool member = sem.scopes[entity.owner].kind == semantic::ScopeKind::Class;
        if (entity.member_info && sem.member_fact(e).virtual_member && !entity.body && !sem.synthetic_member(e) && !sem.member_fact(e).emission_reference) continue;
        if (sem.constructor_member(e)) {
            auto m = sem.member_fact(e);
            if (m.array_entry && !m.complete_entry && !m.base_entry && !m.retained_root) continue;
        }
        if (sem.scopes[entity.owner].kind != semantic::ScopeKind::Namespace && !member && !sem.local_static(e)) continue;
        if (member && entity.kind == semantic::EntityKind::Variable && !entity.is_static) continue;
        if (member && entity.kind == semantic::EntityKind::Function && sem.member_fact(e).in_class_body && !sem.member_demanded(e)) continue;
        if (member && entity.kind == semantic::EntityKind::Function && !entity.body && (!sem.member_demanded(e) || (sem.synthetic_member(e) && !sem.closure_adapter(e).function && !sem.member_fact(e).retained_root && !(sem.destructor_member(e) ? sem.destructor_needed(e) : sem.constructor_needed(e))))) continue;
        if (member && entity.kind == semantic::EntityKind::Variable && entity.constant.valid && !entity.definition) continue;
        if (entity.kind == semantic::EntityKind::Variable) symbol(e);
        if (entity.kind != semantic::EntityKind::Function || entity.template_info) continue;
        if ((sem.conversion_result(e).valid || sem.closure_adapter(e).conversion == e) && entity.inline_function &&
            !entity.instantiation_definition && !sem.member_fact(e).retained_root) {
            deferred_conversions.push_back(e); continue;
        }
        declare_function(e);
    }
    // Inherited forwarding constructors retain a distinct rooted base entry.
    // Both entries consume the same semantic actions, with independent IR IDs.
    for (EntityId e = 1; e < symbols.size(); ++e) {
        if (!symbols[e] || !separate_base(e)) continue;
        bool external = sem.emission_suppressed(e) || (!sem.entities[e].body && !sem.synthetic_member(e));
        Function f; f.symbol = symbol(e, true);
        f.declaration = external;
        auto prior = p.symbols[f.symbol.index-1];
        if (prior.kind == Symbol::FunctionSymbol) {
            if (!external && p.functions[prior.entity-1].declaration) p.functions[prior.entity-1].declaration = false;
            continue;
        }
        FunctionId id(p.functions.size()+1);
        f.signature = function_signature(e,id,true);
        if (sem.function_nonthrowing(e)) p.signatures[f.signature.index-1].boundary.unwind = CUM_NO;
        p.parameters[p.signatures[f.signature.index-1].parameters.begin].object_bytes = sem.object_size(sem.entities[sem.scopes[sem.entities[e].owner].entity].type);
        if (sem.constructor_member(e) && sem.transfer_member(e))
            for (unsigned j = 0; j < 2; ++j) p.parameters[p.signatures[f.signature.index-1].parameters.begin+j].alias = PALM_NOALIAS;
        p.functions.push_back(f);
        auto& sym = p.symbols[f.symbol.index-1]; sym.kind = Symbol::FunctionSymbol; sym.entity = id.index;
    }
    // Reserve source/native identities before allocating generated string or
    // TLS names. Native adapters may also publish the ordinary LowIR spelling.
    for (NodeId n = 1; n < ast.nodes.size(); ++n) {
        if (ast[n].kind != syntax::Kind::Literal || !sem.expression_fact(n).evaluated) continue;
        if (ast.literals[ast[n].literal].kind == LiteralKind::string) string_literal(n);
        else if (sem.literal_call_kind(n) == semantic::LiteralCallKind::Raw) numeric_string_literal(n);
    }
    // All relocation identities exist before any temporary data is emitted.
    // Preserve the established temporary-before-declaration presentation order.
    for (auto e : reference_objects) reference_global(e);
    for (EntityId e = 1; e < symbols.size(); ++e)
        if (symbols[e] && sem.entities[e].kind == semantic::EntityKind::Variable && !sem.static_temporary(e).object) global(e);
    emit_vtables();
    for (EntityId e : definitions) {
        function_body(e);
        if (base_symbols[e] && !p.functions[p.symbols[base_symbols[e].index-1].entity-1].declaration) function_body(e, true);
    }
    emit_deleting_entries();
    emit_adjustor_thunks();
    emit_allocation_adapters();
    if (!global_initializers.empty()) global_initialization();
    emit_tls_initializers();
    emit_aggregate_helpers();
    emit_local_static_destructors();
    global_finalization();
    emit_string_literals();
    // All ordinary calls, member-address constants and lifecycle bodies have
    // now requested their symbol identities. A summarized leaf needs emission
    // only when one of those consumers retained the actual function boundary.
    for (EntityId e : deferred_conversions) {
        if (!symbols[e]) continue;
        auto before = definitions.size(); declare_function(e);
        if (definitions.size() != before) function_body(e);
    }
    emit_member_thunks();
    emit_terminate_adapter();
    // The adapter can introduce runtime declarations. Publish the presentation
    // schedule only after every emission queue has finished.
    emit_source_strings();
    order_lifecycle_entries();
}
void Procedural::function_body(EntityId e, bool base)
{
    auto closure = sem.closure_adapter(e);
    if (closure.conversion == e) { closure_adapter(e); return; }
    auto body_owner = closure.function ? closure.function : e;
    if (sem.entities[body_owner].body) sem.require_body_facts(body_owner);
    function = FunctionId(p.symbols[(base ? base_symbols[e] : symbols[e]).index-1].entity);
    builder.reset(new FunctionBuilder(p, function));
    // The pointer-call entry consumes the same checked body and parameter
    // identities, with its own ABI signature and no implicit closure receiver.
    e = body_owner;
    reset_lifetime(e);
    active_base_entry = base || base_only_entry(e);
    construction_base = 0; hidden_base_addresses = semantic::Index(); vtt_argument = Value();
    returned = sem.types[sem.entities[e].type].child;
    start(block());
    Signature sig = p.signatures[p.functions[function.index-1].signature.index-1];
    unsigned j = 0; this_slot = SlotId();
    return_destination = Value();
    if (sem.indirect_value(returned)) {
        auto param = p.parameters[sig.parameters.begin+j++];
        return_destination = Value(Operand::value(param.value),IRType::Ptr,returned);
        if (auto local = sem.return_object(e)) object_addresses[local] = param.value;
    }
    if (!closure.function && sem.entities[e].member_info && !sem.entities[e].is_static) {
        auto param = p.parameters[sig.parameters.begin+j++];
        this_slot = builder->add_slot(0, IRType::Ptr);
        emit(Opcode::Store, IRType::Ptr, {Operand::value(param.value), Operand::slot(this_slot)});
    }
    for (auto d = sem.scopes[sem.entities[e].scope].first_decl; d; d = sem.declarations[d].next) {
        EntityId id = sem.declarations[d].entity;
        if (sem.entities[id].kind != semantic::EntityKind::Parameter) continue;
        auto param = p.parameters[sig.parameters.begin+j++];
        TypeId type_id = sem.entities[id].type;
        SlotId slot = builder->add_slot(0, type(type_id)); objects[id] = slot;
        if (sem.indirect_parameter(type_id)) object_addresses[id] = param.value;
        else if (type(type_id).kind() == IRType::Object) {
            if (!sem.empty_class(type_id)) {
                Value destination = address(Value(Operand::slot(slot),type(type_id),type_id,true));
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(type_id); copy.alignment = sem.object_alignment(type_id);
                emit(copy,{Operand::value(param.value),destination.operand});
            }
        } else emit(Opcode::Store, param.type, {Operand::value(param.value), Operand::slot(slot)});
    }
    for (auto d = sem.scopes[sem.entities[e].scope].first_decl; d; d = sem.declarations[d].next) {
        auto parameter = sem.declarations[d].entity;
        auto pt = sem.entities[parameter].type;
        if (sem.entities[parameter].kind != semantic::EntityKind::Parameter || !sem.class_value(pt)) continue;
        auto cls = sem.types[pt].entity;
        for (unsigned k = 0; k < sem.virtual_base_count(cls); ++k)
            parameter_base_addresses.put((std::uint64_t(parameter)<<32)|sem.virtual_base_type(cls,k),
                p.parameters[sig.parameters.begin+j++].value.index);
    }
    if (active_base_entry && (sem.constructor_member(e) || sem.destructor_member(e))) {
        auto cls = sem.scopes[sem.entities[e].owner].entity;
        if (sem.virtual_base_count(cls)) {
            vtt_argument = Value(Operand::value(p.parameters[sig.parameters.begin+j++].value),IRType::Ptr);
            for (unsigned k = 0; k < sem.virtual_base_count(cls); ++k)
                hidden_base_addresses.put(sem.virtual_base_type(cls,k),p.parameters[sig.parameters.begin+j++].value.index);
        }
    }
    if (sem.member_fact(e).inherited_constructor)
        for (auto d = sem.scopes[sem.entities[e].scope].first_decl; d; d = sem.declarations[d].next) {
            auto parameter = sem.declarations[d].entity;
            if (sem.entities[parameter].kind == semantic::EntityKind::Parameter && sem.class_value(sem.entities[parameter].type))
                activate_temporary(parameter);
        }
    bool function_try = ast[sem.entities[e].body].kind == syntax::Kind::FunctionTry;
    if (sem.transfer_member(e) && sem.synthetic_member(e)) transfer_body(e);
    else if (!function_try && sem.constructor_member(e)) constructor_body(e,active_base_entry);
    if (!function_try && sem.destructor_member(e)) {
        destructor_prologue(e);
        vpointer_store(sem.scopes[sem.entities[e].owner].entity);
    }
    mark_control_entries(sem.entities[e].body);
    constructor_block_boundary = p.block_order.size();
    statement(sem.entities[e].body);
    if (!function_try && destructor_handler) destructor_finish(e);
    if (!ended) {
        clean_inline(live,0);
        if (!function_try) finish_constructor_handlers();
        if (result_type() == IRType::Void) emit(Opcode::Return, IRType(), {});
        else if (sem.class_value(returned)) {
            exception_fallback();
        }
        else emit(Opcode::Return, result_type(), {result_type().floating() ? Operand::floating(0) : Operand::integer(0)});
    }
    emit_cleanups();
    finish_exception_boundary();
    builder.reset();
}
} }
