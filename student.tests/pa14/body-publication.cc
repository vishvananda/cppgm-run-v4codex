// Public API: a failed source body cannot become an emittable definition.
#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc, char** argv)
{
    using namespace cppgm;
    assert(argc == 3);
    bool valid = std::string(argv[2]) == "valid";
    Preprocessor pp(argv[1], "Sep 13 2026", "00:00:00");
    PostTokenCursor post(pp, pp.identifiers());
    syntax::Ast ast(true);
    syntax::Cursor cursor(post, pp.identifiers(), ast);
    syntax::Parser parser(cursor, ast, pp.identifiers());
    semantic::Analyzer sem(ast, pp.identifiers(), true, true);
    bool rejected = false;
    try { parser.translation_unit(&sem); sem.finish(); }
    catch (const std::exception&) { rejected = true; }
    assert(rejected != valid);
    semantic::EntityId target = 0;
    for (semantic::EntityId e = 1; e < sem.entities.size(); ++e) {
        const auto& item = sem.entities[e];
        if (!item.name) continue;
        auto name = pp.identifiers().spelling(item.name);
        if (item.kind == semantic::EntityKind::Function && !item.template_info &&
            !item.template_pattern && std::string(name.data,name.size) == "target") target = e;
    }
    assert(target);
    auto nodes = ast.nodes.size(), entities = sem.entities.size(), lifetimes = sem.lifetimes.size();
    unsigned failed_finish = 0, failed_body = 0;
    for (unsigned i = 0; i < 10000; ++i) {
        try { sem.finish(); }
        catch (const semantic::FailedSemanticFact&) { ++failed_finish; }
#ifndef ENTRY_PUBLICATION
        try { sem.require_body_facts(target); }
        catch (const semantic::FailedSemanticFact& error) {
            assert(error.fact == semantic::SemanticFact::FunctionDefinition && error.entity == target);
            ++failed_body;
        }
#endif
        assert(ast.nodes.size() == nodes && sem.entities.size() == entities && sem.lifetimes.size() == lifetimes);
    }
#ifndef ENTRY_PUBLICATION
    assert(failed_finish == (valid ? 0u : 10000u));
    assert(failed_body == (valid ? 0u : 10000u));
#endif
    std::cout << "finish failures " << failed_finish << " body failures " << failed_body
        << " nodes " << nodes << " entities " << entities << " lifetimes " << lifetimes
        << " Entity bytes " << sizeof(semantic::Entity) << '\n';
}
