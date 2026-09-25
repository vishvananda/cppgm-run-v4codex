#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
EntityId Analyzer::choose_constructor(TypeId t, const std::vector<NodeId>& args, Expression* result,
    ScopeId scope, bool direct, bool probe, const std::vector<Expression>* values)
{
    if (values && (!probe || values->size() != args.size())) throw std::logic_error("invalid constructor value probe");
    auto value = [&](unsigned i) { return values ? (*values)[i] : expressions[args[i]]; };
    EntityId cls = types[t].entity;
    if (definitions) complete_class(cls);
    if (args.size() == 1 && types[value(0).type].kind == TypeKind::Named &&
        (types.unqualified(value(0).type) == types.unqualified(t) || derived_from(value(0).type, t) || class_value(value(0).type)))
        ensure_transfers(t, false);
    if (args.empty() && !class_facts[entities[cls].class_info].user_constructor)
        return default_constructor(t, scope, !probe);
    EntityId binding = class_facts[entities[cls].class_info].constructor;
    if (!binding) {
        if (!args.empty()) { if (probe) return 0; throw std::runtime_error("no matching constructor"); }
        return default_constructor(t, scope, !probe);
    }
    struct Viable { EntityId entity; std::size_t offset; };
    std::vector<Viable> viable;
    std::vector<Conversion> sequences;
    for (EntityId e : candidates(binding)) {
        ++candidate_work;
        if (entities[e].template_info) {
            e = values ? deduce_function(e,*values) : deduce_function(e,args);
            if (!e) continue;
        }
        if (!direct && members[entities[e].member_info].explicit_constructor) continue;
        Type f = types[entities[e].type];
        if ((!f.variadic && args.size() > f.count) || (args.size() < f.count &&
            (!entities[e].defaults || !default_arguments[entities[e].defaults + args.size()]))) continue;
        if (members[entities[e].member_info].transfer == TransferKind::MoveConstructor &&
            members[entities[e].member_info].synthetic && deleted_transfer(e)) continue;
        std::size_t begin = sequences.size();
        bool valid = true;
        for (std::size_t i = 0; valid && i < args.size(); ++i) {
            Conversion c;
            auto typed = values && !expressions[args[i]].ready;
            if (direct && !i && transfer_member(e) && class_value(value(i).type) &&
                types.unqualified(value(i).type) != types.unqualified(t))
                c = typed ? conversion_function_value(value(i),types.parameters[f.offset+i],true) :
                    conversion_function(args[i],types.parameters[f.offset+i],true);
            if (!c.valid()) c = i < f.count ? (typed ? conversion_value(value(i),types.parameters[f.offset+i]) :
                conversion(args[i], types.parameters[f.offset+i])) : (typed ? ellipsis_conversion_value(value(i)) : ellipsis_conversion(args[i]));
            valid = c.valid(); sequences.push_back(c);
        }
        if (valid) viable.push_back({e, begin}); else sequences.resize(begin);
    }
    if (viable.empty()) { if (probe) return 0; throw std::runtime_error("no viable constructor"); }
    auto preferred = [&](std::size_t a, std::size_t b) {
        auto x = sequences.data()+viable[a].offset, y = sequences.data()+viable[b].offset;
        if (better(x,y,args.size())) return true;
        // A template tie-break is available only when no argument conversion
        // is worse. Crossed conversion advantages do not constitute a tie.
        for (std::size_t i = 0; i < args.size(); ++i)
            if (better(y+i,x+i,1)) return false;
        auto ea = viable[a].entity, eb = viable[b].entity;
        return (!entities[ea].specialization && entities[eb].specialization) || template_more_specialized(ea,eb,args.size());
    };
    std::size_t best = 0;
    for (std::size_t i = 1; i < viable.size(); ++i)
        if (preferred(i,best)) best = i;
    for (std::size_t i = 0; i < viable.size(); ++i)
        if (i != best && !preferred(best,i)) {
            if (probe) { if (result) result->form = ExpressionForm::Overload; return 0; }
            throw std::runtime_error("ambiguous constructor");
        }
    EntityId selected = viable[best].entity;
    if (!probe && deleted_transfer(selected)) throw std::runtime_error("deleted constructor");
    if (!probe) check_default_constructor(selected);
    EntityId access = selected;
    while (members[entities[access].member_info].inherited_constructor)
        access = members[entities[access].member_info].inherited_constructor;
    if (!probe) check_access(access, scope, entities[access].owner);
    auto member = entities[selected].member_info;
    if (!probe && !result && members[member].default_conversions) { demand_member(selected); return selected; }
    Type f = types[entities[selected].type];
    std::vector<NodeId> arguments;
    std::vector<Conversion> selected_arguments;
    auto count = probe ? args.size() : std::max<std::size_t>(args.size(), f.count);
    for (std::size_t i = 0; i < count; ++i) {
        Conversion c;
        NodeId arg = i < args.size() ? args[i] : default_argument(selected,i,&c,probe ? DefaultReason::Recipe : DefaultReason::Argument);
        if (i < args.size()) c = sequences[viable[best].offset+i];
        if (!c.valid()) { if (probe) return 0; throw std::runtime_error("invalid constructor default argument"); }
        arguments.push_back(arg); selected_arguments.push_back(c);
    }
    if (probe) { if (result) store_call(*result,arguments,selected_arguments); return selected; }
    if (result) record_call(*result, arguments, selected_arguments);
    else {
        Expression defaults;
        record_call(defaults, arguments, selected_arguments);
        members[member].default_conversions = defaults.conversions;
    }
    if (!result || !converting_transfer(selected,*result)) demand_member(selected);
    return selected;
}
EntityId Analyzer::default_constructor(TypeId t, ScopeId s, bool demand)
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) return 0;
    if (definitions) complete_class(types[t].entity);
    EntityId cls = types[t].entity;
    auto c = entities[cls].class_info;
    if (class_facts[c].constructor && class_facts[c].user_constructor) return choose_constructor(t, {}, 0, s, true, !demand);
    EntityId ctor = class_facts[c].implicit_constructor;
    if (!ctor) {
        ctor = make_entity(EntityKind::Function, entities[cls].scope, entities[cls].name, 0);
        entities[ctor].type = types.function(types.fundamental(FT_VOID), {}, false);
        entities[ctor].inline_function = true;
        member_facts(ctor);
        auto m = entities[ctor].member_info;
        members[m].synthetic = members[m].constructor = true;
        members[m].deleted = closure(cls).function != 0;
        class_facts[c].implicit_constructor = ctor;
    }
    if (demand) {
        if (members[entities[ctor].member_info].deleted) throw std::runtime_error("deleted default constructor");
        check_default_constructor(ctor); demand_member(ctor);
    }
    return ctor;
}
bool Analyzer::class_initialize(NodeId n, TypeId target, ScopeId s, InitializationMode mode)
{
    NodeId list = ast[n].kind == Kind::Initializer ? ast[n].first : n;
    bool copy = mode == InitializationMode::Copy;
    auto info = entities[types[target].entity].class_info;
    if (ast[list].kind == Kind::BracedInit && class_facts[info].aggregate) return false;
    if (!copy && ast[list].kind == Kind::Call) {
        NodeId callee = ast[list].first;
        EntityId named = ast[callee].kind == Kind::IdExpression ? resolve(ast[callee].detail, s) : 0;
        if (named && (entities[named].kind == EntityKind::Type || entities[named].kind == EntityKind::Alias) &&
            types.unqualified(entities[named].type) == types.unqualified(target)) list = ast[callee].next;
    }
    std::vector<NodeId> args;
    bool grouped = ast[list].kind == Kind::Arguments || ast[list].kind == Kind::ParenInitializer || ast[list].kind == Kind::ParenArguments || ast[list].kind == Kind::BracedInit;
    for (NodeId a = grouped ? ast[list].first : list; a; a = grouped ? ast[a].next : 0) {
        Expression value = expression(a, s);
        if (!grouped && copy) return record_class_initialization(n,target,a);
        if (!grouped && value.category == ValueCategory::Prvalue && types.unqualified(value.type) == types.unqualified(target)) {
            auto retained = retained_initialization(a,target);
            auto c = retained ? copy_conversion_recipe(conversions[retained]) : transfer_initialization(value,target,mode);
            return record_class_initialization(n,target,a,&c);
        }
        args.push_back(a);
    }
    Expression result; result.type = target; result.ready = true; result.evaluated = true;
    EntityId ctor = 0;
    bool reused = reuse_template_constructor(n,target,args,result,s,ctor);
    if (!reused)
        ctor = choose_constructor(target, args, &result, s, !copy || ast[list].kind == Kind::BracedInit);
    if (copy && members[entities[ctor].member_info].explicit_constructor)
        throw std::runtime_error("explicit constructor in copy-list initialization");
    if (converting_transfer(ctor,result)) {
        auto c = result_conversion(ctor,result,target);
        ValueInitialization init; init.source = args[0]; init.conversion = conversions.size(); conversions.push_back(c);
        class_initializer_index.put(key(n,target),value_initializations.size()); value_initializations.push_back(init);
        facts.edit(n).type = target; return true;
    }
    if (!reused && ast[list].kind == Kind::BracedInit) {
        Type f = types[entities[ctor].type];
        for (std::size_t j = 0; j < args.size() && j < f.count; ++j)
            list_conversion(args[j], value_type(types.parameters[f.offset+j]));
    }
    if (!base_initialization) members[entities[ctor].member_info].complete_entry = true;
    if (args.empty() && grouped && members[entities[ctor].member_info].synthetic && !members[entities[ctor].member_info].defaulted_late) {
        prepare_zero_initialization(entities[scopes[entities[ctor].owner].entity].type);
        record_object(result, 0, target, 0); object_uses[result.object_use].value_initialize = true;
    }
    { auto& published = facts.edit(n); published.entity = ctor; published.type = target; published.scope = s; }
    expressions.set(n,result);
    return true;
}
void Analyzer::constructor_actions(EntityId e)
{
    auto m = entities[e].member_info;
    if (members[m].actions_state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::ConstructorActions,e,members[m].source);
    if (members[m].actions_state != FactState::NotStarted) return;
    members[m].actions_state = FactState::Active;
    bool saved_base = base_initialization;
    try {
    EntityId cls = scopes[entities[e].owner].entity;
    ScopeId scope = entities[e].scope;
    if (!scope) {
        scope = make_scope(ScopeKind::Function, entities[e].owner, entities[e].name, e);
        entities[e].scope = scope;
    }
    EntityId inherited = members[m].inherited_constructor;
    if (inherited) {
        inherited_forwarding(e);
        Type f = types[entities[e].type];
        for (unsigned j = 0; j < f.count; ++j) {
            EntityId p = make_entity(EntityKind::Parameter, scope, 0, 0);
            entities[p].type = types.parameters[f.offset+j];
            record(scope, p, 0, entities[p].type, EntityKind::Parameter);
            inherited_arguments[members[m].inherited_arguments+j].parameter = p;
            if (class_value(entities[p].type)) register_destruction(p);
        }
        auto target = types[entities[inherited].type];
        for (unsigned j = 0; j < target.count; ++j) {
            auto argument = inherited_arguments[members[m].inherited_arguments+j];
            if (argument.transfer) {
                members[entities[argument.transfer].member_info].complete_entry = true;
                demand_member(argument.transfer);
                auto transfer = types[entities[argument.transfer].type];
                for (unsigned k = 1; k < transfer.count; ++k) {
                    Conversion c; auto n = default_argument(argument.transfer,k,&c);
                    apply_conversion(n,c);
                    conversions[argument.transfer_defaults+k-1] = c;
                }
            }
            if (argument.value) {
                Conversion c; auto n = default_argument(inherited,j,&c);
                apply_conversion(n,c);
                inherited_arguments[members[m].inherited_arguments+j].conversion = conversions.size();
                conversions.push_back(c);
            }
        }
    }
    size(entities[cls].type);
    Index explicit_initializers;
    NodeId list = child(members[m].source, Kind::CtorInitializer);
    demand_region(list);
    expand_expression_list(list,scope);
    for (NodeId n = ast[list].first; n; n = ast[n].next) {
        NodeId id = child(n, Kind::MemInitializerId);
        EntityId field = resolve(ast[id].detail, expanded_scope(n,entities[e].owner));
        if (!field) throw std::runtime_error("unknown constructor initializer");
        if (entities[field].kind == EntityKind::Alias) field = types[entities[field].type].entity;
        if (field == cls) {
            if (ast[list].first != ast[list].last) throw std::runtime_error("delegation must be the only initializer");
            NodeId init = ast[id].next;
            initialize(init, entities[cls].type, scope);
            EntityId selected = facts[init].entity;
            if (!constructor_member(selected)) throw std::logic_error("missing delegation target");
            members[m].delegated_constructor = selected;
            members[m].action_begin = subobject_actions.size(); members[m].action_count = 1;
            subobject_actions.push_back({0, entities[cls].type, init, selected});
            members[m].actions_state = FactState::Success;
            return;
        }
        if (explicit_initializers.get(field)) throw std::runtime_error("duplicate constructor initializer");
        bool direct_member = nonstatic_field(field) && entities[field].owner == entities[cls].scope;
        bool direct_base = false;
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next)
            direct_base |= bases[b].base == field;
        if (!direct_member && !direct_base)
            throw std::runtime_error("initializer does not name a member or base");
        explicit_initializers.put(field, ast[id].next);
    }
    std::vector<SubobjectAction> work;
    EntityId variant = 0;
    if (entities[cls].key == KW_UNION) {
        for (NodeId n = ast[list].first; n; n = ast[n].next) {
            EntityId field = resolve(ast[child(n, Kind::MemInitializerId)].detail, entities[e].owner);
            if (variant) throw std::runtime_error("multiple initialized union variants");
            variant = field;
        }
        if (!variant) variant = class_facts[entities[cls].class_info].variant_initializer;
    }
    auto add = [&](EntityId field, TypeId type, NodeId initial) {
        EntityId ctor = 0;
        if (!initial && field && class_value(type)) {
            auto info = entities[types[type].entity].class_info;
            // An anonymous union with no chosen variant has no subobject to
            // initialize in a user-provided enclosing constructor. Defaulted
            // enclosing constructors check variant deletion separately.
            if (class_facts[info].storage == field && !class_facts[info].variant_initializer) return;
        }
        default_destructor(type, scope);
        bool saved_base = base_initialization;
        base_initialization = !field;
        if (initial) initialize(initial, type, scope);
        else if (types[type].kind == TypeKind::Array || (types[type].kind == TypeKind::Named && entities[types[type].entity].class_info)) ctor = default_constructor(type, scope);
        else if (types[type].kind == TypeKind::LRef || types[type].kind == TypeKind::RRef || (types[type].cv & 1))
            throw std::runtime_error("uninitialized reference or const member");
        base_initialization = saved_base;
        if (initial) {
            ctor = facts[initial].entity;
            if (class_initialization(initial, type).source) {
                auto c = conversions[class_initialization(initial, type).conversion];
                if (c.kind == Conversion::Kind::Construction) ctor = conversion_objects[c.materialization].constructor;
            }
            if (!constructor_member(ctor)) ctor = 0;
        }
        if (!field) {
            EntityId selected = ctor;
            if (selected && entities[selected].member_info) {
                members[entities[selected].member_info].base_entry = true;
                members[entities[selected].member_info].polymorphic_base_entry |= polymorphic(cls);
            }
        } else if (ctor) members[entities[ctor].member_info].complete_entry = true;
        if (initial || ctor) work.push_back({field, type, initial, ctor});
    };
    for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) {
        EntityId base = bases[b].base;
        if (inherited && base == scopes[entities[inherited].owner].entity) {
            default_destructor(entities[base].type, scope);
            members[entities[inherited].member_info].base_entry = true;
            demand_member(inherited);
            work.push_back({0, entities[base].type, 0, inherited});
        } else add(0, entities[base].type, explicit_initializers.get(base));
    }
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        EntityId field = declarations[d].entity;
        if (!nonstatic_field(field) || entities[field].owner != entities[cls].scope) continue;
        if (entities[cls].key == KW_UNION && field != variant) continue;
        NodeId init = explicit_initializers.get(field);
        if (!init) init = entities[field].initializer;
        add(field, entities[field].type, init);
    }
    members[m].action_begin = subobject_actions.size(); members[m].action_count = work.size();
    subobject_actions.insert(subobject_actions.end(), work.begin(), work.end());
    members[m].actions_state = FactState::Success;
    } catch (...) {
        base_initialization = saved_base;
        members[m].actions_state = FactState::Failure; throw;
    }
}
} }

