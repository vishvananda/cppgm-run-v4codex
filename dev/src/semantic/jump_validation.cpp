#include "semantic/analyzer.h"
#include <functional>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::check_jumps(NodeId body)
{
    // Each initialization extends an immutable active-binding prefix. A jump
    // may leave prefixes but cannot enter a prefix absent at its origin.
    // DFS intervals answer that ancestor test in O(1) per control-flow edge.
    struct Frame { unsigned child = 0, next = 0, enter = 0, leave = 0; };
    struct Label { NodeId node; unsigned frame; std::uint32_t live; };
    struct Jump { NodeId node; unsigned frame; };
    std::vector<Frame> frames(1);
    std::vector<Label> labels(1);
    std::vector<Jump> jumps, cases;
    Index names;
    unsigned active = 0, switch_entry = 0;
    std::uint32_t live = 0, break_live = 0, continue_live = 0;
    NodeId context = 0;
    auto add_object = [&](EntityId e) {
        if (!e || (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter) || entities[e].is_static || entities[e].external_decl) return;
        EntityId dtor = object_destructor(e);
        EntityId temporary = reference_temporary(e);
        bool destruction = reference_choices(e) || destructor_needed(dtor) || parameter_cleanup(e) || temporary_cleanup(temporary) ||
            (types[entities[e].type].kind == TypeKind::Array && !trivial_destructor(entities[e].type));
        if (destruction) {
            LifetimeState state; state.object = temporary ? temporary : e; state.destructor = dtor; state.tail = live; state.depth = lifetimes[live].depth + 1;
            live = lifetimes.size(); lifetimes.push_back(state); object_lifetimes.put(e, live);
            if (temporary) object_lifetimes.put(temporary,live);
            for (auto choice = reference_choices(e); choice; choice = reference_alternatives[choice].next)
                object_lifetimes.put(reference_alternatives[choice].object,live);
        }
        if (!entities[e].initializer && !destruction && !constructor_needed(object_constructor(e))) return;
        unsigned parent = active;
        Frame frame; frame.next = frames[parent].child;
        frames[parent].child = frames.size(); active = frames.size(); frames.push_back(frame);
    };
    std::function<void(NodeId)> visit = [&](NodeId n) {
        if (!n) return;
        Kind k = ast[n].kind;
        bool recorded = k == Kind::Compound || k == Kind::Then || k == Kind::Else || k == Kind::ForInit || k == Kind::Iteration ||
            k == Kind::If || k == Kind::For || k == Kind::While || k == Kind::Do || k == Kind::Switch || k == Kind::Condition ||
            k == Kind::SimpleDeclaration || k == Kind::Class || k == Kind::ExpressionStatement || k == Kind::Return || k == Kind::Goto ||
            k == Kind::Break || k == Kind::Continue || k == Kind::Label || k == Kind::Case || k == Kind::Default;
        if (!recorded) return;
        LifetimeUse use; use.entry = use.exit = live; use.context = context;
        auto record_use = [&]() {
            if (!(use.entry || use.exit || use.target)) return;
            lifetime_index.put(n, lifetime_uses.size()); lifetime_uses.push_back(use);
        };
        if (k == Kind::Condition) { add_object(facts[n].entity); use.exit = live; record_use(); return; }
        if (k == Kind::SimpleDeclaration || k == Kind::Class) {
            add_object(anonymous_object(n));
            NodeId list = child(n, Kind::InitDeclarators);
            for (NodeId c = ast[list].first; c; c = ast[c].next) add_object(facts[ast[c].first].entity);
            use.exit = live; record_use(); return;
        }
        if (k == Kind::Label) {
            if (names.get(ast[n].text)) throw std::runtime_error("duplicate label");
            names.put(ast[n].text, labels.size()); labels.push_back({n, active, live});
        }
        if (k == Kind::Goto) { jumps.push_back({n, active}); record_use(); return; }
        if (k == Kind::Return) {
            use.context = body;
            auto key_id = key(live, body); if (live) return_counts.put(key_id, return_counts.get(key_id) + 1); record_use(); return;
        }
        if (k == Kind::Break || k == Kind::Continue) { use.target = k == Kind::Break ? break_live : continue_live; record_use(); return; }
        if (k == Kind::ExpressionStatement || k == Kind::Iteration) { record_use(); return; }
        if (k == Kind::Case || k == Kind::Default) cases.push_back({active, switch_entry});
        unsigned saved = active, saved_switch = switch_entry;
        auto saved_live = live, saved_break = break_live, saved_continue = continue_live;
        NodeId saved_context = context;
        bool loop = k == Kind::While || k == Kind::For || k == Kind::Do;
        if (loop || k == Kind::Switch) { break_live = live; context = n; }
        if (loop) continue_live = live;
        bool scope = k == Kind::Compound || k == Kind::Then || k == Kind::Else || k == Kind::If ||
            k == Kind::Switch || k == Kind::While || k == Kind::For || k == Kind::Do;
        if (k == Kind::Switch) switch_entry = active;
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            visit(c);
            if (k == Kind::Switch && ast[c].kind == Kind::Condition) switch_entry = active;
            if (k == Kind::For && ast[c].kind == Kind::ForInit) continue_live = live;
        }
        use.exit = live; record_use();
        if (scope) { active = saved; live = saved_live; }
        break_live = saved_break; continue_live = saved_continue; context = saved_context;
        switch_entry = saved_switch;
    };
    ScopeId owner = scopes[facts[body].scope].parent;
    if (scopes[owner].kind == ScopeKind::Function)
        for (auto d = scopes[owner].first_decl; d; d = declarations[d].next) {
            EntityId e = declarations[d].entity;
            if (entities[e].kind == EntityKind::Parameter) add_object(e);
        }
    visit(body);
    unsigned clock = 0;
    std::function<void(unsigned)> number = [&](unsigned i) {
        frames[i].enter = clock++;
        for (unsigned c = frames[i].child; c; c = frames[c].next) number(c);
        frames[i].leave = clock;
    };
    number(0);
    auto ancestor = [&](unsigned target, unsigned source) {
        return frames[target].enter <= frames[source].enter && frames[source].enter < frames[target].leave;
    };
    for (const Jump& j : jumps) {
        auto label = names.get(ast[j.node].text);
        if (!label) throw std::runtime_error("undefined goto label");
        if (!ancestor(labels[label].frame, j.frame)) throw std::runtime_error("goto bypasses initialization");
        facts[j.node].target = labels[label].node;
        if (auto use = lifetime_index.get(j.node)) lifetime_uses[use].target = labels[label].live;
    }
    for (const Jump& j : cases)
        if (!ancestor(j.node, j.frame)) throw std::runtime_error("switch bypasses initialization");
}
} }
