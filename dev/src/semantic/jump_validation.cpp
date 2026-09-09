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
    struct Label { NodeId node; unsigned frame; };
    struct Jump { NodeId node; unsigned frame; };
    std::vector<Frame> frames(1);
    std::vector<Label> labels(1);
    std::vector<Jump> jumps, cases;
    Index names;
    unsigned active = 0, switch_entry = 0;
    auto add_object = [&](EntityId e) {
        if (!e || entities[e].kind != EntityKind::Variable || !entities[e].initializer) return;
        unsigned parent = active;
        Frame frame; frame.next = frames[parent].child;
        frames[parent].child = frames.size(); active = frames.size(); frames.push_back(frame);
    };
    std::function<void(NodeId)> visit = [&](NodeId n) {
        if (!n) return;
        Kind k = ast[n].kind;
        if (k == Kind::Condition) { add_object(facts[n].entity); return; }
        if (k == Kind::SimpleDeclaration) {
            NodeId list = child(n, Kind::InitDeclarators);
            for (NodeId c = ast[list].first; c; c = ast[c].next) add_object(facts[ast[c].first].entity);
            return;
        }
        if (k == Kind::Label) {
            if (names.get(ast[n].text)) throw std::runtime_error("duplicate label");
            names.put(ast[n].text, labels.size()); labels.push_back({n, active});
        }
        if (k == Kind::Goto) { jumps.push_back({n, active}); return; }
        if (k == Kind::Case || k == Kind::Default) cases.push_back({active, switch_entry});
        unsigned saved = active, saved_switch = switch_entry;
        bool scope = k == Kind::Compound || k == Kind::Then || k == Kind::Else || k == Kind::If ||
            k == Kind::Switch || k == Kind::While || k == Kind::For || k == Kind::Do;
        if (k == Kind::Switch) switch_entry = active;
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            visit(c);
            if (k == Kind::Switch && ast[c].kind == Kind::Condition) switch_entry = active;
        }
        if (scope) active = saved;
        switch_entry = saved_switch;
    };
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
    }
    for (const Jump& j : cases)
        if (!ancestor(j.node, j.frame)) throw std::runtime_error("switch bypasses initialization");
}
} }
