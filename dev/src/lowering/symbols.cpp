#include "lowering/procedural.h"
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
    auto parent = abi_scope(scope.parent);
    return abi_scopes[s] = abi.name(parent, scope.name ? spelling(scope.name) : "_GLOBAL__N_1");
}
abi_mangle::Id Procedural::abi_type(TypeId id)
{
    using namespace abi_mangle;
    if (id >= abi_types.size()) abi_types.resize(sem.types.records.size());
    if (abi_types[id]) return abi_types[id];
    auto t = sem.types[id];
    abi_mangle::Id result = 0;
    if (t.cv) result = abi.cv(abi_type(sem.types.unqualified(id)), t.cv);
    else switch (t.kind) {
    case TypeKind::Fundamental: {
        static const AbiBuiltinTypeKind kinds[] = {ABI_BUILTIN_TYPE_SIGNED_CHAR, ABI_BUILTIN_TYPE_SHORT,
            ABI_BUILTIN_TYPE_INT, ABI_BUILTIN_TYPE_LONG, ABI_BUILTIN_TYPE_LONG_LONG,
            ABI_BUILTIN_TYPE_UNSIGNED_CHAR, ABI_BUILTIN_TYPE_UNSIGNED_SHORT, ABI_BUILTIN_TYPE_UNSIGNED_INT,
            ABI_BUILTIN_TYPE_UNSIGNED_LONG, ABI_BUILTIN_TYPE_UNSIGNED_LONG_LONG, ABI_BUILTIN_TYPE_WCHAR,
            ABI_BUILTIN_TYPE_CHAR, ABI_BUILTIN_TYPE_CHAR16, ABI_BUILTIN_TYPE_CHAR32, ABI_BUILTIN_TYPE_BOOL,
            ABI_BUILTIN_TYPE_FLOAT, ABI_BUILTIN_TYPE_DOUBLE, ABI_BUILTIN_TYPE_LONG_DOUBLE, ABI_BUILTIN_TYPE_VOID, ABI_BUILTIN_TYPE_NULLPTR};
        result = abi.builtin(kinds[t.fundamental]); break;
    }
    case TypeKind::Named: {
        auto e = sem.entities[t.entity]; result = abi.name(abi_scope(e.owner), spelling(e.name)); break;
    }
    case TypeKind::Pointer: result = abi.make(abi_mangle::Kind::Pointer, abi_type(t.child)); break;
    case TypeKind::LRef: result = abi.make(abi_mangle::Kind::Reference, abi_type(t.child)); break;
    case TypeKind::RRef: result = abi.make(abi_mangle::Kind::RvalueReference, abi_type(t.child)); break;
    case TypeKind::Array: result = abi.make(abi_mangle::Kind::Array, abi_type(t.child), 0, 0, t.bound); break;
    case TypeKind::Function: {
        std::vector<abi_mangle::Id> params;
        for (unsigned j = 0; j < t.count; ++j) params.push_back(abi_type(sem.types.parameters[t.offset+j]));
        result = abi.make(abi_mangle::Kind::FunctionType, abi_type(t.child), 0, t.variadic, 0, params); break;
    }
    default: throw std::runtime_error("unsupported procedural ABI type");
    }
    if (id >= abi_types.size()) abi_types.resize(sem.types.records.size());
    return abi_types[id] = result;
}
bool Procedural::separate_base(EntityId id) const
{
    auto e = sem.entities[id];
    if (e.member_info && sem.member_fact(id).defaulted_late && (sem.constructor_member(id) || sem.destructor_member(id))) return true;
    if (!e.member_info || !sem.member_fact(id).base_entry) return false;
    return sem.member_fact(id).complete_entry || (e.body && !e.inline_function);
}
SymbolId Procedural::symbol(EntityId id, bool base)
{
    auto e = sem.entities[id];
    bool external = e.member_info && !e.body && !sem.synthetic_member(id);
    bool separate = separate_base(id);
    bool base_only = e.member_info && (external || sem.member_fact(id).inherited_constructor) &&
        sem.member_fact(id).base_entry && !sem.member_fact(id).complete_entry;
    base = base && separate;
    if ((base ? base_symbols[id] : symbols[id])) return base ? base_symbols[id] : symbols[id];
    bool internal = (e.is_static && sem.scopes[e.owner].kind != semantic::ScopeKind::Class) || (e.kind == semantic::EntityKind::Variable &&
        sem.types[e.type].cv & 1 && !e.external_decl);
    for (auto s = e.owner; s && s != sem.global; s = sem.scopes[s].parent)
        if (sem.scopes[s].kind == semantic::ScopeKind::Namespace && !sem.scopes[s].name) internal = true;
    std::string name = spelling(e.name);
    // Source ABI spelling and internal IR identity occupy separate namespaces.
    // In particular a root C++ variable's ABI spelling is the bare source name.
    bool entry = name == "main" && e.owner == sem.global && e.kind == semantic::EntityKind::Function;
    SymbolMetadata metadata;
    metadata.binding = internal ? SBM_INTERNAL : e.inline_function ? SBM_WEAK : SBM_STRONG;
    metadata.inline_hint = e.inline_function; metadata.no_inline = e.no_inline; metadata.force_inline = e.force_inline && !e.no_inline;
    if (e.member_info) metadata.object_root = base || (!separate && !external && sem.member_fact(id).base_entry);
    if (e.member_info) {
        metadata.object_root |= sem.member_fact(id).retained_root;
        for (auto s = e.owner; s && s != sem.global; s = sem.scopes[s].parent)
            if (sem.scopes[s].kind == semantic::ScopeKind::Function) { metadata.object_root = true; metadata.binding = SBM_INTERNAL; }
    }
    if (e.c_linkage) metadata.linkage = LLM_C;
    if (e.thread_local_storage) metadata.storage = GSM_THREAD_LOCAL;
    else if (e.kind == semantic::EntityKind::Variable && (sem.types[e.type].cv & 3) == 1 && type(e.type).scalar() && !reference(e.type)) metadata.storage = GSM_READONLY;
    abi_mangle::Target target;
    auto aname = abi.name(abi_scope(e.owner), name);
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
        target.function.c_linkage = e.c_linkage && !internal;
        auto t = sem.types[e.type]; target.function.variadic = t.variadic;
        for (unsigned j = 0; j < t.count; ++j) target.function.parameters.push_back(abi_type(sem.types.parameters[t.offset+j]));
    } else { target.kind = abi_mangle::TargetKind::Variable; target.type = aname; target.internal = internal; }
    std::uint64_t key = 0;
    if (!internal && (linkage.merge || e.c_linkage)) {
        ++linkage.requests;
        if (e.c_linkage || entry || e.builtin != semantic::Entity::NoBuiltin)
            key = (std::uint64_t(3) << 32) | abi.name(0, name);
        else if (e.kind == semantic::EntityKind::Function)
            key = (std::uint64_t(1) << 32) | abi_mangle::function_entity(abi, target.function);
        else key = (std::uint64_t(2) << 32) | aname;
            if (auto previous = linkage.external.get(key)) { ++linkage.hits; return (base ? base_symbols[id] : symbols[id]) = SymbolId(previous); }
    }
    if (entry) {
        metadata.role = SR_ENTRY; metadata.keep_alias = true;
    } else metadata.object = p.intern(e.c_linkage && !internal && e.kind != semantic::EntityKind::Function ? name : abi_mangle::mangle(abi, target));
    // Several source TUs are emitted as one LowIR program/object. Its local
    // object labels need the same isolation as their internal SymbolIds.
    if (internal && linkage.merge && metadata.object)
        metadata.object = p.intern(p.name(metadata.object) + "." + std::to_string(p.symbols.size()+1));
    if (e.builtin != semantic::Entity::NoBuiltin) {
        metadata.object = p.intern(e.builtin == semantic::Entity::Memcpy ? "cppgm_builtin_memcpy" : e.builtin == semantic::Entity::Strlen ? "cppgm_builtin_strlen" : "cppgm_builtin_memmove");
        metadata.linkage = LLM_C;
    }
    SymbolId allocation_runtime; unsigned allocation_role = 2;
    if (e.allocation_runtime && !e.definition) {
        const char* runtime[] = {"", "cppgm_builtin_operator_new", "cppgm_builtin_operator_new_array", "cppgm_builtin_operator_delete", "cppgm_builtin_operator_delete_array"};
        metadata.object = p.intern(runtime[e.allocation_runtime]);
        allocation_role = e.key == KW_NEW ? 0 : 1;
        allocation_runtime = linkage.allocation_roles[allocation_role];
        if (!allocation_runtime) {
            metadata.role = e.key == KW_NEW ? SR_ALLOCATE_MEMORY : SR_FREE_MEMORY;
        }
    }
    std::string display = e.kind == semantic::EntityKind::Variable ? "@__global_" + name : "@" + name;
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
    if (!separate && !base_only && (sem.constructor_member(id) || sem.destructor_member(id)) && (e.body || sem.synthetic_member(id))) {
        target.function.terminal = sem.destructor_member(id) ? abi_mangle::ABI_TERMINAL_DESTRUCTOR_BASE : abi_mangle::ABI_TERMINAL_CONSTRUCTOR_BASE;
        ObjectAlias alias; alias.name = p.intern(abi_mangle::mangle(abi, target)); alias.target = sid; p.aliases.push_back(alias);
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
    auto return_type = sem.types[t.child];
    bool incomplete_result = return_type.kind == TypeKind::Named && sem.entities[return_type.entity].class_info && !sem.entities[return_type.entity].complete;
    bool indirect_result = sem.indirect_value(t.child);
    Signature sig; sig.result = incomplete_result || indirect_result ? IRType(IRType::Void) : type(t.child); sig.parameters.begin = p.parameters.size();
    sig.parameters.count = t.count + indirect_result;
    if (t.variadic) sig.boundary.arity = CAM_VARIADIC;
    if (indirect_result) {
        Parameter param; param.type = IRType::Ptr; param.passing = PPM_INDIRECT_RESULT; param.object_bytes = sem.object_size(t.child);
        lowir_model::Value v; v.type = param.type; v.owner = owner; v.defined = true;
        if (!owner) v.name = p.intern("%ret");
        p.values.push_back(v); param.value = ValueId(p.values.size()); p.parameters.push_back(param);
    }
    for (unsigned j = 0; j < t.count; ++j) {
        TypeId pt = sem.types.parameters[t.offset+j];
        Parameter param; param.type = sem.indirect_parameter(pt) ? IRType(IRType::Ptr) : type(pt);
        lowir_model::Value v; v.type = param.type; v.owner = owner; v.defined = true;
        if (!owner) v.name = p.intern("%arg" + std::to_string(j));
        p.values.push_back(v); param.value = ValueId(p.values.size());
        if (sem.indirect_parameter(pt)) { param.passing = PPM_BY_ADDRESS; param.object_bytes = sem.object_size(pt); }
        else if (reference(pt)) {
            param.passing = PPM_BY_ADDRESS;
            auto referred = sem.types[pt].child;
            if (sem.types[referred].kind != TypeKind::Function &&
                !(sem.types[referred].kind == TypeKind::Named && !sem.entities[sem.types[referred].entity].complete) &&
                !(sem.types[referred].kind == TypeKind::Array && !sem.types[referred].bound)) param.object_bytes = sem.object_size(referred);
        }
        p.parameters.push_back(param);
    }
    p.signatures.push_back(sig);
    SignatureId result(p.signatures.size());
    if (!owner) indirect_signatures[id] = result;
    return result;
}
Procedural::Procedural(syntax::Ast& a, semantic::Analyzer& s, IdentifierTable& ids, Program& out, Linkage& links)
    : ast(a), sem(s), identifiers(ids), p(out), linkage(links), abi(links.abi), abi_types(s.types.records.size()), abi_scopes(s.scopes.size()),
      symbols(s.entities.size()), strings(a.nodes.size()), base_symbols(s.entities.size()), objects(s.entities.size()), object_addresses(s.entities.size()), labels(a.nodes.size()), control_entries(a.nodes.size()) {}