namespace cppgm { namespace semantic {
bool Analyzer::constructor_needed(EntityId e)
{
    if (!e) return false;
    auto m = entities[e].member_info;
    if (members[m].default_properties == BooleanFact::Failure)
        throw FailedSemanticFact(SemanticFact::DefaultConstructorProperties,e,entities[e].source);
    if (members[m].actions_state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::ConstructorActions,e,members[m].source);
    if (transfer_member(e) && members[m].synthetic) {
        if (members[m].transfer_state == FactState::Failure)
            throw FailedSemanticFact(SemanticFact::Transfer,e,members[m].source);
        if (members[m].transfer_state != FactState::Success)
            throw UnavailableSemanticFact(SemanticFact::Transfer,e,members[m].source);
        return !members[m].constructor || !members[m].transfer_direct;
    }
    if (!members[m].synthetic || members[m].inherited_constructor) return true;
    // Omission needs a completed action plan; an external/user-provided entry
    // is required independently of that plan and was handled above.
    if (members[m].actions_state != FactState::Success)
        throw UnavailableSemanticFact(SemanticFact::ConstructorActions,e,members[m].source);
    auto state = members[m].constructor_effects;
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Failure)
        throw FailedSemanticFact(SemanticFact::ConstructorEffects,e,members[m].source);
    if (state == BooleanFact::Active) throw std::logic_error("cyclic constructor actions");
    members[m].constructor_effects = BooleanFact::Active;
    try {
    bool needed = polymorphic(scopes[entities[e].owner].entity);
    for (unsigned j = 0; j < members[m].action_count; ++j) {
        auto action = subobject_actions[members[m].action_begin+j];
        if (action.initializer) {
            EntityId ctor = facts[action.initializer].entity;
            needed |= ctor && constructor_member(ctor) ? constructor_needed(ctor) : true;
        } else needed |= constructor_needed(action.constructor);
    }
    members[m].constructor_effects = needed ? BooleanFact::True : BooleanFact::False;
    return needed;
    } catch (const UnavailableSemanticFact&) {
        members[m].constructor_effects = BooleanFact::NotStarted; throw;
    } catch (...) {
        members[m].constructor_effects = BooleanFact::Failure; throw;
    }
}
} }
