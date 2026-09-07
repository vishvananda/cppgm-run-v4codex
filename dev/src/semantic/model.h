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

enum class TypeKind : unsigned char { Fundamental, Named, Pointer, LRef, RRef, Array, Function, MemberPointer };
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
    TypeId function(TypeId result, const std::vector<TypeId>& params, bool variadic, unsigned cv = 0);
    TypeId member_pointer(EntityId owner, TypeId child);
    TypeId adjusted(TypeId t);
    TypeId signature(TypeId t);
    TypeId composite(TypeId a, TypeId b);
    const Type& operator[](TypeId t) const { return records[t]; }
};

enum class EntityKind : unsigned char { Type, Alias, Namespace, NamespaceAlias, Variable, Function, Parameter, Enumerator, Overload };
enum class ScopeKind : unsigned char { Namespace, Class, Enum, Template, Function, Block };
struct Constant {
    std::uint64_t bits = 0;
    TypeId type = 0;
    bool valid = false;
    Constant() {}
    Constant(TypeId t, std::uint64_t b) : bits(b), type(t), valid(true) {}
};
// Rare class demand state has a separate arena; ordinary bindings do not pay
// for constructors and layout. The stable index belongs to the class entity.
struct ClassFacts {
    std::uint64_t size = 0, alignment = 0;
    EntityId constructor = 0, implicit_constructor = 0, storage = 0;
    std::uint32_t first_base = 0;
    ScopeId default_constructor = 0;
    unsigned char layout_state = 0;
};
struct Entity {
    EntityKind kind = EntityKind::Variable;
    ETokenType key = TOK_INVALID;
    bool complete = false, scoped = false, template_parameter = false, is_static = false;
    IdentifierId name = 0;
    ScopeId owner = 0, scope = 0;
    NodeId source = 0, definition = 0;
    TypeId type = 0, underlying = 0;
    std::uint32_t class_info = 0, member_info = 0, template_info = 0, specialization = 0;
    EntityId first = 0, second = 0; // Immutable overload union edges.
    Constant constant;
};
enum class DemandState : unsigned char { Dormant, Queued, Active, Complete };
struct MemberFacts {
    TypeId call_type = 0;
    NodeId body = 0, declarator = 0, source = 0;
    DemandState demand = DemandState::Dormant;
    bool synthetic = false;
};
struct TypeArguments { std::uint32_t offset = 0, count = 0; std::uint64_t hash = 0; };
struct TemplateFunction {
    ScopeId environment = 0;
    std::uint32_t offset = 0, count = 0;
    NodeId body = 0, declarator = 0, source = 0;
};
enum class FactState : unsigned char { NotStarted, Active, Success, Failure };
struct Specialization {
    EntityId pattern = 0, entity = 0;
    std::uint32_t arguments = 0;
    FactState declaration = FactState::NotStarted;
    bool emission_demanded = false;
};
struct BaseRelation { EntityId base; std::uint32_t next; };
struct ObjectAction { EntityId object, constructor; TypeId address_type; };
struct Scope {
    ScopeKind kind = ScopeKind::Namespace;
    ScopeId jump = 0;
    std::uint32_t depth = 0;
    ScopeId parent = 0, first_child = 0, last_child = 0, next = 0;
    EntityId entity = 0;
    IdentifierId name = 0;
    NodeId display_name = 0;
    std::uint32_t first_decl = 0, last_decl = 0, first_edge = 0, first_inline = 0;
};
struct Declaration {
    EntityId entity = 0;
    EntityKind kind = EntityKind::Variable;
    NodeId source = 0, display_name = 0;
    TypeId type = 0; // Source view; entity holds completed/adjusted semantic type.
    ETokenType key = TOK_INVALID;
    std::uint32_t next = 0;
};
struct Edge { ScopeId target = 0; std::uint32_t next = 0, inline_next = 0; bool inline_namespace = false; };
enum class ValueCategory : unsigned char { Prvalue, Lvalue, Xvalue };
enum class ExpressionForm : unsigned char { Ordinary, Overload, Cast, ConstantQuery, Abort };
struct Expression {
    TypeId type = 0; // Reference-free language expression type.
    EntityId entity = 0;
    std::uint32_t conversions = 0, count = 0, incoming = 0;
    ValueCategory category = ValueCategory::Prvalue;
    ExpressionForm form = ExpressionForm::Ordinary;
    bool ready = false;
};
struct Conversion {
    TypeId target = 0;
    EntityId function = 0; // Target-selected overload, if any.
    unsigned char rank = 255, qualification = 0;
    bool reference = false, temporary = false, derived = false;
    unsigned char preference = 0;
    bool valid() const { return rank != 255; }
};
struct Fact { TypeId type = 0; EntityId entity = 0; ScopeId scope = 0; std::uint32_t value = 0; };

} }
