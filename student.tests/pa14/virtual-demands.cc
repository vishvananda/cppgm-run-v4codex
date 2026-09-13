// Independent public observations of vtable, body and completion fact states.
#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    using namespace cppgm;
    using namespace semantic;
    assert(argc == 3);
    Preprocessor pp(argv[1], "Sep 13 2026", "00:00:00");
    PostTokenCursor post(pp, pp.identifiers());
    syntax::Ast ast(true);
    syntax::Cursor cursor(post, pp.identifiers(), ast);
    syntax::Parser parser(cursor, ast, pp.identifiers());
    Analyzer sem(ast, pp.identifiers(), true, true);
    parser.translation_unit(&sem);
    std::string mode = argv[2];
    bool failed = mode != "success";
    unsigned failures = 0;
    std::size_t nodes = 0, entities = 0;
    for (unsigned i = 0; i < 10000; ++i) {
        try { sem.finish(); } catch (const std::exception&) { ++failures; }
        if (!i) { nodes = ast.nodes.size(); entities = sem.entities.size(); }
        assert(nodes == ast.nodes.size() && entities == sem.entities.size());
    }
    assert(failures == (failed ? 10000u : 0u));
    unsigned broken = 0, complete = 0, broken_bodies = 0;
    for (EntityId e = 1; e < sem.entities.size(); ++e)
        if (sem.entities[e].member_info && sem.member_fact(e).demand == DemandState::Failed) ++broken_bodies;
    for (EntityId e = 1; e < sem.entities.size(); ++e) if (sem.polymorphic(e)) {
        const auto& v = sem.virtual_class(e);
        if (v.demand == FactState::Failure) ++broken;
        if (v.demand == FactState::Success) ++complete;
        assert(v.demand != FactState::Active);
        if (v.key_function && sem.member_fact(v.key_function).body)
            assert(v.reasons & static_cast<unsigned char>(VtableReason::KeyDefinition));
    }
    assert(broken == (mode == "failure" ? 1u : 0u));
    if (mode == "body-failure") assert(broken_bodies == 1 && complete == 1);
    assert(complete == sem.demanded_vtables().size());
    const auto& emitted = sem.demanded_vtables();
    for (std::size_t j = 1; j < emitted.size(); ++j) assert(emitted[j-1] < emitted[j]);
    std::cout << "failures " << failures << " failed_vtables " << broken << '\n';
    std::cout << '{'; sem.telemetry(std::cout); std::cout << "}\n";
}
