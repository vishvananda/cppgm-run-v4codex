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
    std::uint64_t hash = mix(unsigned(t.kind) | (t.cv << 8) | (unsigned(t.fundamental) << 16) |
                             (unsigned(t.variadic) << 24) | (unsigned(t.ref) << 25));
    hash ^= mix(t.child) ^ mix(std::uint64_t(t.entity) << 32) ^ mix(t.bound);
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
            a.bound == t.bound && a.variadic == t.variadic && a.ref == t.ref && a.count == params.size();
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
TypeId Types::fundamental(EFundamentalType f) { Type t; t.fundamental = f; return intern(t, {}); }
TypeId Types::named(EntityId e) { Type t; t.kind = TypeKind::Named; t.entity = e; return intern(t, {}); }
TypeId Types::compound(TypeKind k, TypeId child, std::uint64_t bound)
{
    Type base = records[child];
    bool ref = base.kind == TypeKind::LRef || base.kind == TypeKind::RRef;
    if ((k == TypeKind::Pointer || k == TypeKind::Array) && ref)
        throw std::runtime_error("pointer or array of reference");
    if (k == TypeKind::Array && (base.kind == TypeKind::Function ||
        (base.kind == TypeKind::Fundamental && base.fundamental == FT_VOID)))
        throw std::runtime_error("invalid array element");
    if ((k == TypeKind::LRef || k == TypeKind::RRef) && ref) {
        if (k == TypeKind::LRef || base.kind == TypeKind::LRef) k = TypeKind::LRef;
        child = base.child;
    }
    Type t; t.kind = k; t.child = child; t.bound = bound;
    return intern(t, {});
}
TypeId Types::qualify(TypeId id, unsigned cv)
{
    Type t = records[id];
    if (!cv || t.kind == TypeKind::LRef || t.kind == TypeKind::RRef || t.kind == TypeKind::Function) return id;
    if (t.kind == TypeKind::Array) return compound(t.kind, qualify(t.child, cv), t.bound);
    t.cv |= cv;
    return intern(t, {});
}
TypeId Types::unqualified(TypeId id)
{
    Type t = records[id];
    if (!t.cv) return id;
    t.cv = 0;
    return intern(t, {});
}
TypeId Types::function(TypeId result, const std::vector<TypeId>& params, bool variadic, unsigned cv, RefQualifier ref)
{
    if (records[result].kind == TypeKind::Array || records[result].kind == TypeKind::Function)
        throw std::runtime_error("invalid function return type");
    Type t; t.kind = TypeKind::Function; t.child = result; t.variadic = variadic; t.cv = cv; t.ref = ref;
    return intern(t, params);
}
TypeId Types::member_pointer(EntityId owner, TypeId child)
{
    Type t; t.kind = TypeKind::MemberPointer; t.entity = owner; t.child = child;
    return intern(t, {});
}
TypeId Types::adjusted(TypeId id)
{
    if (adjustments.size() <= id) adjustments.resize(id + 1);
    if (adjustments[id]) return adjustments[id];
    Type t = records[id];
    TypeId result = t.kind == TypeKind::Array ? compound(TypeKind::Pointer, signature(t.child)) :
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
    if (t.kind == TypeKind::Function) {
        std::vector<TypeId> source(parameters.begin() + t.offset, parameters.begin() + t.offset + t.count);
        for (TypeId& p : source) p = adjusted(p);
        TypeId result = function(signature(t.child), source, t.variadic, t.cv, t.ref);
        signatures[id] = result;
        return result;
    }
    TypeId result = id;
    if (t.kind == TypeKind::Pointer || t.kind == TypeKind::LRef || t.kind == TypeKind::RRef || t.kind == TypeKind::Array)
        result = qualify(compound(t.kind, signature(t.child), t.bound), t.cv);
    if (t.kind == TypeKind::MemberPointer) result = qualify(member_pointer(t.entity, signature(t.child)), t.cv);
    signatures[id] = result;
    return result;
}
TypeId Types::composite(TypeId a, TypeId b)
{
    if (a == b) return a;
    Type x = records[a], y = records[b];
    if (x.kind == TypeKind::Array && y.kind == TypeKind::Array && (!x.bound || !y.bound || x.bound == y.bound))
        return compound(TypeKind::Array, composite(x.child, y.child), x.bound ? x.bound : y.bound);
    throw std::runtime_error("incompatible redeclaration");
}
} }
