// Public semantic API: repeated failure must not resume partial publication.
#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <cassert>
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    using namespace cppgm;
    assert(argc == 3);
    Preprocessor pp(argv[1], "Sep 13 2026", "00:00:00");
    PostTokenCursor post(pp, pp.identifiers());
    syntax::Ast ast(true);
    syntax::Cursor cursor(post, pp.identifiers(), ast);
    syntax::Parser parser(cursor, ast, pp.identifiers());
    // Type-only construction leaves physical layout to the explicit API demand.
    bool deferred_layout = std::string(argv[2]) == "layout";
    semantic::Analyzer sem(ast, pp.identifiers(), !deferred_layout, !deferred_layout);
    parser.translation_unit(&sem);
    bool finish = std::string(argv[2]) == "finish";
    semantic::TypeId target = 0, good = 0;
    for (const auto& entity : sem.entities) {
        if (!entity.name) continue;
        auto spelling = pp.identifiers().spelling(entity.name);
        std::string name(spelling.data,spelling.size);
        if (name == "Target") target = entity.type;
        if (name == "Good") good = entity.type;
    }
    assert(finish || target);
    unsigned failures = 0;
    std::size_t nodes = 0, entities = 0;
    for (unsigned i = 0; i < 10000; ++i) {
        try {
            if (finish) sem.finish(); else sem.object_size(target);
        } catch (const std::exception& error) {
            ++failures;
            if (i < 2) std::cout << i << ' ' << error.what() << '\n';
#ifdef EXPECT_TERMINAL_FACTS
            if (i) {
                auto cached = dynamic_cast<const semantic::FailedSemanticFact*>(&error);
                assert(cached && cached->fact != semantic::SemanticFact::None);
            }
#endif
        }
        if (!i) { nodes = ast.nodes.size(); entities = sem.entities.size(); }
        else { assert(nodes == ast.nodes.size()); assert(entities == sem.entities.size()); }
    }
    std::cout << "failures " << failures << " nodes " << nodes << " entities " << entities << '\n';
    if (good) assert(sem.object_size(good) == 4);
    sem.telemetry(std::cout); std::cout << '\n';
#ifdef EXPECT_TERMINAL_FACTS
    assert(failures == 10000);
#endif
}
