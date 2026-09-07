#pragma once
#include "syntax/ast.h"
#include <cstdint>
#include <vector>

namespace cppgm { namespace semantic {
using syntax::NodeId;
typedef std::uint32_t TypeId;
typedef std::uint32_t EntityId;
typedef std::uint32_t ScopeId;

// Translation-unit owned, open-addressed indexes; no allocation per binding.
class Index {
    struct Slot { std::uint64_t key = 0; std::uint32_t value = 0; };
    std::vector<Slot> slots;
    std::size_t used = 0;
public:
    std::uint32_t get(std::uint64_t key) const;
    void put(std::uint64_t key, std::uint32_t value);
};

enum class TypeKind : unsigned char { Fundamental, Named, Pointer, LRef, RRef, Array, Function };
struct Type {
    TypeKind kind = TypeKind::Fundamental;
    unsigned char cv = 0;
    EFundamentalType fundamental = FT_INT;
    bool variadic = false;
    TypeId child = 0;
    EntityId entity = 0;
    std::uint64_t bound = 0;
    std::uint32_t offset = 0, count = 0;
};
class Types {
    std::vector<TypeId> slots;
    std::vector<std::uint64_t> hashes;
    std::vector<TypeId> signatures, adjustments;
    TypeId intern(Type type, const std::vector<TypeId>& params);
public:
    Types();
    std::vector<Type> records;
    std::vector<TypeId> parameters;
    std::size_t probes = 0, signature_work = 0;
    TypeId fundamental(EFundamentalType f);
    TypeId named(EntityId e);
    TypeId compound(TypeKind k, TypeId child, std::uint64_t bound = 0);
    TypeId qualify(TypeId t, unsigned cv);
    TypeId unqualified(TypeId t);
    TypeId function(TypeId result, const std::vector<TypeId>& params, bool variadic);
    TypeId adjusted(TypeId t);
    TypeId signature(TypeId t);
    TypeId composite(TypeId a, TypeId b);
    const Type& operator[](TypeId t) const { return records[t]; }
};

enum class EntityKind : unsigned char { Type, Alias, Namespace, NamespaceAlias, Variable, Function, Parameter, Enumerator };
enum class ScopeKind : unsigned char { Namespace, Class, Enum, Template, Function, Block };
struct Constant {
    TypeId type = 0;
    std::uint64_t bits = 0;
    bool valid = false;
    Constant() {}
    Constant(TypeId t, std::uint64_t b) : type(t), bits(b), valid(true) {}
};
struct Entity {
    EntityKind kind = EntityKind::Variable;
    IdentifierId name = 0;
    ScopeId owner = 0, scope = 0, default_constructor = 0;
    EntityId constructor = 0;
    std::uint64_t size = 0, alignment = 0;
    unsigned char layout_state = 0;
    bool is_static = false;
    NodeId source = 0;
    TypeId type = 0, underlying = 0;
    ETokenType key = TOK_INVALID;
    Constant constant;
    bool complete = false, scoped = false, template_parameter = false;
};
struct Scope {
    ScopeKind kind = ScopeKind::Namespace;
    ScopeId jump = 0;
    std::uint32_t depth = 0;
    ScopeId parent = 0, first_child = 0, last_child = 0, next = 0;
    EntityId entity = 0;
    IdentifierId name = 0;
    NodeId display_name = 0;
    std::uint32_t first_decl = 0, last_decl = 0, first_edge = 0;
};
struct Declaration {
    EntityId entity = 0;
    EntityKind kind = EntityKind::Variable;
    NodeId source = 0, display_name = 0;
    TypeId type = 0; // Source view; entity holds completed/adjusted semantic type.
    ETokenType key = TOK_INVALID;
    std::uint32_t next = 0;
};
struct Edge { ScopeId target = 0; std::uint32_t next = 0; bool inline_namespace = false; };
struct Fact { TypeId type = 0; EntityId entity = 0; ScopeId scope = 0; std::uint32_t value = 0; };

} }
