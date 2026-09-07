// Independent whole-owner tests: direct graph producers bypass the adapter.
template<class Action> static void rejects(Action action) {
    bool rejected = false;
    try { action(); } catch (const std::runtime_error&) { rejected = true; }
    assert(rejected);
}
static void final_api_checks() {
    AbiFactFile file; Graph& g = file.graph;
    Id i = g.builtin(ABI_BUILTIN_TYPE_INT), u = g.builtin(ABI_BUILTIN_TYPE_UNSIGNED_INT);
    Id b = g.builtin(ABI_BUILTIN_TYPE_BOOL);
    Id minus = g.make(Kind::Value, u, 0, 0, ~0ull);
    assert(minus == g.make(Kind::Value, u, 0, 0, 4294967295ull));
    assert(g.make(Kind::Value, b, 0, 0, 2) == g.make(Kind::Value, b, 0, 0, 1));
    assert(g.make(Kind::Cv, g.make(Kind::Cv, i, 1), 2) == g.cv(i, 3));
    Id fn = g.make(Kind::FunctionType, i);
    assert(g.cv(fn, 1) == g.make(Kind::FunctionType, i, 1));
    Target t; t.type = g.make(Kind::FunctionType, i, 5);
    assert(mangle(g, t) == "KFivRE"); file.cases.push_back(t);
    Target f; f.kind = TargetKind::Function; f.function.name = g.path("f");
    f.function.parameters = {g.cv(fn, 1), fn, g.cv(fn, 1), fn};
    assert(mangle(g, f) == "_Z1fKFivEFivES_S0_"); file.cases.push_back(f);
    Function host; host.name = g.path("host");
    Id context = function_entity(g, host);
    f.function.parameters.clear(); f.function.context = context;
    assert(mangle(g, f) == "_ZZ4hostvE1fv"); file.cases.push_back(f);
    Target thunk; thunk.kind = TargetKind::VirtualThunk;
    thunk.function.name = g.path("C::f"); thunk.this_adjust = -8; thunk.vcall_offset = -32;
    assert(mangle(g, thunk) == "_ZTvn8_n32_N1C1fEv"); file.cases.push_back(thunk);
    Id prefix = g.make(Kind::Parameter, 0, 1);
    t.type = g.make(Kind::Template, prefix, 0, 0, 0, {g.make(Kind::TypeArgument, i)});
    file.cases.push_back(t);
    Function tagged; tagged.name = g.path("ns::tagged");
    tagged.tags = {g.string("z"), g.string("a"), g.string("z")};
    Id entity = function_entity(g, tagged);
    tagged.tags = {g.string("a"), g.string("z")};
    assert(entity == function_entity(g, tagged));
    tagged.name = g.make(Kind::Tagged, tagged.name, 0, 0, 0, tagged.tags);
    tagged.tags.clear();
    assert(entity == function_entity(g, tagged));
    f.function = tagged; file.cases.push_back(f);
    assert(mangle(g, f) == "_ZN2ns6taggedB1aB1zEv");
    Function specialized; specialized.name = g.path("specialized");
    specialized.arguments = {g.make(Kind::TypeArgument, i)};
    specialized.template_prefix = true;
    Id specialization = function_entity(g, specialized);
    specialized.name = g.make(Kind::Template, specialized.name, 0, 0, 0, specialized.arguments);
    specialized.arguments.clear(); specialized.template_prefix = false;
    assert(specialization == function_entity(g, specialized));
    auto copy = parse_fact_text(serialize_fact_file(file));
    assert(mangle_fact_file(file) == mangle_fact_file(copy));
    // Reject invalid edges on publication, before they can form cycles or
    // reach a different ID arena during encoding/serialization.
    rejects([&]() { g.make(Kind::Pointer, g.size()); });
    rejects([&]() { g.make(Kind::Decltype, i); });
    rejects([&]() { g.make(Kind::Template, i, 0, 0, 0, {i}); });
    Id completed = g.make(Kind::Template, g.path("Box"), 0, 0, 0, {g.make(Kind::TypeArgument, i)});
    rejects([&]() { g.make(Kind::Template, completed); });
    rejects([&]() { g.make(Kind::FunctionEntity, host.name, 0, 0, 0, {0}); });
    rejects([&]() { g.make(Kind::Cv, i, 4); });
    rejects([&]() { g.make(Kind::FunctionType, i, 12); });
    rejects([&]() { g.make(Kind::Name, 0, ~0u); });
    rejects([&]() { g.spelling(~0u); });
    // Every recursive route shares the limit, including new external-name
    // substitution states and enclosing-function contexts.
    Function local; local.name = g.path("local");
    Id local_context = context;
    for (unsigned n = 0; n < 1500; ++n) {
        local.context = local_context;
        local_context = function_entity(g, local);
    }
    t.type = g.make(Kind::Local, local_context, g.string("L"));
    rejects([&]() { mangle(g, t); });
    Id external = function_entity(g, host);
    Function use; use.name = g.path("use");
    for (unsigned n = 0; n < 1500; ++n) {
        use.arguments = {g.make(Kind::EntityArgument, external)};
        external = function_entity(g, use);
    }
    f.function = use;
    rejects([&]() { mangle(g, f); });
    std::cout << "final audit canonical values, function facts, graph validation and nesting pass\n";
}
