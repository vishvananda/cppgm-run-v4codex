#pragma once
#include "syntax/ast.h"
#include <cstdint>
#include <exception>
#include <vector>

namespace cppgm { namespace semantic {
using syntax::NodeId;
typedef std::uint32_t TypeId;
typedef std::uint32_t EntityId;
typedef std::uint32_t ScopeId;
// Compact disjoint identities: a type argument is a TypeId; a value argument
// tags a canonical QueryId. Constants are Value queries (type plus bits), and
// dependent expressions retain their typed query graph until substitution.
using ArgumentId = std::uint32_t;
inline bool value_argument(ArgumentId a) { return (a >> 31) != 0; }
inline std::uint32_t argument_query(ArgumentId a) { return a & 0x7fffffffU; }

using Index = IdIndex;

enum class LiteralCallKind : unsigned char { String, Scalar, Pack, Raw };
enum class FactState : unsigned char { NotStarted, Active, Success, Failure };
enum class PatternMemberKind : unsigned char { Constructors, Destructor, PureVirtual };
enum class PatternPropertyKind : unsigned char { DefaultInitialization, Destruction };
enum class InitializationMode : unsigned char { Direct, Copy };
// Boolean success has two outcomes, while active and failed remain distinct.
// This compact encoding does not confuse a pending query with a false value.
enum class BooleanFact : unsigned char { NotStarted, Active, False, True, Failure };
enum class SemanticFact : unsigned char { None, ClassDefinition, FunctionDefinition, ClassLayout, MemberBody, TranslationUnit, MemberDefinition, Vtable, DestructorTriviality, DestructorException, ConstructorActions, DestructorActions, ConstructorEffects, DestructorEffects, Transfer, CopyStorage, DefaultArgument, ListConversion, DefaultDemand, DefaultBinding, InitializerBinding, ListInitialization, DefaultConstructorProperties, DefaultDestructorProperties };
// A cached rejection names its narrow producer without owning diagnostic text.
// The initial request reports the original error; subsequent demands cannot
// reinterpret partial publication as recursion or successful completion.
struct FailedSemanticFact : std::exception {
    SemanticFact fact;
    EntityId entity;
    NodeId source;
    FailedSemanticFact(SemanticFact f, EntityId e, NodeId n) : fact(f), entity(e), source(n) {}
    const char* what() const noexcept override { return "previously failed semantic fact"; }
};
// An absent prerequisite is not a failed computation for a complete key. A
// later declaration may make it available; no negative result is published.
struct UnavailableSemanticFact : std::exception {
    SemanticFact fact;
    EntityId entity;
    NodeId source;
    UnavailableSemanticFact(SemanticFact f, EntityId e, NodeId n) : fact(f), entity(e), source(n) {}
    const char* what() const noexcept override { return "semantic prerequisite unavailable"; }
};

enum class TypeKind : unsigned char { Fundamental, Named, Pointer, LRef, RRef, Array, Function, MemberPointer, DependentName, Decltype, DependentArray, ArgumentPack, PackExpansion, AliasApplication };
// The lookup obligation is part of a dependent name's canonical identity.
enum class DependentNameKind : unsigned char { Type, Application, Template };
enum class DeductionKind : unsigned char { Call, ClassPattern, PartialOrdering };
enum ExpansionCount { UnboundPack = -1, UnequalPacks = -2, DeferredPacks = -3 };
enum class RefQualifier : unsigned char { None, Lvalue, Rvalue };
struct FunctionQualifiers { unsigned char cv = 0; RefQualifier ref = RefQualifier::None; };
struct Type {
    TypeKind kind = TypeKind::Fundamental;
    unsigned char cv = 0;
    RefQualifier ref = RefQualifier::None;
    EFundamentalType fundamental = FT_INT;
    bool variadic = false;
    TypeId child = 0;
    EntityId entity = 0;
    // MemberPointer uses this slot for its canonical owner TypeId. Its entity
    // is only the Named owner projection consumed after substitution.
    std::uint64_t bound = 0;
    TypeId member_owner() const { return TypeId(bound); }
    std::uint32_t offset = 0, count = 0;
};
class Types {
    std::vector<TypeId> slots;
    std::vector<std::uint64_t> hashes;
    std::vector<TypeId> signatures, adjustments, alias_targets;
    TypeId intern(Type type, const std::vector<TypeId>& params);
public:
    Types();
    std::vector<Type> records;
    std::vector<TypeId> parameters;
    std::size_t probes = 0, signature_work = 0;
    TypeId fundamental(EFundamentalType f);
    TypeId named(EntityId e);
    TypeId alias_application(EntityId alias, TypeId result, std::uint32_t arguments);
    TypeId alias_target(TypeId type);
    TypeId pack_expansion(ArgumentId pattern, std::uint32_t captures);
    TypeId compound(TypeKind k, TypeId child, std::uint64_t bound = 0);
    TypeId qualify(TypeId t, unsigned cv);
    TypeId unqualified(TypeId t);
    TypeId function(TypeId result, const std::vector<TypeId>& params, bool variadic, unsigned cv = 0, RefQualifier ref = RefQualifier::None);
    TypeId member_pointer(EntityId owner, TypeId child);
    TypeId member_pointer_type(TypeId owner, TypeId child);
    TypeId dependent_name(TypeId owner, IdentifierId name, const std::vector<TypeId>& arguments, DependentNameKind kind);
    TypeId decltype_type(std::uint32_t expression, bool direct);
    TypeId adjusted(TypeId t);
    TypeId signature(TypeId t);
    TypeId composite(TypeId a, TypeId b);
    const Type& operator[](TypeId t) const { return records[t]; }
};

enum class EntityKind : unsigned char { Type, Alias, Namespace, NamespaceAlias, Variable, Function, Parameter, Enumerator, Overload };
enum class ScopeKind : unsigned char { Namespace, Class, Enum, Template, Function, Block, Control };
struct Constant {
    std::uint64_t bits = 0;
    TypeId type = 0;
    bool valid = false;
    Constant() {}
    Constant(TypeId t, std::uint64_t b) : bits(b), type(t), valid(true) {}
};
// Rare class demand state has a separate arena; ordinary bindings do not pay
// for constructors and layout. The stable index belongs to the class entity.
enum class Access : unsigned char { Public, Protected, Private };
enum class TransferKind : unsigned char { None = 0, CopyConstructor = 1, MoveConstructor = 2, CopyAssignment = 4, MoveAssignment = 8 };
struct ClassFacts {
    NodeId definition_source = 0;
    ScopeId definition_scope = 0;
    FactState definition_state = FactState::NotStarted;
    Access current_access = Access::Public;
    std::uint64_t size = 0, alignment = 0, requested_alignment = 0;
    std::uint64_t nonvirtual_size = 0, nonvirtual_alignment = 1;
    unsigned char packing = 0;
    EntityId constructor = 0, implicit_constructor = 0, storage = 0, destructor = 0;
    EntityId inherited_base = 0;
    EntityId first_conversion = 0;
    std::uint32_t first_base = 0;
    std::uint32_t primary_base = 0;
    std::uint32_t virtual_bases_begin = 0, virtual_bases_count = 0;
    unsigned rtti_flags = ~0U; // Uncomputed until an RTTI consumer demands it.
    ScopeId default_constructor = 0, member_initializer_scope = 0;
    FactState layout_state = FactState::NotStarted;
    std::uint32_t empty_types_begin = 0, empty_types_count = 0;
    bool aggregate = true, empty = true, nearly_empty = false;
    std::uint32_t virtual_info = 0;
    std::uint32_t lifecycle_begin = 0, lifecycle_count = 0;
    std::uint32_t vtt_secondary = 0, vtt_base_size = 0, vtt_size = 0;
    std::uint64_t base_offset = 0;
    EntityId local_function = 0;
    unsigned local_ordinal = 0;
    EntityId value_constructor = 0;
    EntityId variant_initializer = 0;
    bool has_member_initializer = false;
    unsigned char declared_transfers = 0, generated_transfers = 0;
    BooleanFact copy_storage_state = BooleanFact::NotStarted;
    BooleanFact trivial_destructor_state = BooleanFact::NotStarted;
    BooleanFact destructor_exception_state = BooleanFact::NotStarted;
    unsigned char value_abi = 0;
    unsigned char parameter_abi = 0;
    unsigned char parameter_state = 0; // Unqueried, rejected, body pending, proven.
    EntityId parameter_transfer = 0;
    bool user_constructor = false, user_destructor = false, final_class = false;
};
// Sparse object storage facts. Ordinary fields keep their existing offset;
// bit-fields and explicit declaration alignment use this descriptor by EntityId.
struct FieldFacts {
    std::uint64_t alignment = 0, declared_width = 0;
    TypeId storage_type = 0;
    unsigned char shift = 0, width = 0;
    bool bit_field = false, may_clear_unit = true, unit_transfer = false;
};
enum class InitKind : unsigned char { Scalar, Group, String, Constructor, Value, Converted };
struct ZeroInitialization {
    enum Kind : unsigned char { Scalar, Representation, Composite, Array, Reference, MemberPointer } kind = Scalar;
    TypeId type = 0;
    std::uint32_t first = 0, count = 0, child = 0;
    std::uint64_t bytes = 0, alignment = 1, elements = 0;
    bool bulk = true;
};
struct ZeroPart { std::uint32_t plan; std::uint64_t offset; };
struct InitAction {
    std::uint32_t conversion = 0;
    EntityId helper_transfer = 0, helper_parameter = 0;
    TypeId type = 0;
    NodeId source = 0;
    EntityId field = 0;
    std::uint32_t first = 0, next = 0;
    std::uint64_t index = 0, count = 1;
    InitKind kind = InitKind::Scalar;
    bool helper_safe = true; // Evaluating arguments cannot observe delayed destination stores.
    bool helper_copy = false; // Proven independent, trivially copyable class representation.
    bool helper_commutes = false; // Transfer has no throwing/external effects before later clauses.
    bool zero = false; // Value-initialization precedes the selected default constructor.
};
struct Entity {
    Access access = Access::Public;
    EntityKind kind = EntityKind::Variable;
    ETokenType key = TOK_INVALID;
    bool complete = false, scoped = false, template_parameter = false, is_static = false;
    bool parameter_pack = false;
    FactState body_state = FactState::NotStarted;
    bool deleted_function = false;
    syntax::FunctionEffects effects = syntax::FunctionEffects::Unknown;
    IdentifierId name = 0;
    ScopeId owner = 0, scope = 0;
    NodeId source = 0, definition = 0, initializer = 0, body = 0;
    enum Builtin : unsigned char { NoBuiltin, Memcpy, Memmove, Strlen, Malloc, Free } builtin = NoBuiltin;
    bool c_linkage = false, external_decl = false, thread_local_storage = false, inline_function = false;
    bool no_inline = false, force_inline = false, stable_prefix = false;
    bool constexpr_function = false;
    unsigned char allocation_runtime = 0;
    bool array_allocation = false;
    bool mutable_field = false;
    bool template_member = false, template_pattern = false, explicit_specialization = false;
    bool instantiation_declaration = false, instantiation_definition = false;
    unsigned char exception_spec = 0; // Low two bits: absent, direct noexcept, throwing, parenthesized true; bit 7: seen.
    enum Emission : unsigned char { HiddenFriend = 1, Used = 2 };
    unsigned char emission = 0;
    // Source presence is not successful checking. Bodies and their later
    // lifetime/control facts have independent terminal publication states.
    FactState lifetime_state = FactState::NotStarted;
    std::uint32_t defaults = 0;
    std::uint64_t member_offset = 0;
    TypeId type = 0, underlying = 0;
    std::uint32_t class_info = 0, member_info = 0, template_info = 0, specialization = 0;
    EntityId first = 0, second = 0; // Immutable overload union edges.
    Constant constant;
};
enum class DemandState : unsigned char { Dormant, Queued, Active, Complete, Failed };
enum class MemberDemandReason : unsigned char { Use = 1, LocalDefinition = 2, Vtable = 4, Transfer = 8, DefaultArgument = 16 };
struct MemberFacts {
    std::uint32_t prototype = 0;
    TypeId call_type = 0;
    TypeId conversion_target = 0;
    TypeId conversion_hiding_target = 0; // Alpha-normalized template conversion type.
    std::uint32_t explicit_condition = 0; // Canonical QueryId.
    EntityId next_conversion = 0;
    EntityId inherited_constructor = 0;
    std::uint32_t inherited_arguments = 0;
    FactState inherited_state = FactState::NotStarted;
    EntityId delegated_constructor = 0;
    NodeId body = 0, declarator = 0, source = 0;
    ScopeId body_environment = 0;
    DemandState demand = DemandState::Dormant;
    unsigned char demand_reasons = 0;
    bool synthetic = false, referenced = false, in_class_body = false;
    bool constructor = false, destructor = false, explicit_constructor = false, deleted = false;
    FactState actions_state = FactState::NotStarted;
    bool source_demand = false, base_entry = false, array_entry = false;
    bool complete_entry = false, retained_root = false;
    std::uint32_t action_begin = 0, action_count = 0;
    std::uint32_t default_conversions = 0;
    BooleanFact constructor_effects = BooleanFact::NotStarted, destructor_effects = BooleanFact::NotStarted;
    BooleanFact default_properties = BooleanFact::NotStarted;
    FactState destructor_properties = FactState::NotStarted;
    bool const_default = false;
    FactState exception_state = FactState::NotStarted;
    bool nonthrowing = false;
    std::uint32_t destruction_begin = 0, destruction_count = 0;
    TransferKind transfer = TransferKind::None;
    FactState transfer_state = FactState::NotStarted;
    bool transfer_trivial = false, transfer_direct = false, transfer_noexcept = false, defaulted_late = false;
    bool scalar_transfer_body = false;
    bool virtual_member = false, pure = false, final_member = false, override_member = false;
    std::uint32_t virtual_slot = 0; // One-based slot within the address point.
    TypeId virtual_signature = 0;
    EntityId deleting_deallocation = 0;
    bool emission_reference = false, polymorphic_base_entry = false, deleting_complete = false;
    std::uint32_t transfer_begin = 0, transfer_count = 0;
    EntityId transfer_parameter = 0;
};
enum class VtableReason : unsigned char { KeyDefinition = 1, Constructor = 2, Destructor = 4, Rtti = 8, Dispatch = 16 };
struct VirtualSlot {
    EntityId function = 0, declaration = 0;
    std::uint32_t origin = 0, implementation = 0;
    std::uint32_t receiver = 0; // Zero: complete class; otherwise one-based view.
    std::int64_t this_adjustment = 0, result_adjustment = 0;
    std::int64_t result_virtual_row = 0;
    VirtualSlot() {}
    explicit VirtualSlot(EntityId e) : function(e), declaration(e) {}
};
struct VirtualPrefixRow {
    EntityId base = 0, declaration = 0; // Exactly one identity: vbase or vcall.
    std::uint32_t origin = 0; // Declaration occurrence in the prefix owner.
    unsigned receiver = 0; // Final overrider occurrence, filled by layout.
};
struct VirtualView {
    EntityId type = 0;
    std::uint32_t subobject = 0; // Physical vptr occurrence; primary aliases share it.
    std::uint32_t parent = 0, edge = 0;
    std::uint32_t begin = 0, count = 0; // Slice of the owning class's slot arena.
    std::uint64_t offset = 0;
    std::uint64_t group_address_point = 16;
    std::uint32_t vcall_rows = 0, vbase_rows = 0;
    std::uint64_t address_point = 16;
    EntityId virtual_anchor = 0;
    std::uint64_t virtual_tail = 0;
    unsigned prefix_begin = 0, prefix_count = 0;
    unsigned vtt_index = 0;
    bool store = false;
};
struct VirtualClass {
    // One contiguous arena per completed class; views never own allocations.
    // Complete, then deleting destructor occupies two entries.
    std::vector<VirtualSlot> slots;
    std::uint32_t primary_count = 0;
    std::vector<VirtualView> views;
    std::vector<unsigned> store_order; // Physical vptr order, completed with layout.
    // Closest-to-address-point first. Virtual-prefix is the canonical template
    // when this class is itself a virtual base; view rows carry local receivers.
    std::vector<VirtualPrefixRow> prefix, virtual_prefix, view_prefix;
    std::vector<EntityId> base_order; // ABI preorder; lifecycle uses postorder.
    std::vector<unsigned> vtt_order;
    Index signatures;
    EntityId key_function = 0;
    std::uint64_t address_point = 16;
    bool polymorphic = false;
    bool abstract = false, referenced = false;
    FactState demand = FactState::NotStarted, construction_demand = FactState::NotStarted;
    unsigned char reasons = 0;
};
struct TransferAction {
    enum Kind : unsigned char { Scalar, Reference, Subobject, Unit, Storage, Empty } kind = Scalar;
    EntityId field = 0, function = 0;
    TypeId type = 0;
    std::uint32_t default_conversions = 0, base = 0;
    std::uint64_t bytes = 0, alignment = 1;
};
struct TypeArguments { std::uint32_t offset = 0, count = 0; std::uint64_t hash = 0; };
struct TemplateFunction {
    EntityId primary = 0;
    std::uint32_t explicit_arguments = 0;
    ScopeId environment = 0;
    std::uint32_t offset = 0, count = 0;
    NodeId body = 0, declarator = 0, source = 0;
    std::uint32_t source_parameters = 0, source_count = 0, parent_frame = 0;
};
struct Specialization {
    EntityId pattern = 0, entity = 0;
    std::uint32_t arguments = 0;
    EntityId definition_pattern = 0;
    std::uint32_t definition_arguments = 0;
    FactState declaration = FactState::NotStarted, body = FactState::NotStarted;
    ScopeId environment = 0;
    std::uint32_t context = 0;
    bool emission_demanded = false;
};
struct TemplateDefinition {
    NodeId source = 0, declarator = 0, initializer = 0;
    std::uint32_t parameters = 0, count = 0, next = 0;
    std::uint32_t selected_next = 0;
    NodeId member_template = 0;
    std::uint32_t heads = 0, head_count = 0;
    bool checked = false;
};
struct TemplateDefinitionHead {
    EntityId pattern = 0;
    std::uint32_t parameters = 0, count = 0, depth = 0;
};
struct TemplateDefinitionOwner { EntityId specialization = 0; std::uint32_t path = 0; };
struct BaseRelation { EntityId base; std::uint32_t next; Access access = Access::Public; std::uint64_t offset = 0; bool virtual_base = false;
    BaseRelation(EntityId b, std::uint32_t n, Access a = Access::Public, bool v = false) : base(b), next(n), access(a), virtual_base(v) {} };
struct ObjectAction { EntityId object, constructor; TypeId address_type; };
struct LifecycleBase {
    EntityId type; std::uint64_t offset; std::uint32_t vtt; bool virtual_base;
};
// A constructor owns each projected anonymous-storage path. Parent identities
// share prefixes; the completed layout supplies one cumulative byte offset.
struct ConstructionStorage {
    EntityId field; std::uint32_t parent; std::uint64_t offset;
};
struct FieldProjection {
    std::uint64_t offset = 0;
    EntityId object = 0; // Non-member storage for an injected namespace/local field.
};
struct SubobjectAction {
    EntityId field; TypeId type; NodeId initializer; EntityId constructor; std::uint32_t base;
    std::uint32_t storage = 0, receiver_storage = 0;
    SubobjectAction(EntityId f, TypeId t, NodeId i, EntityId c, std::uint32_t b = 0)
        : field(f), type(t), initializer(i), constructor(c), base(b) {}
};
struct InheritedArgument {
    EntityId parameter = 0, transfer = 0;
    NodeId value = 0;
    std::uint32_t conversion = 0, transfer_defaults = 0;
};
struct DestructionAction {
    EntityId field; TypeId type; EntityId destructor; std::uint32_t base;
    DestructionAction(EntityId f, TypeId t, EntityId d, std::uint32_t b = 0)
        : field(f), type(t), destructor(d), base(b) {}
};
struct LifetimeState { EntityId object = 0, destructor = 0; std::uint32_t tail = 0, depth = 0; };
struct ThrowUse { TypeId type = 0; NodeId source = 0; std::uint32_t conversion = 0; EntityId destructor = 0; };
struct LifetimeUse {
    std::uint32_t entry = 0, exit = 0, target = 0;
    NodeId context = 0, expression_region = 0, target_region = 0;
};
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
struct Edge { ScopeId target = 0; std::uint32_t next = 0, inline_next = 0; bool inline_namespace = false, injected_member = false; };
enum class ValueCategory : unsigned char { Prvalue, Lvalue, Xvalue };
enum class Intrinsic : unsigned char {
    None, VaStart, VaEnd, VaCopy, StackAlloc, Expect, Abort, Unreachable, AtomicFetchAdd, AtomicAddFetch,
    Clz, Ctz, Popcount, Parity, Ffs, Bswap, Clzg, Ctzg, Popcountg, Prefetch, AssumeAligned, FltRounds, AddOverflow, SubOverflow, MulOverflow
};
enum class ExpressionForm : unsigned char { Ordinary, Overload, Cast, ConstantQuery, Abort, Unreachable, PseudoDestructor, Construction, OperatorCall, LiteralCall, FloatFinite, FloatNaN, FloatInfinite, FloatNormal, FloatSignbit, FloatGreater, FloatGreaterEqual, FloatLess, FloatLessEqual, FloatLessGreater, FloatUnordered, FloatClassify, InitializerList, ListValue, BoundMember, Expect, Typeid, DynamicCast, TypeinfoEqual, TypeinfoUnequal };
struct RttiExpression {
    TypeId type = 0, source = 0;
    std::int64_t hint = -1;
    bool dynamic = false, reference = false;
};
enum class CallInputs : unsigned char { Concrete, Source, Context, Query };
struct Expression {
    std::uint32_t object_use = 0; // Rare field/member-call facts in the TU arena.
    TypeId type = 0; // Reference-free language expression type.
    EntityId entity = 0; // Known identity of this value, never a producing call.
    // Calls record their selected declaration in Fact::entity. Conversion ranges
    // belong only to this node (including operators), not to transparent wrappers.
    std::uint32_t conversions = 0, count = 0, incoming = 0;
    std::uint32_t arguments = 0, argument_count = 0;
    ValueCategory category = ValueCategory::Prvalue;
    ExpressionForm form = ExpressionForm::Ordinary;
    CallInputs inputs = CallInputs::Concrete;
    bool ready : 1;
    bool evaluated : 1;
    bool null_pointer_constant : 1;
    bool discarded_form : 1; // Built-in forms eligible for [expr]/11 volatile reads.
    Expression() : ready(false), evaluated(false), null_pointer_constant(false), discarded_form(false) {}
};
struct BaseAdjustment {
    std::uint64_t offset = 0, total = 0;
    std::uint32_t next = 0, edge = 0, inverse = 0;
    std::uint32_t subobject = 0;
    EntityId source = 0;
    std::int64_t virtual_row = 0;
    std::uint64_t virtual_tail = 0;
    bool ambiguous = false, laid_out = false;
};
// Relative subobject identity: a shared virtual anchor, followed by an interned
// nonvirtual edge path. Zero denotes the current complete class occurrence.
struct SubobjectIdentity { EntityId anchor = 0; std::uint32_t path = 0; };
struct SubobjectPath { std::uint32_t edge = 0, next = 0; };
struct ObjectUse {
    unsigned capture = 0; // Checked closure field containing an object address or source this.
    EntityId callable_entry = 0; // Receiver-free closure ABI; selected declaration stays on the call.
    ScopeId naming_scope = 0; EntityId temporary = 0; NodeId node = 0; TypeId type = 0;
    NodeId member_pointer = 0, callee = 0; // Actual callable of an intrinsic invocation.
    std::uint32_t member_target = 0; // Completed StaticFact for an immediate member address.
    std::uint32_t arrow = 0;
    std::uint32_t virtual_slot = 0;
    unsigned adjustment = 0, qualifier_adjustment = 0; std::uint32_t callee_conversion = 0;
    bool value_initialize = false, source_owned = false; };
struct ArrowStep {
    EntityId function = 0, temporary = 0;
    TypeId result = 0;
    unsigned adjustment = 0, virtual_slot = 0;
};
struct ArrowChain { std::uint32_t first = 0, count = 0; TypeId type = 0; };
struct Conversion {
    TypeId target = 0;
    std::uint32_t adjustment = 0; // Canonical BaseAdjustment chain; zero is the identity path.
    EntityId function = 0; // Target-selected overload, if any.
    std::uint32_t materialization = 0;
    unsigned char rank = 255, qualification = 0;
    bool reference : 1, temporary : 1, derived : 1, empty_copy : 1, fold_widen : 1, implicit_move : 1;
    bool storage_write : 1;
    bool preserve_widen : 1, ambiguous : 1, constant_forbidden : 1, ellipsis_object : 1, ellipsis_unavailable : 1;
    Conversion() : reference(false), temporary(false), derived(false), empty_copy(false), fold_widen(false),
        implicit_move(false), storage_write(false), preserve_widen(false), ambiguous(false), constant_forbidden(false), ellipsis_object(false), ellipsis_unavailable(false) {}
    unsigned char preference = 0;
    enum class Kind : unsigned char { Standard, Explicit, Contextual, Discarded, Construction, User, ListPlan, List, QueryList };
    Kind kind = Kind::Standard;
    bool valid() const { return rank != 255; }
};
enum class ConversionUse : unsigned char { Recipe, Temporary, Destination };
struct ConversionObject { EntityId constructor = 0, temporary = 0; Expression call; std::uint32_t branches = 0; bool elided = false, elision_permission = false, retained = false; ConversionUse use = ConversionUse::Recipe; };
enum class CallFailure : unsigned char { None, NoViable, Ambiguous };
struct CallSelection {
    EntityId entity = 0, conflicting = 0;
    CallFailure failure = CallFailure::NoViable;
};
// Implicit language operations consume typed values, never manufactured AST
// nodes. Conversions include the receiver first for a nonstatic member call.
struct RangeOperation {
    Expression result;
    EntityId function = 0, temporary = 0;
    TypeId returned = 0;
    ETokenType op = TOK_INVALID;
    std::uint32_t adjustment = 0, virtual_slot = 0;
    unsigned supplied = 0;
    bool receiver = false;
};
struct RangePlan {
    NodeId source = 0, body = 0;
    EntityId range = 0, begin = 0, end = 0, variable = 0;
    TypeId array = 0, index_type = 0, list_element = 0;
    EntityId list_begin = 0, list_size = 0;
    RangeOperation first, last, test, next, element;
    std::uint32_t begin_conversion = 0, end_conversion = 0, element_conversion = 0, condition_conversion = 0;
    std::uint32_t loop_live = 0, body_live = 0;
    bool initialize_range = false;
};
// Declaration slots identify defaults; a function specialization supplies the
// concrete parameter/environment component of a default's fact key.
enum class DefaultReason : unsigned char { Declaration = 1, Argument = 2, Recipe = 4 };
enum class DefaultDependencyKind : unsigned char { Member, Specialization, Storage, Argument };
struct DefaultDependency {
    std::uint32_t target = 0, next = 0;
    DefaultDependencyKind kind = DefaultDependencyKind::Member;
    DefaultDependency(std::uint32_t t, std::uint32_t n, DefaultDependencyKind k) : target(t), next(n), kind(k) {}
};
struct DefaultArgumentFact {
    NodeId root = 0, value = 0;
    std::uint32_t conversion = 0, dependencies = 0;
    FactState state = FactState::NotStarted, demand = FactState::NotStarted;
    unsigned char reasons = 0;
};
struct ExceptionSpecificationFact {
    NodeId expression = 0, declarator = 0;
    NodeId dynamic_types = 0;
    std::uint32_t allowed_begin = 0, allowed_count = 0;
    ScopeId scope = 0;
    std::uint32_t previous = 0;
    EntityId pattern = 0;
    FactState state = FactState::NotStarted;
    unsigned char specification = 0;
    unsigned char prior_specification = 0;
    bool destructor = false;
};
struct ListPlan {
    NodeId source = 0; TypeId target = 0; ScopeId scope = 0;
    std::uint32_t query = 0; // Canonical formation query; arguments are QueryIds in query_edges.
    std::uint32_t literal = 0; // Character-array initialization from an immutable literal.
    EntityId constructor = 0; Expression call;
    std::uint32_t fields = 0, explicit_count = 0;
    TypeId backing_element = 0;
    EntityId backing_begin = 0, backing_size = 0;
    bool aggregate = false, direct_binding = false, direct = false, zero = false, allocated = false;
    FactState state = FactState::NotStarted;
    unsigned char rank = 255;
    FactState validation = FactState::NotStarted;
};
struct ListField { EntityId field = 0; TypeId type = 0; std::uint64_t index = 0, count = 1; };
struct ListObject { EntityId temporary = 0, backing = 0; std::uint32_t plan = 0, initializer = 0; Expression call; };
struct InitializerListType { TypeId element = 0; EntityId begin = 0, size = 0; };
struct UserConversion {
    Conversion object, result;
    EntityId temporary = 0, source_temporary = 0, object_entity = 0;
    unsigned adjustment = 0;
    std::uint32_t virtual_slot = 0;
    bool prepared = false, omit_receiver = false;
    ConversionUse use = ConversionUse::Recipe;
};
struct BuiltinOperator { TypeId type = 0, computation = 0; ValueCategory category = ValueCategory::Prvalue; Conversion arguments[2]; };
struct ScalarConsumption { NodeId expression = 0; TypeId target = 0; std::uint32_t conversion = 0; unsigned char truth = 0; bool private_destination = false; };
struct ValueInitialization { NodeId source = 0; std::uint32_t conversion = 0; };
struct ReferenceStorage { EntityId object = 0, reference = 0; bool scalar = false, conditional = false; };
struct ValueReturn { NodeId source = 0; EntityId local = 0; std::uint32_t conversion = 0, next = 0; };
struct FunctionReturn { EntityId object = 0; std::uint32_t first = 0, last = 0; };
struct PlacementNew {
    EntityId allocation = 0, constructor = 0, deallocation = 0, destructor = 0;
    TypeId type = 0, leaf = 0; NodeId initializer = 0, bound = 0;
    std::uint64_t fixed_count = 0, stride = 0, cookie = 0;
    Expression call; bool array = false, zero = false, construct = false, narrow_extent = false;
    std::uint32_t zero_plan = 0;
};
struct DeleteExpression {
    EntityId deallocation = 0, destructor = 0;
    TypeId type = 0, leaf = 0; NodeId operand = 0;
    std::uint64_t cookie = 0;
    Conversion conversion; bool array = false, sized = false;
    std::uint32_t virtual_slot = 0;
    bool global_deallocation = false;
};
struct StaticValue {
    enum Kind : unsigned char { Invalid, Integer, Floating, Address, String, MemberFunction, Vtable } kind = Invalid;
    std::uint64_t bits = 0;
    long double floating = 0;
    bool signaling = false;
    EntityId entity = 0;
    NodeId string = 0;
    std::int64_t addend = 0;
};
struct ConstantField { EntityId field; TypeId type; StaticValue value; std::uint64_t offset; };
struct ConstantObject { std::uint32_t first = 0, count = 0; bool valid = false; };
struct ConstructorConstantAction { EntityId field; TypeId type; NodeId source; std::uint32_t argument; };
struct Fact { NodeId target = 0; TypeId type = 0; EntityId entity = 0; ScopeId scope = 0; std::uint32_t value = 0; };

} }