void Procedural::run()
{
    for (EntityId e = 1; e < sem.entities.size(); ++e) {
        auto entity = sem.entities[e];
        if (sem.static_temporary(e).object) { reference_global(e); continue; }
        bool member = sem.scopes[entity.owner].kind == semantic::ScopeKind::Class;
        if (sem.constructor_member(e)) {
            auto m = sem.member_fact(e);
            if (m.array_entry && !m.complete_entry && !m.base_entry && !m.retained_root) continue;
        }
        if (sem.scopes[entity.owner].kind != semantic::ScopeKind::Namespace && !member) continue;
        if (member && entity.kind == semantic::EntityKind::Variable && !entity.is_static) continue;
        if (member && entity.kind == semantic::EntityKind::Function && !entity.body && (!sem.member_demanded(e) || (sem.synthetic_member(e) && !sem.member_fact(e).retained_root && !(sem.destructor_member(e) ? sem.destructor_needed(e) : sem.constructor_needed(e))))) continue;
        if (member && entity.kind == semantic::EntityKind::Variable && entity.constant.valid && !entity.definition) continue;
        if (entity.kind == semantic::EntityKind::Variable) symbol(e);
        if (entity.kind != semantic::EntityKind::Function) continue;
        bool defined = entity.body || ((sem.constructor_member(e) || sem.destructor_member(e) || sem.transfer_member(e)) && sem.synthetic_member(e));
        Function f; f.symbol = symbol(e); f.declaration = !defined;
        auto& existing = p.symbols[f.symbol.index-1];
        if (existing.kind == Symbol::FunctionSymbol) {
            if (!entity.body) continue;
            auto& prior = p.functions[existing.entity-1];
            if (!prior.declaration) {
                if (entity.inline_function) continue;
                throw std::runtime_error("multiple function definitions");
            }
            prior.declaration = false;
            prior.signature = signature(sem.call_type(e), FunctionId(existing.entity));
            definitions.push_back(e);
            continue;
        }
        FunctionId id(p.functions.size()+1);
        f.signature = signature(sem.call_type(e), id);
        if (entity.stable_prefix) p.signatures[f.signature.index-1].boundary.query = CQM_STABLE_PREFIX;
        if (sem.function_nonthrowing(e)) p.signatures[f.signature.index-1].boundary.unwind = CUM_NO;
        if (entity.member_info && !entity.is_static)
            p.parameters[p.signatures[f.signature.index-1].parameters.begin + sem.indirect_value(sem.types[entity.type].child)].object_bytes = sem.object_size(sem.entities[sem.scopes[entity.owner].entity].type);
        if (sem.constructor_member(e) && sem.transfer_member(e))
            for (unsigned j = 0; j < 2; ++j) p.parameters[p.signatures[f.signature.index-1].parameters.begin+j].alias = PALM_NOALIAS;
        if (entity.builtin != semantic::Entity::NoBuiltin) {
            auto& sig = p.signatures[f.signature.index-1]; sig.boundary.unwind = CUM_NO;
            if (entity.builtin == semantic::Entity::Strlen) sig.boundary.effects = CFXM_READONLY;
            if (entity.builtin == semantic::Entity::Memcpy)
                for (unsigned j = 0; j < 2; ++j) p.parameters[sig.parameters.begin+j].alias = PALM_NOALIAS;
        }
        p.functions.push_back(f);
        auto& sym = p.symbols[f.symbol.index-1]; sym.kind = Symbol::FunctionSymbol; sym.entity = id.index;
        if (defined) definitions.push_back(e);
    }
    // Inherited forwarding constructors retain a distinct rooted base entry.
    // Both entries consume the same semantic actions, with independent IR IDs.
    for (EntityId e = 1; e < sem.entities.size(); ++e) {
        if (!symbols[e] || !separate_base(e)) continue;
        bool external = !sem.entities[e].body && !sem.synthetic_member(e);
        Function f; f.symbol = symbol(e, true);
        f.declaration = external;
        FunctionId id(p.functions.size()+1);
        f.signature = signature(sem.call_type(e), id);
        if (sem.function_nonthrowing(e)) p.signatures[f.signature.index-1].boundary.unwind = CUM_NO;
        p.parameters[p.signatures[f.signature.index-1].parameters.begin].object_bytes = sem.object_size(sem.entities[sem.scopes[sem.entities[e].owner].entity].type);
        if (sem.constructor_member(e) && sem.transfer_member(e))
            for (unsigned j = 0; j < 2; ++j) p.parameters[p.signatures[f.signature.index-1].parameters.begin+j].alias = PALM_NOALIAS;
        p.functions.push_back(f);
        auto& sym = p.symbols[f.symbol.index-1]; sym.kind = Symbol::FunctionSymbol; sym.entity = id.index;
    }
    // Reserve source/native identities before allocating generated string or
    // TLS names. Native adapters may also publish the ordinary LowIR spelling.
    for (NodeId n = 1; n < ast.nodes.size(); ++n)
        if (ast[n].kind == syntax::Kind::Literal && ast.literals[ast[n].literal].kind == LiteralKind::string && sem.expression_fact(n).evaluated)
            string_literal(n);
    for (EntityId e = 1; e < sem.entities.size(); ++e)
        if (symbols[e] && sem.entities[e].kind == semantic::EntityKind::Variable && !sem.static_temporary(e).object) global(e);
    for (EntityId e : definitions) {
        function_body(e);
        if (base_symbols[e]) function_body(e, true);
    }
    emit_allocation_adapters();
    if (!global_initializers.empty()) global_initialization();
    emit_tls_initializers();
    emit_aggregate_helpers();
    global_finalization();
}
void Procedural::function_body(EntityId e, bool base)
{
    function = FunctionId(p.symbols[(base ? base_symbols[e] : symbols[e]).index-1].entity);
    builder.reset(new FunctionBuilder(p, function));
    reset_lifetime(e);
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
    if (sem.entities[e].member_info && !sem.entities[e].is_static) {
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
        else if (sem.class_value(type_id)) {
            if (!sem.empty_class(type_id)) {
                Value destination = address(Value(Operand::slot(slot),type(type_id),type_id,true));
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(type_id); copy.alignment = sem.object_alignment(type_id);
                emit(copy,{Operand::value(param.value),destination.operand});
            }
        } else emit(Opcode::Store, param.type, {Operand::value(param.value), Operand::slot(slot)});
    }
    if (sem.transfer_member(e) && sem.synthetic_member(e)) transfer_body(e);
    else if (sem.constructor_member(e)) constructor_body(e);
    if (sem.destructor_member(e)) destructor_prologue(e);
    mark_control_entries(sem.entities[e].body);
    statement(sem.entities[e].body);
    if (destructor_handler) destructor_finish(e);
    if (!ended) {
        clean_inline(live,0);
        finish_constructor_handlers();
        if (result_type() == IRType::Void) emit(Opcode::Return, IRType(), {});
        else if (sem.class_value(returned)) emit(Opcode::Unreachable,IRType(),{});
        else emit(Opcode::Return, result_type(), {result_type().floating() ? Operand::floating(0) : Operand::integer(0)});
    }
    emit_cleanups();
    builder.reset();
}
} }
