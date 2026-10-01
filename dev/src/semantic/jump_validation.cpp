#include "semantic/analyzer.h"
#include <functional>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::check_jumps(NodeId body, bool binding_only)
{
    // Each initialization extends an immutable active-binding prefix. A jump
    // may leave prefixes but cannot enter a prefix absent at its origin.
    // DFS intervals answer that ancestor test in O(1) per control-flow edge.
    struct Frame { unsigned child = 0, next = 0, enter = 0, leave = 0; };
    struct Label { NodeId node; unsigned frame; std::uint32_t live; NodeId exception, region; };
    struct Jump { NodeId node; unsigned frame; std::uint32_t live; };
    std::vector<Frame> frames(1);
    std::vector<Label> labels(1);
    std::vector<Jump> jumps, cases;
    Index names;
    unsigned active = 0, switch_entry = 0;
    std::uint32_t live = 0, break_live = 0, continue_live = 0;
    NodeId context = 0;
    NodeId region = 0, break_region = 0, continue_region = 0;
    NodeId exception = 0, break_exception = 0, continue_exception = 0;
    auto enter_initialization = [&]() {
        unsigned parent = active;
        Frame frame; frame.next = frames[parent].child;
        frames[parent].child = frames.size(); active = frames.size(); frames.push_back(frame);
    };
    auto add_object = [&](EntityId e) {
        if (!e || (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter) || entities[e].is_static || entities[e].external_decl) return;
        if (binding_only) {
            if (entities[e].initializer) enter_initialization();
            return;
        }
        EntityId dtor = object_destructor(e);
        EntityId temporary = reference_temporary(e);
        bool destruction = reference_choices(e) || (!trivial_destructor(entities[e].type) && destructor_needed(dtor)) || parameter_cleanup(e) || temporary_cleanup(temporary) ||
            (types[entities[e].type].kind == TypeKind::Array && !trivial_destructor(entities[e].type));
        if (destruction) {
            LifetimeState state; state.object = temporary ? temporary : e; state.destructor = dtor; state.tail = live; state.depth = lifetimes[live].depth + 1;
            live = lifetimes.size(); lifetimes.push_back(state); object_lifetimes.put(e, live);
            if (temporary) object_lifetimes.put(temporary,live);
            for (auto choice = reference_choices(e); choice; choice = reference_alternatives[choice].next)
                object_lifetimes.put(reference_alternatives[choice].object,live);
        }
        if (!entities[e].initializer && !destruction && !constructor_needed(object_constructor(e))) return;
        enter_initialization();
    };
    std::function<void(NodeId)> visit = [&](NodeId n) {
        if (!n) return;
        Kind k = ast[n].kind;
        if (k == Kind::Lambda || k == Kind::Sizeof || k == Kind::SizeofPack || k == Kind::TypeTrait) return;
        if (k == Kind::StatementExpression) {
            auto saved = active, prior = region;
            region = n;
            enter_initialization(); // Even an empty body forbids incoming jumps.
            visit(ast[n].first); active = saved; region = prior; return;
        }
        bool recorded = k == Kind::Compound || k == Kind::Then || k == Kind::Else || k == Kind::ForInit || k == Kind::Iteration ||
            k == Kind::If || k == Kind::For || k == Kind::RangeFor || k == Kind::While || k == Kind::Do || k == Kind::Switch || k == Kind::Condition ||
            k == Kind::SimpleDeclaration || k == Kind::Class || k == Kind::ExpressionStatement || k == Kind::Assembly || k == Kind::Return || k == Kind::Goto ||
            k == Kind::Break || k == Kind::Continue || k == Kind::Label || k == Kind::Case || k == Kind::Default ||
            k == Kind::Throw || k == Kind::Try || k == Kind::FunctionTry || k == Kind::Handler;
        if (!recorded) {
            for (auto c = ast[n].first; c; c = ast[c].next) visit(c);
            return;
        }
        LifetimeUse use; use.entry = use.exit = live; use.context = context;
        use.expression_region = region;
        auto record_use = [&]() {
            // Empty lifetimes need no record. Source EH context is owned by
            // lowering; a lexical return context alone carries no cleanup.
            if (!(use.entry || use.exit || use.target || use.expression_region)) return;
            lifetime_index.put(n, lifetime_uses.size()); lifetime_uses.push_back(use);
        };
        if (k == Kind::Condition) { visit(ast[n].first); add_object(facts[n].entity); use.exit = live; record_use(); return; }
        if (k == Kind::SimpleDeclaration || k == Kind::Class) {
            add_object(anonymous_object(n));
            NodeId list = child(n, Kind::InitDeclarators);
            for (NodeId c = ast[list].first; c; c = ast[c].next) {
                auto e = facts[ast[c].first].entity;
                auto prior = live;
                add_object(e);
                auto complete = live; live = prior;
                if (e) visit(entities[e].initializer);
                live = complete;
            }
            use.exit = live; record_use(); return;
        }
        if (k == Kind::Label) {
            if (names.get(ast[n].text)) throw std::runtime_error("duplicate label");
            names.put(ast[n].text, labels.size()); labels.push_back({n, active, live, exception, region});
        }
        if (k == Kind::Goto) { jumps.push_back({n, active, live}); record_use(); return; }
        if (k == Kind::Return) {
            visit(ast[n].first);
            use.context = body;
            auto key_id = key(live, body); if (live) return_counts.put(key_id, return_counts.get(key_id) + 1); record_use(); return;
        }
        if (k == Kind::Break || k == Kind::Continue) {
            use.target = k == Kind::Break ? break_live : continue_live;
            use.target_region = k == Kind::Break ? break_region : continue_region;
            auto target = k == Kind::Break ? break_exception : continue_exception;
            if (target) jump_exception_targets.put(n,target);
            record_use(); return;
        }
        if (k == Kind::ExpressionStatement || k == Kind::Iteration || k == Kind::Throw || k == Kind::Assembly) {
            for (auto c = ast[n].first; c; c = ast[c].next) visit(c);
            record_use(); return;
        }
        if (k == Kind::Case || k == Kind::Default) cases.push_back({active, switch_entry,0});
        unsigned saved = active, saved_switch = switch_entry;
        auto saved_live = live, saved_break = break_live, saved_continue = continue_live;
        NodeId saved_context = context;
        auto saved_break_region = break_region, saved_continue_region = continue_region;
        auto saved_exception = exception, saved_break_exception = break_exception, saved_continue_exception = continue_exception;
        if (k == Kind::Try || k == Kind::FunctionTry) {
            // A jump may leave a protected body, but may not enter it. The
            // target's lexical owner is retained once for direct lowering.
            auto protected_body = child(n,Kind::Compound);
            enter_initialization(); exception = n; visit(protected_body);
            active = saved; live = saved_live; exception = saved_exception;
            for (auto h = ast[protected_body].next; h; h = ast[h].next) visit(h);
            record_use(); return;
        }
        if (k == Kind::Handler) { exception = n; enter_initialization(); add_object(facts[n].entity); }
        if (k == Kind::RangeFor) {
            break_region = continue_region = region;
            break_live = live; context = n;
            break_exception = continue_exception = exception;
            if (binding_only) {
                enter_initialization();
                visit(ast[n].last);
            } else {
                auto index = range_index.get(n);
                auto plan = ranges[index];
                if (plan.initialize_range) add_object(plan.range);
                add_object(plan.begin); add_object(plan.end);
                enter_initialization();
                break_live = continue_live = live; ranges[index].loop_live = live;
                use.exit = live;
                add_object(plan.variable); enter_initialization();
                ranges[index].body_live = live;
                visit(plan.body);
            }
            record_use(); active = saved; live = saved_live;
            break_live = saved_break; continue_live = saved_continue; context = saved_context;
            break_region = saved_break_region; continue_region = saved_continue_region;
            break_exception = saved_break_exception; continue_exception = saved_continue_exception;
            return;
        }
        bool loop = k == Kind::While || k == Kind::For || k == Kind::Do;
        if (loop || k == Kind::Switch) { break_live = live; context = n; break_exception = exception; break_region = region; }
        if (loop) { continue_live = live; continue_exception = exception; continue_region = region; }
        bool scope = k == Kind::Compound || k == Kind::Then || k == Kind::Else || k == Kind::If ||
            k == Kind::Switch || k == Kind::While || k == Kind::For || k == Kind::Do || k == Kind::Try || k == Kind::Handler;
        if (k == Kind::Switch) switch_entry = active;
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            if (!binding_only && k == Kind::If && (ast[n].flags & 1) &&
                ast[c].kind != Kind::Condition && ast[c].kind !=
                (constant_truth(constants[facts[n].value]) ? Kind::Then : Kind::Else)) continue;
            bool header = (loop || k == Kind::Switch) &&
                (ast[c].kind == Kind::Condition || ast[c].kind == Kind::ForInit || ast[c].kind == Kind::Iteration);
            auto loop_break = break_live, loop_continue = continue_live;
            auto loop_break_exception = break_exception, loop_continue_exception = continue_exception;
            auto loop_break_region = break_region, loop_continue_region = continue_region;
            if (header) {
                break_live = saved_break; continue_live = saved_continue;
                break_exception = saved_break_exception; continue_exception = saved_continue_exception;
                break_region = saved_break_region; continue_region = saved_continue_region;
            }
            visit(c);
            if (header) {
                break_live = loop_break; continue_live = loop_continue;
                break_exception = loop_break_exception; continue_exception = loop_continue_exception;
                break_region = loop_break_region; continue_region = loop_continue_region;
            }
            if (k == Kind::Switch && ast[c].kind == Kind::Condition) switch_entry = active;
            if (k == Kind::For && ast[c].kind == Kind::ForInit) continue_live = live;
        }
        use.exit = live; record_use();
        if (scope) { active = saved; live = saved_live; }
        break_live = saved_break; continue_live = saved_continue; context = saved_context;
        break_region = saved_break_region; continue_region = saved_continue_region;
        exception = saved_exception; break_exception = saved_break_exception; continue_exception = saved_continue_exception;
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
        if (!binding_only) {
            auto from = j.live, to = labels[label].live;
            while (from != to) {
                if (lifetimes[from].depth >= lifetimes[to].depth) from = lifetimes[from].tail;
                else {
                    initialization_guards.put(lifetimes[to].object,1);
                    to = lifetimes[to].tail;
                }
            }
        }
        facts.edit(j.node).target = labels[label].node;
        if (labels[label].exception) jump_exception_targets.put(j.node,labels[label].exception);
        if (auto use = lifetime_index.get(j.node)) {
            lifetime_uses[use].target = labels[label].live;
            lifetime_uses[use].target_region = labels[label].region;
        }
    }
    for (const Jump& j : cases)
        if (!ancestor(j.node, j.frame)) throw std::runtime_error("switch bypasses initialization");
}
} }
