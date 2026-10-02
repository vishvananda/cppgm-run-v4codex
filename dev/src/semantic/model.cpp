#include "semantic/model.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
namespace {
std::uint64_t mix(std::uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
}
Types::Types() { records.push_back(Type()); hashes.push_back(0); }
TypeId Types::intern(Type t, const std::vector<TypeId>& params)
{
    auto flags = unsigned(t.kind) | (t.cv << 8) | (unsigned(t.fundamental) << 16) |
        (unsigned(t.variadic) << 24) | (unsigned(t.ref) << 25) | (unsigned(t.unknown_bound) << 27);
    // Pack disjoint 32-bit fields before mixing instead of hashing every small
    // field separately. The full canonical type comparison still resolves collisions.
    std::uint64_t hash = mix((std::uint64_t(flags) << 32) | t.child) ^
        mix((std::uint64_t(t.entity) << 32) | t.alignment_queries) ^
        mix(t.bound ^ (std::uint64_t(t.alignment)*0x9e3779b97f4a7c15ULL));
    for (TypeId p : params) hash = mix(hash ^ p);
    if (slots.empty() || records.size() * 2 >= slots.size()) {
        slots.assign(slots.empty() ? 64 : slots.size() * 2, 0);
        for (TypeId i = 1; i < records.size(); ++i) {
            std::size_t p = hashes[i] & (slots.size() - 1);
            while (slots[p]) p = (p + 1) & (slots.size() - 1);
            slots[p] = i;
        }
    }
    std::size_t p = hash & (slots.size() - 1);
    while (slots[p]) {
        ++probes;
        const Type& a = records[slots[p]];
        bool same = hashes[slots[p]] == hash && a.kind == t.kind && a.cv == t.cv &&
            a.fundamental == t.fundamental && a.child == t.child && a.entity == t.entity &&
            a.alignment == t.alignment && a.alignment_queries == t.alignment_queries && a.bound == t.bound && a.unknown_bound == t.unknown_bound && a.variadic == t.variadic && a.ref == t.ref && a.count == params.size();
        for (std::size_t i = 0; same && i < params.size(); ++i) same = parameters[a.offset + i] == params[i];
        if (same) return slots[p];
        p = (p + 1) & (slots.size() - 1);
    }
    t.offset = parameters.size(); t.count = params.size();
    parameters.insert(parameters.end(), params.begin(), params.end());
    slots[p] = records.size();
    records.push_back(t); hashes.push_back(hash);
    return slots[p];
}
TypeId Types::make_fundamental(EFundamentalType f)
{
    Type t; t.fundamental = f;
    return fundamentals[f] = intern(t, {});
}
TypeId Types::bit_integer(unsigned width, bool unsign)
{
    Type t; t.fundamental = unsign ? FT_UBITINT : FT_BITINT; t.bound = width;
    return intern(t,{});
}
TypeId Types::named(EntityId e) { Type t; t.kind = TypeKind::Named; t.entity = e; return intern(t, {}); }
TypeId Types::alias_application(EntityId alias, TypeId result, std::uint32_t arguments)
{
    Type t; t.kind = TypeKind::AliasApplication; t.entity = alias; t.child = result; t.bound = arguments;
    return intern(t,{});
}
TypeId Types::alias_target(TypeId id)
{
    if (records[id].kind != TypeKind::AliasApplication) return id;
    if (alias_targets.size() <= id) alias_targets.resize(id+1);
    if (alias_targets[id]) return alias_targets[id];
    auto t = records[id];
    auto result = qualify(alias_target(t.child),t.cv);
    alias_targets[id] = result; return result;
}
TypeId Types::pack_expansion(ArgumentId pattern, std::uint32_t captures)
{
    Type t; t.kind = TypeKind::PackExpansion; t.bound = pattern; t.entity = captures;
    return intern(t,{});
}
TypeId Types::decltype_type(std::uint32_t expression, bool direct)
{
    Type t; t.kind = TypeKind::Decltype; t.entity = expression; t.bound = direct; return intern(t,{});
}
TypeId Types::dependent_name(TypeId owner, IdentifierId name, const std::vector<TypeId>& args, DependentNameKind kind)
{
    Type t; t.kind = TypeKind::DependentName; t.child = owner; t.entity = name; t.bound = unsigned(kind);
    return intern(t,args);
}
TypeId Types::compound(TypeKind k, TypeId child, std::uint64_t bound, bool unknown_bound)
{
    Type base = records[child];
    bool ref = base.kind == TypeKind::LRef || base.kind == TypeKind::RRef;
    if ((k == TypeKind::Pointer || k == TypeKind::Array || k == TypeKind::DependentArray) && ref)
        throw std::runtime_error("pointer or array of reference");
    if ((k == TypeKind::Array || k == TypeKind::DependentArray) && (base.kind == TypeKind::Function ||
        (base.kind == TypeKind::Fundamental && base.fundamental == FT_VOID)))
        throw std::runtime_error("invalid array element");
    if ((k == TypeKind::LRef || k == TypeKind::RRef) && ref) {
        if (k == TypeKind::LRef || base.kind == TypeKind::LRef) k = TypeKind::LRef;
        child = base.child;
    }
    Type t; t.kind = k; t.child = child; t.bound = bound; t.unknown_bound = unknown_bound;
    if (k == TypeKind::Array || k == TypeKind::DependentArray) { t.alignment = base.alignment; t.alignment_queries = base.alignment_queries; }
    return intern(t, {});
}
TypeId Types::aligned(TypeId id, std::uint64_t bytes, std::uint32_t queries)
{
    auto t = records[id]; t.alignment = bytes ? 1 : 0; t.alignment_queries = queries;
    while (bytes > 1) { ++t.alignment; bytes >>= 1; }
    return intern(t,std::vector<TypeId>(parameters.begin()+t.offset,parameters.begin()+t.offset+t.count));
}
unsigned Types::storage_alignment(TypeId id) const { return records[id].alignment; }
TypeId Types::qualify(TypeId id, unsigned cv)
{
    Type t = records[id];
    if (!cv || t.kind == TypeKind::LRef || t.kind == TypeKind::RRef || t.kind == TypeKind::Function) return id;
    if ((t.cv & cv) == cv) return id;
    if (t.kind == TypeKind::Array || t.kind == TypeKind::DependentArray) return aligned(compound(t.kind, qualify(t.child, cv), t.bound,t.unknown_bound),t.alignment ? std::uint64_t(1) << (t.alignment-1) : 0,t.alignment_queries);
    t.cv |= cv;
    if (t.kind == TypeKind::DependentName)
        return intern(t,std::vector<TypeId>(parameters.begin()+t.offset,parameters.begin()+t.offset+t.count));
    return intern(t, {});
}
TypeId Types::unqualified(TypeId id)
{
    Type t = records[id];
    if (t.kind == TypeKind::Array || t.kind == TypeKind::DependentArray)
        return aligned(compound(t.kind,unqualified(t.child),t.bound,t.unknown_bound),t.alignment ? std::uint64_t(1) << (t.alignment-1) : 0,t.alignment_queries);
    // Member-function cv is part of its signature, not top-level object cv.
    if (t.kind == TypeKind::Function) return id;
    if (!(t.cv & 3)) return id;
    t.cv &= 4;
    if (t.kind == TypeKind::DependentName)
        return intern(t,std::vector<TypeId>(parameters.begin()+t.offset,parameters.begin()+t.offset+t.count));
    return intern(t, {});
}
TypeId Types::non_atomic(TypeId id)
{
    auto t = records[id];
    if (!(t.cv & 4)) return id;
    t.cv &= ~4u;
    return intern(t,std::vector<TypeId>(parameters.begin()+t.offset,parameters.begin()+t.offset+t.count));
}
TypeId Types::function(TypeId result, const std::vector<TypeId>& params, bool variadic, unsigned cv, RefQualifier ref)
{
    if (records[result].kind == TypeKind::Array || records[result].kind == TypeKind::DependentArray || records[result].kind == TypeKind::Function)
        throw std::runtime_error("invalid function return type");
    Type t; t.kind = TypeKind::Function; t.child = result; t.variadic = variadic; t.cv = cv; t.ref = ref;
    return intern(t, params);
}
TypeId Types::member_pointer(EntityId owner, TypeId child)
{
    return member_pointer_type(named(owner),child);
}
TypeId Types::member_pointer_type(TypeId owner, TypeId child)
{
    owner = unqualified(signature(owner));
    Type t; t.kind = TypeKind::MemberPointer; t.bound = owner; t.child = child;
    t.entity = records[owner].kind == TypeKind::Named ? records[owner].entity : 0;
    return intern(t, {});
}
TypeId Types::adjusted(TypeId id)
{
    if (adjustments.size() <= id) adjustments.resize(id + 1);
    if (adjustments[id]) return adjustments[id];
    Type t = records[id];
    TypeId result = t.kind == TypeKind::AliasApplication ? alias_application(t.entity,adjusted(qualify(t.child,t.cv)),t.bound) :
        t.kind == TypeKind::PackExpansion ? pack_expansion(adjusted(t.bound),t.entity) :
        t.kind == TypeKind::Array || t.kind == TypeKind::DependentArray ? compound(TypeKind::Pointer, signature(t.child)) :
        t.kind == TypeKind::Function ? compound(TypeKind::Pointer, signature(id)) : unqualified(signature(id));
    adjustments[id] = result;
    return result;
}
TypeId Types::signature(TypeId id)
{
    if (signatures.size() <= id) signatures.resize(id + 1);
    if (signatures[id]) return signatures[id];
    ++signature_work;
    Type t = records[id];
    if (t.alignment || t.alignment_queries) {
        t.alignment = 0; t.alignment_queries = 0;
        auto result = signature(intern(t,std::vector<TypeId>(parameters.begin()+t.offset,parameters.begin()+t.offset+t.count))); signatures[id] = result; return result;
    }
    if (t.kind == TypeKind::Function) {
        std::vector<TypeId> source(parameters.begin() + t.offset, parameters.begin() + t.offset + t.count);
        for (TypeId& p : source) p = adjusted(p);
        TypeId result = function(signature(t.child), source, t.variadic, t.cv, t.ref);
        signatures[id] = result;
        return result;
    }
    TypeId result = id;
    if (t.kind == TypeKind::Pointer || t.kind == TypeKind::BlockPointer || t.kind == TypeKind::LRef || t.kind == TypeKind::RRef || t.kind == TypeKind::Array || t.kind == TypeKind::DependentArray || vector_kind(t.kind) || dependent_vector_kind(t.kind))
        result = qualify(compound(t.kind, signature(t.child), t.bound,t.unknown_bound), t.cv);
    if (t.kind == TypeKind::MemberPointer) result = qualify(member_pointer_type(t.member_owner(), signature(t.child)), t.cv);
    signatures[id] = result;
    return result;
}
TypeId Types::composite(TypeId a, TypeId b)
{
    if (a == b) return a;
    Type x = records[a], y = records[b];
    if (x.kind == TypeKind::Array && y.kind == TypeKind::Array && (x.unknown_bound || y.unknown_bound || x.bound == y.bound))
        return compound(TypeKind::Array, composite(x.child, y.child), x.unknown_bound ? y.bound : x.bound,x.unknown_bound && y.unknown_bound);
    throw std::runtime_error("incompatible redeclaration");
}
} }
