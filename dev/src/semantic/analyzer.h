#pragma once
#include "semantic/type_query.h"
#include "semantic/expression_store.h"
#include "semantic/fact_store.h"
#include "semantic/template_binding.h"
#include "semantic/model.h"
#include "syntax/parser.h"

namespace cppgm { namespace semantic {
// Semantic facts extend the parser's sole graph, indexed by its stable NodeId.
// The parser calls this boundary before proceeding to the next source region.
class Analyzer : public syntax::DeclarationConsumer {
public:
    Analyzer(syntax::Ast& ast, IdentifierTable& ids, bool calls = false, bool definitions = false);
    void consume(NodeId declaration) override;
    void finish();
    void require_body_facts(EntityId e) const;
    void write(std::ostream& out) const;
    void write_semantics(std::ostream& out, NodeId root) const;
    void telemetry(std::ostream& out) const;
    Types types;
    std::vector<Entity> entities;
    std::vector<Scope> scopes;
    std::vector<Declaration> declarations;
    FactStore facts;
    Expression expression_fact(NodeId n) const { return expressions[n]; }
    NodeId call_argument(const Expression& call, unsigned i = 0) const;
    ObjectUse object_fact(NodeId n) const {
        auto use = object_uses[expressions[n].object_use];
        return use.source_owned ? project_object_use(use,n) : use;
    }
    ObjectUse project_object_use(ObjectUse use, NodeId n) const;
    const Conversion& conversion_fact(std::uint32_t n) const { return conversions[n]; }
    EntityId specialization_pattern(EntityId e) const { return specializations[entities[e].specialization].pattern; }
    TypeArguments specialization_arguments(EntityId e) const { return argument_packs[specializations[entities[e].specialization].arguments]; }
    TypeId template_argument(std::uint32_t n) const { return argument_types[n]; }
    unsigned template_ordinal(EntityId e) const { return parameter_ordinals.get(e)-1; }
    const TypeQuery& type_query(QueryId id) const { return type_queries[id]; }
    QueryId type_query_child(QueryId id, unsigned i) const { return query_edges[type_queries[id].offset+i]; }
    TypeArguments query_arguments(std::uint32_t pack) const { return argument_packs[pack]; }
    ScopeId global = 0;
    std::vector<NodeId> call_arguments, default_arguments;
    NodeId default_argument(EntityId e, unsigned parameter, Conversion* converted = 0, DefaultReason reason = DefaultReason::Argument);
    NodeId default_argument_value(EntityId e, unsigned parameter) const;
    // Queries completed expression facts; keys are the expression and target.
    StaticValue static_value(NodeId n, TypeId target);
    std::size_t static_requests = 0, static_hits = 0;
    Constant constant_fact(NodeId n) const { return facts[n].value ? constants[facts[n].value] : Constant(); }
    std::uint64_t object_size(TypeId t) { return size(t); }
    std::uint64_t object_alignment(TypeId t) { return size(t, true); }
    bool unsigned_type(TypeId t) const { return is_unsigned(t); }
    TypeId call_type(EntityId e) const;
    bool member_demanded(EntityId e) const;
    bool dormant_hidden_friend(EntityId e) const { return entities[e].emission == Entity::HiddenFriend; }
    const VirtualClass& virtual_class(EntityId e) const { return virtual_classes[class_facts[entities[e].class_info].virtual_info]; }
    std::uint32_t virtual_class_id(EntityId e) const { return class_facts[entities[e].class_info].virtual_info; }
    std::size_t virtual_class_count() const { return virtual_classes.size(); }
    std::size_t class_count() const { return class_facts.size(); }
    std::size_t member_count() const { return members.size(); }
    const std::vector<EntityId>& demanded_vtables() const { return vtable_emission; }
    EntityId local_function(EntityId e) const { return entities[e].class_info ? class_facts[entities[e].class_info].local_function : local_enum_functions.get(e); }
    unsigned local_ordinal(EntityId e) const { return entities[e].class_info ? class_facts[entities[e].class_info].local_ordinal : local_enum_ordinals.get(e); }
    bool polymorphic(EntityId e) const { return entities[e].class_info && class_facts[entities[e].class_info].virtual_info; }
    std::uint64_t base_offset(TypeId t) { size(t); return class_facts[entities[types[t].entity].class_info].base_offset; }
    EntityId direct_base(EntityId e) const { auto b = class_facts[entities[e].class_info].first_base; return b ? bases[b].base : 0; }
    bool constructor_member(EntityId e) const;
    bool constructor_needed(EntityId e);
    bool destructor_needed(EntityId e);
    bool temporary_cleanup(EntityId object);
    Expression member_pointer_expression(NodeId n, ScopeId s);
    bool trivial_destructor(TypeId t);
    EntityId type_destructor(TypeId t) const;
    EntityId converted_temporary(const Conversion& c) const;
    EntityId bound_temporary(NodeId n) const;
    bool destructor_member(EntityId e) const;
    bool function_nonthrowing(EntityId e);
    bool scalar_transfer_source(EntityId transfer, NodeId source) const;
    EntityId object_destructor(EntityId e) const { return object_destructors.get(e); }
    const LifetimeUse& lifetime_use(NodeId n) const { return lifetime_uses[lifetime_index.get(n)]; }
    std::uint32_t object_lifetime(EntityId e) const { return object_lifetimes.get(e); }
    unsigned return_count(std::uint32_t state, NodeId context) const { return return_counts.get(key(state, context)); }
    std::vector<LifetimeState> lifetimes = std::vector<LifetimeState>(1);
    std::vector<DestructionAction> destruction_actions;
    EntityId value_constructor(TypeId t) const;
    EntityId object_constructor(EntityId e) const;
    EntityId static_vptr(EntityId e) const { return static_vptr_objects.get(e); }
    const MemberFacts& member_fact(EntityId e) const { return members[entities[e].member_info]; }
    std::vector<SubobjectAction> subobject_actions;
    bool synthetic_member(EntityId e) const;
    bool transfer_member(EntityId e) const;
    bool trivial_transfer(EntityId e) const;
    bool direct_transfer(EntityId e) const;
    bool class_value(TypeId t) const;
    bool indirect_value(TypeId t) const;
    bool indirect_parameter(TypeId t) const;
    const ScalarConsumption& scalar_consumption(EntityId object) const;
    bool empty_class(TypeId t) const;
    bool parameter_cleanup(EntityId e) const;
    EntityId reference_temporary(EntityId e) const { return reference_temporaries.get(e); }
    struct ReferenceAlternative { EntityId object; std::uint32_t next; };
    std::vector<ReferenceAlternative> reference_alternatives = std::vector<ReferenceAlternative>(1);
    std::uint32_t reference_choices(EntityId e) const { return conditional_references.get(e); }
    const ReferenceStorage& static_temporary(EntityId e) const { return reference_storage[static_temporaries.get(e)]; }
    EntityId reference_scalar(EntityId e) const { return reference_scalars.get(e); }
    std::vector<ReferenceStorage> reference_storage = std::vector<ReferenceStorage>(1);
    const ValueInitialization& class_initialization(NodeId n, TypeId t) const;
    const ValueReturn& class_return(NodeId n) const;
    EntityId return_object(EntityId e) const;
    std::vector<TransferAction> transfers;
    bool nonstatic_field(EntityId e) const;
    EntityId injected_storage(EntityId field) const;
    EntityId anonymous_object(NodeId declaration) const { return anonymous_objects.get(declaration); }
    const FieldFacts& field_fact(EntityId e) const { return field_facts[field_index.get(e)]; }
    std::vector<InitAction> initializers = std::vector<InitAction>(1);
    std::vector<ListPlan> list_plans = std::vector<ListPlan>(1);
    std::vector<ListObject> list_objects = std::vector<ListObject>(1);
    std::uint32_t initializer_plan(NodeId n, TypeId t) const;
    bool zero_value(TypeId t);
    std::uint32_t prepare_zero_initialization(TypeId t);
    std::uint32_t zero_initialization(TypeId t) const { return zero_initialization_index.get(t); }
    std::vector<ZeroInitialization> zero_initializations = std::vector<ZeroInitialization>(1);
    std::vector<ZeroPart> zero_parts;
    bool empty_value(TypeId t);
    bool initializer_work(std::uint32_t plan);
    std::vector<ConversionObject> conversion_objects = std::vector<ConversionObject>(1);
    std::vector<UserConversion> user_conversions = std::vector<UserConversion>(1);
    IdentifierId literal_suffix(EntityId e) const { return literal_functions.get(e); }
    const PlacementNew& placement_fact(NodeId n) const { return placements[placement_index.get(n)]; }
    const DeleteExpression& delete_fact(NodeId n) const { return deletions[delete_index.get(n)]; }
    const ConstantObject& constant_construction(NodeId n, TypeId t);
    std::vector<ConstantField> constant_fields;
private:
    FactState completion_state = FactState::NotStarted;
    Index list_index, direct_list_index, empty_list_index, empty_direct_list_index;
    Index class_typedef_declarations;
    std::vector<ListField> list_fields;
    Conversion list_initialization(NodeId n, TypeId to, ScopeId s = 0, bool direct = false);
    Conversion list_element(NodeId& cursor, TypeId to, ScopeId s);
    std::uint32_t list_aggregate(NodeId& cursor, TypeId to, ScopeId s);
    void prepare_list(NodeId n, Conversion& c);
    void store_call(Expression& owner, const std::vector<NodeId>& args, const std::vector<Conversion>& selected);
    EntityId global_allocation(ETokenType op, bool array);
    bool array_operator(NodeId name) const;
    Expression delete_expression(NodeId n, ScopeId s);
    void finish_allocations();
    EntityId select_deallocation(TypeId t, bool array, bool global, ScopeId s);
    Index delete_index;
    std::vector<DeleteExpression> deletions = std::vector<DeleteExpression>(1);
    void prepare_value_boundary(TypeId t);
    void prepare_function_boundaries();
    bool local_scalar(EntityId object) const;
    bool private_scalar(EntityId object) const;
    void observe_scalar(NodeId expression);
    bool direct_class_call(NodeId expression);
    unsigned char scalar_truth(NodeId expression);
    void prepare_scalar_consumption(EntityId object);
    void schedule_parameter_bodies(EntityId& cursor);
    void query_parameter_representation(TypeId type);
    void finish_parameter_representation(TypeId type);
    void class_result(NodeId n, Expression& result, ScopeId s);
    bool record_class_initialization(NodeId n, TypeId target, NodeId source, const Conversion* selected = 0);
    void record_class_return(NodeId n, ScopeId s);
    void finish_class_returns(EntityId e);
    EntityId current_function = 0;
    Index class_initializer_index, class_return_index, function_return_index;
    Index reference_temporaries;
    Index conditional_references;
    std::uint64_t reference_binding_work = 0;
    NodeId reference_operand(NodeId n) const;
    void local_reference(NodeId n, EntityId reference, bool conditional = false);
    Index static_temporaries, reference_scalars;
    void static_reference(EntityId e);
    void retain_reference_object(NodeId n, EntityId reference, bool conditional);
    std::vector<ValueInitialization> value_initializations = std::vector<ValueInitialization>(1);
    std::vector<ValueReturn> value_returns = std::vector<ValueReturn>(1);
    std::vector<FunctionReturn> function_returns = std::vector<FunctionReturn>(1);
    void classify_transfer(EntityId e, NodeId special, ScopeId context);
    void ensure_transfers(TypeId t, bool assignment);
    void prepare_transfer(EntityId e);
    bool copy_storage_type(TypeId t);
    bool deleted_transfer(EntityId e);
    EntityId select_transfer(TypeId target, TypeId source, ValueCategory category, bool assignment,
        InitializationMode mode = InitializationMode::Direct);
    Conversion transfer_initialization(Expression value, TypeId target, InitializationMode mode);
    Conversion transfer_conversion(TypeId from, ValueCategory category, TypeId to);
    bool transfer_accessible(EntityId e, ScopeId context) const;
    void prepare_scalar_transfer(EntityId e);
    bool scalar_transfer_node(NodeId n);
    std::vector<unsigned char> scalar_transfer_nodes;
    std::uint64_t scalar_transfer_work = 0;
    Index anonymous_objects;
    Index constant_constructors, constant_objects;
    Index static_vptr_objects;
    void prepare_static_vptrs();
    std::vector<ConstantObject> constructor_constants = std::vector<ConstantObject>(1), object_constants = std::vector<ConstantObject>(1);
    std::vector<ConstructorConstantAction> constructor_constant_actions;
    std::uint32_t constant_constructor(EntityId ctor);
    std::vector<PlacementNew> placements = std::vector<PlacementNew>(1);
    Index placement_index;
    Expression placement_new(NodeId n, ScopeId s);
    Index literal_functions, literal_names;
    IdentifierId literal_name(IdentifierId suffix);
    Expression literal_call(NodeId n, ScopeId s);
    Index initializer_work_index;
    Index initializer_index, zero_value_index, value_contexts;
    Index zero_initialization_index;
    TypeId initialized_field_type(TypeId owner, EntityId field);
    void aggregate_initialization(NodeId n, TypeId t, ScopeId s);
    std::uint32_t initializer_item(NodeId& cursor, TypeId t, ScopeId s);
    bool aggregate_type(TypeId t) const;
    bool string_initialization(NodeId n, TypeId t) const;
    void list_conversion(NodeId n, TypeId t);
    void list_conversion_from(NodeId n, TypeId from, TypeId target);
    Index scalar_observations, scalar_consumption_index;
    std::vector<ScalarConsumption> scalar_consumptions = std::vector<ScalarConsumption>(1);
    std::uint64_t scalar_consumption_work = 0, scalar_observation_count = 0;
    std::uint64_t unit_transfer_fields = 0;
    std::uint64_t parameter_queries = 0, parameter_query_work = 0;
    Index field_index, local_class_names, local_enum_functions, local_enum_ordinals;
    std::vector<FieldFacts> field_facts = std::vector<FieldFacts>(1);
    std::uint64_t alignment_attributes(NodeId n, ScopeId s);
    FieldFacts& field_metadata(EntityId e);
    void bit_field_properties(EntityId field, Constant count);
    void bit_field_declaration(NodeId n, ScopeId s);
    void class_layout(EntityId e);
    syntax::AstView ast;
    bool definitions;
    IdentifierTable& ids;
    bool calls;
    bool c_linkage = false;
    struct StaticFact { FactState state = FactState::NotStarted; StaticValue value; };
    Index static_index;
    std::vector<StaticFact> static_facts;
    StaticValue static_value_impl(NodeId n, TypeId target);
    void function_defaults(EntityId e, NodeId d, ScopeId s, NodeId source);
    void bind_template_defaults(NodeId d, ScopeId s, ScopeId head = 0, bool allowed = true);
    enum class TemplateClassUseKind : unsigned char { DefaultArgument, MemberInitializer };
    struct TemplateClassUse { NodeId source; ScopeId scope, head; EntityId entity; TemplateClassUseKind kind; };
    std::vector<TemplateClassUse> template_class_uses;
    ScopeId active_template_class = 0;
    std::size_t template_default_binding_work = 0, template_default_binding_queued = 0;
    Index template_initializer_bindings;
    std::size_t template_initializer_binding_work = 0, template_initializer_binding_queued = 0;
    void bind_template_initializer(EntityId entity, ScopeId scope);
    void check_template_initialization(NodeId n, TypeId target, ScopeId scope,
        InitializationMode mode = InitializationMode::Direct);
    bool check_template_initializer_item(NodeId& cursor, TypeId target, ScopeId scope);
    bool check_template_constructor(NodeId n, TypeId target, ScopeId scope, InitializationMode mode);
    bool reuse_template_constructor(NodeId n, TypeId target, const std::vector<NodeId>& args,
        Expression& result, ScopeId scope, EntityId& selected);
    std::uint32_t retained_initialization(NodeId n, TypeId target);
    void remember_initialization(NodeId n, Conversion conversion);
    Index template_initialization_conversions, template_initializer_calls, template_initializer_narrowing;
    std::size_t initializer_recipe_work = 0, initializer_recipe_uses = 0;
    Index template_default_bindings;
    std::uint64_t default_argument_key(EntityId e, unsigned parameter) const;
    Index default_argument_index;
    std::vector<DefaultArgumentFact> default_argument_facts = std::vector<DefaultArgumentFact>(1);
    std::vector<DefaultDependency> default_dependencies;
    std::uint32_t active_default_fact = 0;
    void record_default_dependency(DefaultDependencyKind kind, std::uint32_t target);
    void capture_default_conversion(const Conversion& c);
    void demand_default_fact(std::uint32_t id);
    std::size_t default_dependency_work = 0, default_demand_work = 0;
    Index default_environments;
    std::size_t default_environment_work = 0, default_argument_work = 0;
    ScopeId default_environment(EntityId e, ScopeId head);
    void demand_region(NodeId root);
    ExpressionStore expressions;
    std::vector<ObjectUse> object_uses = std::vector<ObjectUse>(1);
    Index object_destructors, lifetime_index, object_lifetimes, return_counts;
    std::vector<LifetimeUse> lifetime_uses = std::vector<LifetimeUse>(1);
    std::vector<EntityId> jump_bodies;
    void finish_body(EntityId e);
    EntityId default_destructor(TypeId t, ScopeId s = 0, bool demand = true);
    void destructor_actions(EntityId e);
    bool implicit_destructor_nonthrowing(EntityId cls);
    bool type_destructor_nonthrowing(TypeId type);
    void require_destructor_class(EntityId cls);
    bool variant_destruction_effects(TypeId t);
    Index variant_destruction_index;
    void register_destruction(EntityId e);
    EntityId destination_destructor(TypeId t, ScopeId s);
    void exception_specification(EntityId e, NodeId declarator, ScopeId scope);
    Index friendships, using_access, using_functions, hidden_friends;
    bool friend_declaration(NodeId n, ScopeId s);
    EntityId associated_lookup(IdentifierId name, const std::vector<NodeId>& args);
    EntityId associated_type_lookup(IdentifierId name, std::vector<TypeId> work);
    ScopeId access_override = 0;
    Access declaration_access(ScopeId s) const;
    bool privileged(ScopeId context, EntityId cls) const;
    bool class_derives(EntityId derived, EntityId base) const;
    std::uint32_t access_base(EntityId entity) const;
    void check_base_entity_access(EntityId from, EntityId to, ScopeId context);
    void check_access(EntityId e, ScopeId context, ScopeId naming = 0, TypeId object = 0);
    void check_base_access(TypeId from, TypeId to, ScopeId context);
    ScopeId naming_class(ScopeId s) const;
    void record_object(Expression& owner, NodeId node, TypeId type, unsigned adjustment);
    std::vector<Conversion> conversions;
    Index function_families, function_signatures, function_ref_modes;
    std::size_t expression_work = 0, candidate_work = 0, conversion_work = 0, dependence_work = 0;
    TypeId return_type = 0;
    unsigned loop_depth = 0, switch_depth = 0;
    unsigned unevaluated_depth = 0;
    struct SwitchContext { TypeId type = 0; bool has_default = false; Index labels; };
    std::vector<SwitchContext> switches;
    IdentifierId constant_builtin = 0, abort_builtin = 0;
    Index ordinary, tags, namespaces, qualifiers, edge_index;
    std::vector<Constant> constants;
    std::vector<ClassFacts> class_facts;
    std::vector<MemberFacts> members;
    std::vector<BaseRelation> bases;
    std::vector<ObjectAction> actions;
    Index object_actions, specialization_index, parameter_ordinals;
    Index template_families, template_signatures;
    std::vector<TypeId> canonical_parameters;
    ScopeId active_template_scope = 0;
    std::vector<TemplateFunction> templates;
    std::vector<EntityId> template_parameters;
    Index template_default_types;
    std::vector<TypeQuery> type_queries = std::vector<TypeQuery>(1);
    std::vector<QueryId> query_edges, query_slots;
    std::vector<std::uint64_t> query_hashes = std::vector<std::uint64_t>(1);
    std::vector<TypeQueryFact> query_facts = std::vector<TypeQueryFact>(1);
    Index query_sources, query_callee_sources, signature_parameters;
    std::size_t query_work = 0;
    struct QueryValue { std::uint32_t constant = 0; FactState state = FactState::NotStarted; };
    Index query_value_index, template_value_queries, template_value_dependence;
    std::vector<QueryValue> query_values = std::vector<QueryValue>(1);
    bool template_body_values = false;
    std::size_t query_value_work = 0, template_value_work = 0, template_value_uses = 0;
    std::uint32_t query_value(QueryId id);
    bool bind_template_size(NodeId node, ScopeId scope);
    bool reuse_template_value(NodeId node, ScopeId scope, Expression& result);
    bool fixed_layout_operand(NodeId node) const;
    void reuse_value_conversions(NodeId node, NodeId source, Expression& result);
    Index template_value_conversions;
    std::size_t value_conversion_variants = 0, value_conversion_records = 0;
    QueryId intern_query(TypeQuery query, const std::vector<QueryId>& children);
    QueryId expression_query(NodeId n, ScopeId s, bool callee = false);
    QueryId substitute_query(QueryId id, const Index& bindings, Index& cache, std::uint32_t owner = 0);
    TypeQueryFact query_fact(QueryId id);
    TypeId query_decltype(QueryId id, bool direct);
    TypeId fundamental_cast_type(ETokenType op);
    TypeId parameter_body_type(TypeId source);
    TypeQueryFact query_call(const TypeQuery& query, const std::vector<TypeQueryFact>& children);
    CallSelection select_call(EntityId family, const std::vector<Expression>& values,
        const std::vector<NodeId>* nodes, TypeId object, ValueCategory category,
        ScopeId naming, std::uint32_t explicit_arguments, std::vector<Conversion>& selected);
    TypeQueryFact query_operator(const TypeQuery& query, const std::vector<TypeQueryFact>& children);
    Expression conditional_value(Expression left, Expression right);
    TypeQueryFact query_conditional(const TypeQuery& query, const std::vector<TypeQueryFact>& children);
    TypeId dependent_decltype(NodeId n, ScopeId s);
    std::vector<TypeArguments> argument_packs;
    std::vector<TypeId> argument_types;
    std::vector<std::uint32_t> argument_slots;
    std::vector<Specialization> specializations;
    // A complete substitution frame and source type/query identify immutable
    // facts. Separate indexes retain the complete 32-bit ID spaces.
    Index specialization_type_cache, specialization_query_cache, substitution_binding_cache;
    Index template_declaration_sources;
    std::size_t template_declaration_work = 0, declaration_publications = 0;
    void publish_template_binding(NodeId source, EntityId concrete);
    std::size_t substitution_work = 0, substitution_hits = 0, substitution_records = 0;
    Index substitution_frame_index, template_type_contexts, substitution_frame_contexts;
    std::vector<TemplateSubstitutionFrame> substitution_frames = std::vector<TemplateSubstitutionFrame>(1);
    std::uint32_t substitution_frame(std::uint32_t specialization, std::uint32_t parameters,
        std::uint32_t count, std::uint32_t parent = 0);
    TypeId substitution_argument(std::uint32_t frame, EntityId parameter) const;
    void attach_template_context(std::uint32_t context, std::uint32_t frame);
    EntityId substitution_entity(std::uint32_t frame, EntityId source) const;
    EntityId substitution_binding(std::uint32_t frame, EntityId source);
    ScopeId substitution_scope(std::uint32_t frame, ScopeId source) const;
    bool pattern_scope(ScopeId scope) const;
    Index template_type_sources, template_signature_sources;
    bool template_type_probe = false;
    std::size_t template_type_work = 0, template_type_uses = 0;
    std::size_t template_signature_work = 0, template_signature_uses = 0, parameter_publications = 0;
    TypeId bind_template_type(NodeId specs, NodeId declarator, ScopeId scope);
    TypeId bind_template_special_type(NodeId d, ScopeId scope);
    TypeId reuse_template_type(NodeId node, ScopeId scope);
    std::vector<unsigned char> type_dependence;
    std::vector<EntityId> specialization_demand;
    std::size_t specialization_cursor = 0, template_bodies = 0, template_completions = 0;
    std::vector<EntityId> demand_queue;
    std::size_t demand_cursor = 0;
    unsigned anonymous_classes = 0, anonymous_enums = 0;
    double analysis_ms = 0;
    std::vector<Edge> edges;
    std::vector<ScopeId> qualified_work;
    std::vector<std::uint64_t> visited;
    std::uint64_t walk = 0;
    std::size_t lookup_work = 0, analyzed = 0, constant_work = 0;
    struct Body { NodeId node, declarator; ScopeId owner; EntityId entity; NodeId source; };
    struct DeferredDefault { EntityId function; unsigned parameter; };
    std::vector<DeferredDefault> declaration_defaults;
    std::vector<Body> bodies;
    unsigned class_depth = 0;
    enum class Lookup { Ordinary, Tag, Namespace, Qualifier };
    std::uint64_t key(ScopeId s, IdentifierId n) const;
    EntityId local(ScopeId s, IdentifierId n, Lookup mode = Lookup::Ordinary) const;
    EntityId lookup(ScopeId s, IdentifierId n, Lookup mode = Lookup::Ordinary, bool qualified = false);
    EntityId imported(ScopeId s, IdentifierId n, Lookup mode, std::uint64_t visit);
    EntityId merge_lookup(EntityId a, EntityId b);
    bool function_binding(EntityId e) const;
    std::vector<EntityId> candidates(EntityId e);
    EntityId declare_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type, bool constructor = false, TypeId conversion = 0);
    EntityId declare_alias(ScopeId s, IdentifierId name, NodeId source, TypeId type);
    TypeId source_type(EntityId e) const;
    TypeId type_name(NodeId n, ScopeId s, NodeId last = 0);
    TypeId injected_template_type(EntityId e, ScopeId use);
    TypeId qualified_type(TypeId owner, IdentifierId name, const std::vector<TypeId>& args, bool template_id);
    EntityId resolve(NodeId name, ScopeId s, Lookup mode = Lookup::Ordinary);
    ScopeId name_owner(NodeId name, ScopeId s, bool declaration = false);
    ScopeId common_ancestor(ScopeId a, ScopeId b) const;
    ScopeId target(EntityId e) const;
    Index operator_names;
    IdentifierId terminal(NodeId name);
    IdentifierId operator_name(ETokenType op, bool array = false);
    ETokenType operator_token(NodeId name) const;
    void declare_operator(EntityId e, NodeId name);
    bool operator_expression(NodeId n, ScopeId s, ETokenType op, std::vector<NodeId> args, Expression& result);
    NodeId decl_name(NodeId d) const;
    NodeId child(NodeId n, syntax::Kind k) const;
    bool spec_has(NodeId n, ETokenType op) const;
    bool encloses(ScopeId outer, ScopeId inner) const;
    ScopeId make_scope(ScopeKind k, ScopeId parent, IdentifierId name = 0, EntityId e = 0, bool visible = true);
    void attach_scope(ScopeId s, ScopeId parent);
    EntityId make_entity(EntityKind k, ScopeId s, IdentifierId name, NodeId source);
    void bind(ScopeId s, IdentifierId n, EntityId e);
    std::uint32_t record(ScopeId s, EntityId e, NodeId source, TypeId type, EntityKind kind);
    void add_edge(ScopeId s, ScopeId to, bool is_inline = false);
    void declaration(NodeId n, ScopeId s);
    void simple(NodeId n, ScopeId s);
    unsigned base_steps(TypeId from, EntityId to);
    TypeId implicit_object_type(ScopeId s);
    void member_facts(EntityId e);
    void virtual_declaration(EntityId e, NodeId d, NodeId init, NodeId specs, NodeId source, ScopeId s);
    void complete_virtuals(EntityId cls);
    void vtable_definition_available(EntityId e);
    void demand_vtable(EntityId cls, VtableReason reason);
    void check_covariance(EntityId e, EntityId base);
    void reject_abstract(TypeId t);
    std::vector<VirtualClass> virtual_classes = std::vector<VirtualClass>(1);
    std::vector<EntityId> key_vtable_demand, vtable_emission;
    std::size_t key_vtable_cursor = 0;
    std::size_t virtual_slot_work = 0, virtual_declaration_work = 0, virtual_demands = 0;
    Conversion object_conversion(EntityId e, TypeId object, ValueCategory category, ScopeId naming = 0);
    Expression member_value(EntityId e, unsigned object_cv, ValueCategory category);
    FunctionQualifiers function_qualifiers(NodeId parameters);
    bool prototype_scope_needed(NodeId parameters);
    void check_pointer_arithmetic(ETokenType op, TypeId left, TypeId right);
    void template_facts(EntityId e, ScopeId environment = 0);
    EntityId declare_template_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type);
    void instantiate_function(EntityId e);
    void instantiate_parameters(NodeId d, std::uint32_t context, std::uint32_t frame, ScopeId environment);
    EntityId deduce_target(EntityId pattern, TypeId target);
    bool template_more_specialized(EntityId a, EntityId b);
    NodeId instantiate_default(EntityId e, NodeId source);
    void validate_list_plan(std::uint32_t id);
    TypeId declare_class_template(NodeId n, ScopeId s);
    bool dependent_template_syntax(NodeId n, ScopeId s);
    EntityId specialize_class(EntityId pattern, const std::vector<TypeId>& args);
    EntityId class_template_name(NodeId part, EntityId e, ScopeId s);
    void complete_class(EntityId e);
    bool template_defaults(EntityId pattern, std::vector<TypeId>& args);
    ScopeId specialization_environment(EntityId e);
    bool retain_template_definition(NodeId n, ScopeId s);
    void explicit_instantiation(NodeId n, ScopeId s);
    std::uint32_t definition_root(EntityId pattern);
    std::uint32_t definition_path(std::uint32_t parent, IdentifierId name);
    TemplateDefinitionOwner definition_owner(EntityId cls);
    bool instantiate_member_definition(EntityId e);
    void demand_template_storage(EntityId e);
    Index definition_roots, definition_paths, definition_index, definition_owner_index, definition_applications, storage_requested;
    Index definition_traversals, unmatched_definitions;
    Index definition_source_parameters;
    std::vector<TemplateDefinition> template_definitions = std::vector<TemplateDefinition>(1);
    std::vector<TemplateDefinitionOwner> definition_owners = std::vector<TemplateDefinitionOwner>(2);
    std::vector<EntityId> storage_demand;
    std::uint32_t definition_path_count = 0;
    std::size_t storage_cursor = 0;
    std::size_t template_definition_work = 0, definition_direct_work = 0;
    std::size_t definition_requests = 0, definition_hits = 0, definition_edges = 0;
    std::size_t definition_signature_requests = 0, definition_signature_work = 0;
    ScopeId member_definition_environment = 0;
    Index template_binding_index, template_pattern_entities, template_pattern_scopes, template_bound_bodies;
    Index template_base_dependence, template_class_bindings, template_pattern_bases;
    std::vector<TemplateBinding> template_bindings = std::vector<TemplateBinding>(1);
    std::size_t template_binding_work = 0;
    Index template_fixed_expressions;
    std::size_t template_fixed_work = 0, template_fixed_uses = 0;
    std::size_t template_fixed_call_work = 0, template_fixed_call_uses = 0;
    void check_fixed_expression(NodeId n, ScopeId s);
    bool reuse_fixed_expression(NodeId n, ScopeId s, Expression& result);
    bool check_fixed_call(NodeId n, ScopeId s);
    bool check_fixed_member(NodeId n, ScopeId s);
    bool check_template_field(NodeId n, ScopeId s, EntityId field, bool explicit_object = false);
    void bind_template_object_context(ScopeId function, NodeId parameters);
    TypeId template_method_shape(NodeId parameters, ScopeId scope);
    TemplateObjectContext template_object_context(ScopeId scope) const;
    bool reuse_template_field(NodeId n, ScopeId scope, Expression& result);
    TemplateMemberUse template_field_use(EntityId field, ScopeId scope, EntityId pattern = 0);
    Index template_object_context_index, template_field_sources, template_member_use_index;
    Index template_method_shapes;
    std::vector<unsigned char> prototype_scope_requirements;
    std::vector<NodeId> prototype_scope_work;
    Index template_class_patterns, template_class_contexts;
    std::vector<TemplateObjectContext> template_object_contexts = std::vector<TemplateObjectContext>(1);
    std::vector<TemplateMemberUse> template_member_uses = std::vector<TemplateMemberUse>(1);
    void check_fixed_conversion(Expression source, NodeId n, Conversion& c, ScopeId s);
    Conversion copy_conversion_recipe(Conversion c);
    void reuse_fixed_call(NodeId n, NodeId source, ScopeId s, Expression& result);
    void use_selected_function(EntityId e, bool direct);
    TemplateBinding bind_template_name(NodeId n, ScopeId s, NodeId last = 0);
    bool bind_template_expression(NodeId n, ScopeId s, bool callee = false);
    bool bind_template_expression_impl(NodeId n, ScopeId s, bool callee);
    EntityId pattern_declaration(EntityKind kind, ScopeId s, IdentifierId name, NodeId source, bool dependent);
    void bind_template_declaration(NodeId n, ScopeId s, std::vector<Body>* deferred = 0, bool defaults_allowed = true);
    void bind_template_body(const Body& body);
    void bind_template_statement(NodeId n, ScopeId s);
    ScopeId bind_template_class(NodeId n, ScopeId parent, EntityId entity = 0, std::vector<Body>* deferred = 0);
    void check_template_parameters(NodeId n, ScopeId s);
    void index_template_members(NodeId n, std::uint32_t path, ScopeId s);
    TypeId template_member_signature(TypeId type, ScopeId head, EntityId primary, ScopeId owner);
    void template_signature_bindings(ScopeId scope, EntityId primary, Index& bindings);
    TypeId template_member_aliases(TypeId type, EntityId primary, Index& cache);
    ScopeId template_signature_owner(TypeId type, EntityId primary);
    std::uint32_t check_template_member_definition(NodeId d, std::uint32_t path, IdentifierId name, ScopeId head, EntityId primary);
    int template_exception(NodeId d, ScopeId s);
    Index template_prototype_index, template_prototype_sources;
    Index template_signature_index, template_signature_groups;
    struct TemplatePrototype {
        NodeId declarator; ScopeId environment; std::uint32_t next;
        TypeId signature = 0; std::uint32_t definitions = 0;
        bool inline_definition = false;
        TemplatePrototype(NodeId d = 0, ScopeId s = 0, std::uint32_t n = 0, bool defined = false)
            : declarator(d), environment(s), next(n), inline_definition(defined) {}
    };
    std::vector<TemplatePrototype> template_prototypes = std::vector<TemplatePrototype>(1);

