#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::check_default_constructor(EntityId e)
{
    if (!default_constructor_valid(e)) throw FailedSemanticFact(SemanticFact::DefaultConstructorProperties,e,entities[e].source);
    return members[entities[e].member_info].default_properties == BooleanFact::True;
}
bool Analyzer::default_constructor_valid(EntityId e)
{
    auto m = entities[e].member_info;
    if (!m || !members[m].constructor || transfer_member(e) || members[m].inherited_constructor || !members[m].synthetic)
        return true;
    auto state = members[m].default_properties;
    if (state == BooleanFact::True || state == BooleanFact::False) return true;
    if (state == BooleanFact::Failure) return false;
    if (state == BooleanFact::Active) return false;
    members[m].default_properties = BooleanFact::Active;
    try {
        auto cls = scopes[entities[e].owner].entity;
        complete_class(cls);
        if (!entities[cls].complete) throw UnavailableSemanticFact(SemanticFact::ClassDefinition,cls,entities[cls].source);
        auto scope = entities[cls].scope;
        auto info = entities[cls].class_info;
        bool valid = true;
        bool trivial = !members[m].defaulted_late && !polymorphic(cls), const_default = true;
        auto subobject = [&](TypeId type, bool initialized, bool variant, bool mutable_field) {
            while (types[type].kind == TypeKind::Array) type = types[type].child;
            if (!default_destruction_valid(type,scope)) { valid = false; return; }
            if (initialized) { trivial = false; return; }
            auto kind = types[type].kind;
            if (kind == TypeKind::LRef || kind == TypeKind::RRef)
                { valid = false; return; }
            EntityId ctor = 0;
            if (class_value(type)) {
                ctor = default_constructor(type,scope,false);
                if (!ctor || deleted_transfer(ctor) || !accessible(ctor,scope,entities[ctor].owner) || !default_constructor_valid(ctor))
                    { valid = false; return; }
                bool child_trivial = members[entities[ctor].member_info].default_properties == BooleanFact::True;
                if (variant && !child_trivial) { valid = false; return; }
                trivial &= child_trivial;
            }
            bool const_child = ctor && (!members[entities[ctor].member_info].synthetic ||
                members[entities[ctor].member_info].defaulted_late || members[entities[ctor].member_info].const_default);
            const_default &= const_child || mutable_field;
            if (!variant && (types[type].cv & 1) && (!ctor ||
                !const_child))
                valid = false;
        };
        for (auto b = class_facts[info].first_base; b; b = bases[b].next)
            subobject(entities[bases[b].base].type,false,false,false);
        bool has_variant = false, all_const = true;
        for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (!nonstatic_field(field) || entities[field].owner != scope) continue;
            auto type = entities[field].type;
            bool variant = entities[cls].key == KW_UNION;
            if (variant) {
                has_variant = true;
                while (types[type].kind == TypeKind::Array) type = types[type].child;
                all_const &= (types[type].cv & 1) != 0;
            }
            subobject(type,entities[field].initializer != 0,variant,entities[field].mutable_field);
        }
        if (has_variant && all_const) valid = false;
        members[m].const_default = const_default;
        members[m].default_properties = valid ? (trivial ? BooleanFact::True : BooleanFact::False) : BooleanFact::Failure;
        return valid;
    } catch (const UnavailableSemanticFact&) {
        members[m].default_properties = BooleanFact::NotStarted; throw;
    } catch (...) { members[m].default_properties = BooleanFact::Failure; throw; }
}
EntityId Analyzer::check_default_initialization(TypeId type, ScopeId scope)
{
    while (types[type].kind == TypeKind::Array) type = types[type].child;
    auto kind = types[type].kind;
    if (kind == TypeKind::LRef || kind == TypeKind::RRef || fundamental(type,FT_VOID))
        throw std::runtime_error("invalid default initialized object type");
    EntityId ctor = 0;
    if (class_value(type)) {
        complete_class(types[type].entity); reject_abstract(type);
        ctor = default_constructor(type,scope,false);
        if (!ctor || deleted_transfer(ctor)) throw std::runtime_error("deleted default constructor");
        auto access = ctor;
        while (members[entities[access].member_info].inherited_constructor)
            access = members[entities[access].member_info].inherited_constructor;
        check_access(access,scope,entities[access].owner);
        check_default_constructor(ctor);
        default_destructor(type,scope,false);
    }
    if ((types[type].cv & 1) && (!ctor ||
        (members[entities[ctor].member_info].synthetic && !members[entities[ctor].member_info].defaulted_late &&
         !members[entities[ctor].member_info].const_default)))
        throw std::runtime_error("const object requires initialization");
    return ctor;
}
void Analyzer::bind_template_default_initialization(EntityId e, ScopeId scope)
{
    auto type = entities[e].type;
    auto element = type;
    while (types[element].kind == TypeKind::Array) element = types[element].child;
    if (types[element].kind == TypeKind::Named && template_pattern_aggregates.get(types[element].entity) &&
        !entities[types[element].entity].class_info) {
        check_pattern_default_initialization(element,scope); return;
    }
    if (!type || dependent_type(type)) return;
    struct Recipe { unsigned& depth; Recipe(unsigned& d) : depth(d) { ++depth; } ~Recipe() { --depth; } } guard(unevaluated_depth);
    auto ctor = check_default_initialization(type,scope);
    if (!ctor) return;
    auto f = types[entities[ctor].type];
    for (unsigned i = 0; i < f.count; ++i) default_argument(ctor,i,0,DefaultReason::Recipe);
    template_default_constructors.put(e,ctor); ++default_initialization_work;
}
void Analyzer::prepare_default_call(EntityId ctor)
{
    auto m = entities[ctor].member_info;
    if (members[m].default_conversions) return;
    auto f = types[entities[ctor].type];
    std::vector<NodeId> args; std::vector<Conversion> selected;
    for (unsigned i = 0; i < f.count; ++i) {
        Conversion c; args.push_back(default_argument(ctor,i,&c)); selected.push_back(c);
    }
    Expression call; record_call(call,args,selected);
    members[m].default_conversions = call.conversions;
}
bool Analyzer::pattern_class_type(TypeId type) const
{
    return types[type].kind == TypeKind::Named && template_pattern_aggregates.get(types[type].entity) &&
        !entities[types[type].entity].class_info;
}
void Analyzer::bind_pattern_member(EntityId e, NodeId declaration, NodeId declarator)
{
    auto scope = entities[e].owner, cls = scopes[scope].entity;
    if (scopes[scope].kind != ScopeKind::Class || !entities[cls].type || entities[cls].class_info) return;
    auto name = decl_name(declarator);
    bool destructor = ast[ast[name].last].op == OP_COMPL;
    bool constructor = !destructor && terminal(name) == entities[cls].name;
    member_facts(e);
    auto m = entities[e].member_info;
    members[m].constructor = constructor; members[m].destructor = destructor;
    explicit_specifier(e,declaration,scope);
    auto init = entities[e].initializer;
    if (!init) init = child(declaration,Kind::Initializer);
    auto special = child(init,Kind::SpecialInitializer);
    members[m].deleted = special && ast[special].op == KW_DELETE;
    members[m].synthetic = special && ast[special].op == KW_DEFAULT;
    if (init && !special) template_pattern_members.put(key(cls,unsigned(PatternMemberKind::PureVirtual)),e); // An explicitly pure member makes this source class abstract.
    function_defaults(e,declarator,scope,declaration);
    if (constructor || destructor) {
        auto k = key(cls,unsigned(destructor ? PatternMemberKind::Destructor : PatternMemberKind::Constructors));
        template_pattern_members.put(k,destructor ? e : merge_lookup(template_pattern_members.get(k),e));
    }
}
void Analyzer::check_pattern_default_initialization(TypeId type, ScopeId scope, bool base)
{
    auto cls = types[type].entity;
    if (template_pattern_aggregates.get(cls) == 3) throw std::runtime_error("incomplete local default object");
    if (!base && template_pattern_members.get(key(cls,unsigned(PatternMemberKind::PureVirtual)))) throw std::runtime_error("abstract local default object");
    auto family = template_pattern_members.get(key(cls,unsigned(PatternMemberKind::Constructors)));
    EntityId selected = 0;
    for (auto e : candidates(family)) {
        auto f = types[entities[e].type];
        if (f.count && (!entities[e].defaults || !default_arguments[entities[e].defaults])) continue;
        if (selected) throw std::runtime_error("ambiguous local default constructor");
        selected = e;
    }
    if (family && !selected) throw std::runtime_error("missing local default constructor");
    if (selected) {
        if (members[entities[selected].member_info].deleted) throw std::runtime_error("deleted local default constructor");
        check_access(selected,scope,entities[selected].owner);
    }
    auto cs = entities[cls].scope;
    check_pattern_destruction(cls,scope);
    if (selected && !members[entities[selected].member_info].synthetic) return;
    auto property = key(type,unsigned(PatternPropertyKind::DefaultInitialization));
    auto state = FactState(template_pattern_property_states.get(property));
    if (state == FactState::Success) return;
    if (state == FactState::Failure) throw FailedSemanticFact(SemanticFact::DefaultConstructorProperties,cls,entities[cls].source);
    if (state == FactState::Active) throw std::runtime_error("recursive local default properties");
    template_pattern_property_states.put(property,unsigned(FactState::Active));
    try {
        for (auto b = template_pattern_bases.get(cls); b; b = bases[b].next) {
            auto base = entities[bases[b].base].type;
            if (pattern_class_type(base)) { check_pattern_default_initialization(base,cs,true); continue; }
            auto ctor = default_constructor(base,cs,false);
            if (!ctor || deleted_transfer(ctor)) throw std::runtime_error("invalid local default base constructor");
            check_access(ctor,cs,entities[ctor].owner);
            check_default_constructor(ctor); default_destructor(base,cs,false);
        }
        for (auto d = scopes[cs].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (!nonstatic_field(field) || entities[field].initializer) continue;
            auto target = initialized_field_type(type,field);
            while (types[target].kind == TypeKind::Array) target = types[target].child;
            if (types[target].kind == TypeKind::Named && template_pattern_aggregates.get(types[target].entity) &&
                !entities[types[target].entity].class_info) check_pattern_default_initialization(target,cs);
            else if (!dependent_type(target)) check_default_initialization(target,cs);
        }
        template_pattern_property_states.put(property,unsigned(FactState::Success));
    } catch (...) { template_pattern_property_states.put(property,unsigned(FactState::Failure)); throw; }
}
} }
