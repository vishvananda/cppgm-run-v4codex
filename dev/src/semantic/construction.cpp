#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
EntityId Analyzer::choose_constructor(TypeId t, const std::vector<NodeId>& args, Expression* result, ScopeId scope)
{
    EntityId cls = types[t].entity;
    EntityId binding = class_facts[entities[cls].class_info].constructor;
    if (!binding) {
        if (!args.empty()) throw std::runtime_error("no matching constructor");
        return default_constructor(t, scope);
    }
    struct Viable { EntityId entity; std::size_t offset; };
    std::vector<Viable> viable;
    std::vector<Conversion> sequences;
    for (EntityId e : candidates(binding)) {
        ++candidate_work;
        Type f = types[entities[e].type];
        if ((!f.variadic && args.size() > f.count) || (args.size() < f.count &&
            (!entities[e].defaults || !default_arguments[entities[e].defaults + args.size()]))) continue;
        std::size_t begin = sequences.size();
        bool valid = true;
        for (std::size_t i = 0; valid && i < args.size(); ++i) {
            Conversion c = i < f.count ? conversion(args[i], types.parameters[f.offset+i]) : ellipsis_conversion(args[i]);
            valid = c.valid(); sequences.push_back(c);
        }
        if (valid) viable.push_back({e, begin}); else sequences.resize(begin);
    }
    if (viable.empty()) throw std::runtime_error("no viable constructor");
    std::size_t best = 0;
    for (std::size_t i = 1; i < viable.size(); ++i)
        if (better(sequences.data()+viable[i].offset, sequences.data()+viable[best].offset, args.size())) best = i;
    for (std::size_t i = 0; i < viable.size(); ++i)
        if (i != best && !better(sequences.data()+viable[best].offset, sequences.data()+viable[i].offset, args.size()))
            throw std::runtime_error("ambiguous constructor");
    EntityId selected = viable[best].entity;
    if (members[entities[selected].member_info].deleted) throw std::runtime_error("deleted constructor");
    EntityId access = selected;
    while (members[entities[access].member_info].inherited_constructor)
        access = members[entities[access].member_info].inherited_constructor;
    check_access(access, scope, entities[access].owner);
    Type f = types[entities[selected].type];
    std::vector<NodeId> arguments;
    std::vector<Conversion> selected_arguments;
    for (std::size_t i = 0; i < std::max<std::size_t>(args.size(), f.count); ++i) {
        NodeId arg = i < args.size() ? args[i] : default_arguments[entities[selected].defaults+i];
        Conversion c = i < args.size() ? sequences[viable[best].offset+i] : conversion(arg, types.parameters[f.offset+i]);
        if (!c.valid()) throw std::runtime_error("invalid constructor default argument");
        if (result) { arguments.push_back(arg); selected_arguments.push_back(c); }
        else apply_conversion(arg, c);
    }
    if (result) record_call(*result, arguments, selected_arguments);
    demand_member(selected);
    return selected;
}
EntityId Analyzer::default_constructor(TypeId t, ScopeId s)
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) return 0;
    EntityId cls = types[t].entity;
    auto c = entities[cls].class_info;
    if (class_facts[c].constructor) return choose_constructor(t, {}, 0, s);
    EntityId ctor = class_facts[c].implicit_constructor;
    if (!ctor) {
        ctor = make_entity(EntityKind::Function, entities[cls].scope, entities[cls].name, 0);
        entities[ctor].type = types.function(types.fundamental(FT_VOID), {}, false);
        entities[ctor].inline_function = true;
        member_facts(ctor);
        auto m = entities[ctor].member_info;
        members[m].synthetic = members[m].constructor = true;
        class_facts[c].implicit_constructor = ctor;
    }
    demand_member(ctor);
    return ctor;
}
bool Analyzer::class_initialize(NodeId n, TypeId target, ScopeId s)
{
    NodeId list = ast[n].kind == Kind::Initializer ? ast[n].first : n;
    auto info = entities[types[target].entity].class_info;
    if (ast[list].kind == Kind::BracedInit && class_facts[info].aggregate) return false;
    if (ast[list].kind == Kind::Call) {
        NodeId callee = ast[list].first;
        EntityId named = ast[callee].kind == Kind::IdExpression ? resolve(ast[callee].detail, s) : 0;
        if (named && (entities[named].kind == EntityKind::Type || entities[named].kind == EntityKind::Alias) &&
            types.unqualified(entities[named].type) == types.unqualified(target)) list = ast[callee].next;
    }
    std::vector<NodeId> args;
    bool grouped = ast[list].kind == Kind::Arguments || ast[list].kind == Kind::ParenInitializer || ast[list].kind == Kind::ParenArguments || ast[list].kind == Kind::BracedInit;
    for (NodeId a = grouped ? ast[list].first : list; a; a = grouped ? ast[a].next : 0) {
        Expression value = expression(a, s);
        if (!grouped && types.unqualified(value.type) == types.unqualified(target)) return false;
        args.push_back(a);
    }
    Expression result; result.type = target; result.ready = true; result.evaluated = true;
    EntityId ctor = choose_constructor(target, args, &result, s);
    if (ast[list].kind == Kind::BracedInit) {
        Type f = types[entities[ctor].type];
        for (std::size_t j = 0; j < args.size() && j < f.count; ++j)
            list_conversion(args[j], value_type(types.parameters[f.offset+j]));
    }
    if (!base_initialization) members[entities[ctor].member_info].complete_entry = true;
    // The initializer wrapper retains its starting token, including '='.
    bool copy = ast[n].kind == Kind::Initializer && (ast[n].flags & 1);
    if (copy && members[entities[ctor].member_info].explicit_constructor)
        throw std::runtime_error("explicit constructor in copy initialization");
    if (args.empty() && grouped && members[entities[ctor].member_info].synthetic) {
        record_object(result, 0, target, 0); object_uses[result.object_use].value_initialize = true;
    }
    facts[n].entity = ctor; facts[n].type = target; facts[n].scope = s;
    expressions[n] = result;
    return true;
}
void Analyzer::constructor_actions(EntityId e)
{
    auto m = entities[e].member_info;
    if (members[m].actions_ready) return;
    members[m].actions_ready = true;
    EntityId cls = scopes[entities[e].owner].entity;
    ScopeId scope = entities[e].scope;
    if (!scope) {
        scope = make_scope(ScopeKind::Function, entities[e].owner, entities[e].name, e);
        entities[e].scope = scope;
    }
    EntityId inherited = members[m].inherited_constructor;
    if (inherited) {
        Type f = types[entities[e].type];
        for (unsigned j = 0; j < f.count; ++j) {
            EntityId p = make_entity(EntityKind::Parameter, scope, 0, 0);
            entities[p].type = types.parameters[f.offset+j];
            record(scope, p, 0, entities[p].type, EntityKind::Parameter);
        }
    }
    size(entities[cls].type);
    Index explicit_initializers;
    NodeId list = child(members[m].source, Kind::CtorInitializer);
    for (NodeId n = ast[list].first; n; n = ast[n].next) {
        NodeId id = child(n, Kind::MemInitializerId);
        EntityId field = resolve(ast[id].detail, entities[e].owner);
        if (!field) throw std::runtime_error("unknown constructor initializer");
        if (entities[field].kind == EntityKind::Alias) field = types[entities[field].type].entity;
        if (explicit_initializers.get(field)) throw std::runtime_error("duplicate constructor initializer");
        if (!nonstatic_field(field) && entities[field].kind != EntityKind::Type)
            throw std::runtime_error("initializer does not name a member or base");
        explicit_initializers.put(field, ast[id].next);
    }
    std::vector<SubobjectAction> work;
    auto add = [&](EntityId field, TypeId type, NodeId initial) {
        EntityId ctor = 0;
        default_destructor(type, scope);
        bool saved_base = base_initialization;
        base_initialization = !field;
        if (initial) initialize(initial, type, scope);
        else if (types[type].kind == TypeKind::Array || (types[type].kind == TypeKind::Named && entities[types[type].entity].class_info)) ctor = default_constructor(type, scope);
        else if (types[type].kind == TypeKind::LRef || types[type].kind == TypeKind::RRef || (types[type].cv & 1))
            throw std::runtime_error("uninitialized reference or const member");
        base_initialization = saved_base;
        if (!field) {
            EntityId selected = initial ? facts[initial].entity : ctor;
            if (selected && entities[selected].member_info) members[entities[selected].member_info].base_entry = true;
        } else if (ctor) members[entities[ctor].member_info].complete_entry = true;
        if (initial || ctor) work.push_back({field, type, initial, ctor});
    };
    for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) {
        EntityId base = bases[b].base;
        if (inherited) {
            default_destructor(entities[base].type, scope);
            members[entities[inherited].member_info].base_entry = true;
            demand_member(inherited);
            work.push_back({0, entities[base].type, 0, inherited});
        } else add(0, entities[base].type, explicit_initializers.get(base));
    }
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        EntityId field = declarations[d].entity;
        if (!nonstatic_field(field) || entities[field].owner != entities[cls].scope) continue;
        NodeId init = explicit_initializers.get(field);
        if (!init) init = entities[field].initializer;
        add(field, entities[field].type, init);
    }
    members[m].action_begin = subobject_actions.size(); members[m].action_count = work.size();
    // Necessary initialization is conservatively nontrivial here. A later
    // lifecycle fact can prove default subobject actions trivial independently.
    members[m].nontrivial = !work.empty();
    subobject_actions.insert(subobject_actions.end(), work.begin(), work.end());
}
} }

