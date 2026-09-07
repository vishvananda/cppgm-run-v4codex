#include "abi/itanium/abi_mangle.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace abi_mangle;
static std::string read(const char* path) {
    std::ifstream in(path); if (!in) throw std::runtime_error("input open");
    std::ostringstream text; text << in.rdbuf(); return text.str();
}
static void api_checks() {
    Graph g;
    Id c = g.path("ns::C"), int_type = g.builtin(ABI_BUILTIN_TYPE_INT);
    assert(c == g.path("::ns::C"));
    assert(g.cv(g.cv(c, 1), 2) == g.cv(g.cv(c, 2), 1));
    Id arg = g.make(Kind::TypeArgument, c);
    Id box = g.make(Kind::Template, g.path("Box"), 0, 0, 0, {arg});
    Target f; f.kind = TargetKind::Function; f.function.name = g.path("use");
    f.function.parameters = {box, box};
    assert(mangle(g, f) == "_Z3use3BoxIN2ns1CEES2_");
    Target op; op.kind = TargetKind::Function; op.function.name = g.path("ns::operator");
    op.function.terminal = ABI_TERMINAL_PLUS; op.function.parameters = {c};
    op.function.category = FunctionCategory::Nonmember;
    assert(mangle(g, op) == "_ZN2nspsENS_1CE");
    Function host; host.name = g.path("host"); host.parameters = {int_type};
    Id ctx = function_entity(g, host);
    Id local = g.make(Kind::Local, ctx, g.string("Local"));
    Target t; t.type = local;
    assert(mangle(g, t) == "Z4hostiE5Local");
    assert(mangle(g, f) == "_Z3use3BoxIN2ns1CEES2_");
    // Unrelated graph growth must not affect identity or work for an existing
    // symbol; this exercises the production path without a fact adapter.
    auto before = g.stats.emitted_nodes;
    mangle(g, f); auto visits = g.stats.emitted_nodes - before;
    for (unsigned i = 0; i < 100000; ++i) g.name(0, "unrelated" + std::to_string(i));
    before = g.stats.emitted_nodes;
    assert(mangle(g, f) == "_Z3use3BoxIN2ns1CEES2_");
    assert(g.stats.emitted_nodes - before == visits);
    // A long chain uses flat graph storage and iterative modifier emission.
    Id pointer = int_type;
    for (unsigned i = 0; i < 20000; ++i) pointer = g.make(Kind::Pointer, pointer);
    t.type = pointer; assert(mangle(g, t) == std::string(20000, 'P') + 'i');
    Target thunk; thunk.kind = TargetKind::Thunk; thunk.function.name = g.path("C::self");
    thunk.has_result_adjust = true; thunk.virtual_result = true;
    thunk.result_vcall_offset = -32;
    assert(mangle(g, thunk) == "_ZTch0_v0_n32_N1C4selfEv");
    AbiFactFile file; file.graph = std::move(g); file.cases = {f, thunk, op};
    auto reparsed = parse_fact_text(serialize_fact_file(file));
    assert(mangle_fact_file(reparsed) == mangle_fact_file(file));
    std::cout << "direct graph identity, local context, sparse work, deep modifiers and thunk checks pass\n";
}
int main(int argc, char** argv) {
    try {
        if (argc == 1) { api_checks(); return 0; }
        AbiFactFile file = parse_fact_text(read(argv[1]));
        std::string serialized = serialize_fact_file(file);
        if (argc > 2) { std::cout << serialized; return 0; }
        AbiFactFile copy = parse_fact_text(serialized);
        std::string names = mangle_fact_file(file);
        if (mangle_fact_file(copy) != names) {
            std::cerr << "roundtrip changed names\n" << serialized << "expected:\n" << names
                      << "actual:\n" << mangle_fact_file(copy); return 2;
        }
        std::cout << names;
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