    std::uint32_t intern_arguments(const std::vector<TypeId>& args);
    EntityId specialize(EntityId pattern, const std::vector<TypeId>& args);
    bool dependent_type(TypeId type);
    TypeId substitute_type(TypeId pattern, const Index& bindings, Index& cache, std::uint32_t owner = 0);
    bool deduce_type(TypeId pattern, TypeId actual, Index& bindings);
    EntityId deduce_function(EntityId pattern, const std::vector<NodeId>& args, unsigned begin = 0);
    EntityId deduce_function(EntityId pattern, const std::vector<Expression>& args, unsigned begin = 0);
    template<class Arguments> EntityId deduce_function_values(EntityId pattern, const Arguments& args);
    EntityId explicit_template(NodeId name, EntityId binding, ScopeId s);
    void demand_specialization(EntityId e);
    void demand_member(EntityId e, MemberDemandReason reason = MemberDemandReason::Use);
    void require_member_definition(EntityId e);
    void require_member_body(EntityId e);
    void prepare_value_initialization(TypeId t, ScopeId s = 0);
    EntityId default_constructor(TypeId t, ScopeId s = 0, bool demand = true);
    EntityId choose_constructor(TypeId t, const std::vector<NodeId>& args, Expression* result = 0, ScopeId scope = 0,
        bool direct = true, bool probe = false, const std::vector<Expression>* values = 0);
    bool converting_transfer(EntityId constructor, const Expression& call) const;
    Conversion result_conversion(EntityId constructor, const Expression& call, TypeId target);
    void constructor_actions(EntityId e);
    bool inherit_using(NodeId name, ScopeId scope);
    void inherited_constructors(EntityId cls);
    bool base_initialization = false;
    bool class_initialize(NodeId n, TypeId target, ScopeId s, InitializationMode mode);
    void default_initialize(EntityId object);
    bool derived_from(TypeId from, TypeId to);
    void write_function(std::ostream& out, EntityId e, NodeId body, ScopeId scope, unsigned depth, bool definition = true) const;
    void write_object(std::ostream& out, EntityId e, NodeId init, unsigned depth) const;
    void write_action(std::ostream& out, const ObjectAction& action, unsigned depth) const;
    void function_body(const Body& body);
    void check_jumps(NodeId body, bool binding_only = false);
    void schedule_body(const Body& body);
    void statements(NodeId n, ScopeId s);
    void resolve_statement(NodeId n, ScopeId s);
    void resolve_condition(NodeId n, ScopeId s, bool is_switch);
    TypeId condition_target(Expression value, bool is_switch);
    Expression template_statement_value(NodeId n, ScopeId s);
    bool fixed_initializer_operands(NodeId n) const;
    bool template_aggregate_type(TypeId target) const;
    Index template_pattern_aggregates;
    void bind_template_condition(NodeId n, ScopeId s, bool is_switch);
    void bind_template_return(NodeId n, ScopeId s);
    Index template_statement_conversions;
    std::size_t body_checks = 0, body_lifetime_checks = 0;
    std::size_t statement_conversion_work = 0, statement_conversion_uses = 0;
    Conversion return_conversion(NodeId source, Expression value, TypeId target, bool eligible);
    Expression expression(NodeId n, ScopeId s);
    Expression resolve_expression(NodeId n, ScopeId s);
    Expression call_expression(NodeId n, ScopeId s);
    Expression unary_expression(NodeId n, ScopeId s);
    Expression binary_expression(NodeId n, ScopeId s);
    Expression cast_expression(NodeId n, ScopeId s, TypeId target, NodeId operand);
    TypeId value_type(TypeId t);
    TypeId decay(TypeId t);
    TypeId promote_expression(NodeId n);
    TypeId promote(TypeId t);
    TypeId arithmetic_type(TypeId a, TypeId b);
    TypeId composite_pointer(TypeId a, TypeId b);
    bool object_pointer(TypeId t);
    bool arithmetic(TypeId t) const;
    bool fundamental(TypeId t, EFundamentalType f) const;
    bool pointer(TypeId t) const;
    bool null_constant(NodeId n);
    bool qualification(TypeId from, TypeId to, unsigned& added, bool intermediate_const = true);
    bool similar_type(TypeId a, TypeId b);
    Conversion conversion(NodeId n, TypeId target, bool user = true);
    Conversion conversion_value(Expression source, TypeId target, bool user = true, NodeId node = 0);
    Conversion standard_conversion(Expression source, TypeId target, NodeId node = 0);
    Conversion conversion_function(NodeId n, TypeId target, bool explicit_allowed = false, bool direct_reference = false, EntityId object = 0);
    Conversion conversion_function_value(Expression source, TypeId target, bool explicit_allowed = false, bool direct_reference = false, EntityId object = 0);
    void prepare_user_conversion(NodeId n, Conversion& conversion, ConversionUse use = ConversionUse::Temporary);
    std::vector<EntityId> conversion_candidates(TypeId source);
    EntityId conversion_lookup(ScopeId owner, TypeId target);
    Index conversion_families, conversion_bindings;
    void builtin_operators(ETokenType op, const std::vector<NodeId>& arguments, std::vector<BuiltinOperator>& results);
    void builtin_operators_values(ETokenType op, const std::vector<Expression>& arguments, std::vector<BuiltinOperator>& results, const std::vector<NodeId>* nodes = 0);
    std::vector<TypeId> builtin_operand_types(NodeId n);
    std::vector<TypeId> builtin_operand_types_value(Expression source);
    Conversion converting_constructor(NodeId n, TypeId target);
    Conversion converting_constructor_value(Expression source, TypeId target, NodeId node = 0);
    void materialize_conversion(NodeId n, Conversion& c, bool defer = false,
        ConversionUse use = ConversionUse::Temporary);
    void record_call(Expression& owner, const std::vector<NodeId>& args, std::vector<Conversion>& selected);
    bool better(const Conversion* a, const Conversion* b, std::size_t count);
    Conversion ellipsis_conversion(NodeId n);
    Conversion ellipsis_conversion_value(Expression source);
    void apply_conversion(NodeId n, Conversion& c);
    void record_conversion(Expression& owner, NodeId n, Conversion c);
    Conversion boolean_conversion(NodeId n);
    Conversion boolean_conversion_value(Expression source, NodeId node = 0);
    Expression value_fact(const Expression& source) const;
    void require_conversion(NodeId n, TypeId target, bool direct = false);
    void select_function(NodeId n, EntityId e, bool direct = true);
    void initialize(NodeId init, TypeId target, ScopeId s, InitializationMode mode = InitializationMode::Direct);
    TypeId builtin_binary(ETokenType op, NodeId a, NodeId b, Expression& result);
    void modifiable(NodeId n);
    void write_resolved(std::ostream& out, NodeId n, unsigned depth) const;
    void write_expression(std::ostream& out, NodeId n, unsigned depth, TypeId override_type = 0, ValueCategory override_category = ValueCategory::Prvalue) const;
    void write_variable(std::ostream& out, NodeId d, NodeId init, unsigned depth) const;
    void write_entity_name(std::ostream& out, EntityId e) const;
    void template_declaration(NodeId n, ScopeId s);
    void namespace_declaration(NodeId n, ScopeId s);
    TypeId class_type(NodeId n, ScopeId s, IdentifierId anonymous_name = 0, bool emit = true, bool static_union = false);
    TypeId enum_type(NodeId n, ScopeId s, IdentifierId anonymous_name = 0, bool emit = true);
    TypeId specifiers(NodeId n, ScopeId s, IdentifierId anonymous_name = 0);
    TypeId type_id(NodeId n, ScopeId s);
    TypeId declarator(NodeId n, TypeId base, ScopeId s, NodeId dynamic_array = 0, bool name_resolved = false);
    TypeId parameter(NodeId n, ScopeId s);
    EntityId declare_object(NodeId d, NodeId init, TypeId t, NodeId specs, ScopeId s, NodeId source);
    void declaration_attributes(EntityId e, NodeId specs, NodeId source);
    Constant evaluate(NodeId n, ScopeId s);
    Constant evaluate_value(NodeId n, ScopeId s);
    Constant binary(ETokenType op, Constant a, Constant b, bool converted = false);
    Constant convert(Constant value, TypeId to, bool explicit_cast = false);
    TypeId expression_type(NodeId n, ScopeId s, bool decltype_form = false);
    std::uint64_t size(TypeId t, bool alignment = false);
    bool integral(TypeId t) const;
    bool is_unsigned(TypeId t) const;
    bool scoped_enum(TypeId t) const;
    unsigned width(TypeId t) const;
    void write_scope(std::ostream& out, ScopeId s, unsigned depth) const;
    void write_type(std::ostream& out, TypeId t, NodeId display_name = 0, ETokenType key = TOK_INVALID, TypeId completed = 0) const;
    void write_name(std::ostream& out, NodeId n) const;
    void spelling(std::ostream& out, IdentifierId n) const;
};
} }