namespace cppgm { namespace semantic {
bool Analyzer::constructor_needed(EntityId e)
{
    if (!e) return false;
    auto m = entities[e].member_info;
    if (!members[m].synthetic || members[m].inherited_constructor) return true;
    if (members[m].trivial_state == 2) return members[m].nontrivial;
    if (members[m].trivial_state == 1) throw std::logic_error("cyclic constructor actions");
    members[m].trivial_state = 1;
    bool needed = false;
    for (unsigned j = 0; j < members[m].action_count; ++j) {
        auto action = subobject_actions[members[m].action_begin+j];
        if (action.initializer) {
            EntityId ctor = facts[action.initializer].entity;
            needed |= ctor && constructor_member(ctor) ? constructor_needed(ctor) : true;
        } else needed |= constructor_needed(action.constructor);
    }
    members[m].nontrivial = needed; members[m].trivial_state = 2;
    return needed;
}
void Analyzer::prepare_value_initialization(TypeId t, ScopeId s)
{
    Type type = types[t];
    if (type.kind == TypeKind::LRef || type.kind == TypeKind::RRef) throw std::runtime_error("value-initialized reference");
    if (type.kind == TypeKind::Array) { prepare_value_initialization(type.child, s); return; }
    if (type.kind != TypeKind::Named || !entities[type.entity].class_info) return;
    auto c = entities[type.entity].class_info;
    if (class_facts[c].value_state == 2) return;
    if (class_facts[c].value_state == 1) throw std::runtime_error("recursive value initialization");
    class_facts[c].value_state = 1;
    if (!class_facts[c].aggregate) {
        EntityId ctor = default_constructor(t, s);
        class_facts[c].value_constructor = ctor;
        if (!base_initialization) members[entities[ctor].member_info].complete_entry = true;
    }
    else for (auto d = scopes[entities[type.entity].scope].first_decl; d; d = declarations[d].next) {
        EntityId e = declarations[d].entity;
        if (nonstatic_field(e)) prepare_value_initialization(entities[e].type, s);
    }
    class_facts[c].value_state = 2;
}
} }
