#include "preprocess/expression.h"
#include "preprocess/expression_value.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <new>
#include <type_traits>

static std::size_t allocations;
void* operator new(std::size_t n) {
    ++allocations;
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    return p;
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }

using namespace cppgm;
struct Macros { IdentifierId active; bool enabled; };
static bool lookup(IdentifierId id, void* context)
{
    Macros& macros = *static_cast<Macros*>(context);
    return id == macros.active && macros.enabled;
}

static PPExpressionResult line(PPTokenCursor& cursor, PPExpressionEvaluator& evaluator)
{
    for (;;) {
        PPToken t = cursor.next();
        if (t.kind == PPTokenKind::newline || t.kind == PPTokenKind::eof) return evaluator.finish();
        evaluator.push(t);
    }
}

static void macro_identity_and_lifetimes()
{
    IdentifierTable names;
    Macros macros = { names.intern(TextView("alpha", 5)), true };
    SourceBuffer source("defined(al\\\npha) ? -7 : u'a'\ndefined alpha\ndefined(alpha)\n");
    PPTokenCursor cursor(source, names);
    PPExpressionEvaluator evaluator(names, lookup, &macros);
    PPExpressionResult first = line(cursor, evaluator);
    assert(first.valid && first.value.is_unsigned && first.value.bits == std::uint64_t(0)-7);
    for (int i = 0; i < 10000; ++i) {
        std::string name = "new_" + std::to_string(i);
        names.intern(TextView(name.data(), name.size()));
    }
    macros.enabled = false;
    assert(line(cursor, evaluator).value.bits == 0);
    macros.enabled = true;
    assert(line(cursor, evaluator).value.bits == 1); // no stale macro lookup cache
    assert(first.value.bits == std::uint64_t(0)-7); // no borrowed value storage
    assert(evaluator.finish().empty);
}

static void literal_promotions()
{
    for (unsigned type = FT_SIGNED_CHAR; type <= FT_BOOL; ++type) {
        PostToken token;
        token.kind = PostTokenKind::literal;
        token.type = static_cast<EFundamentalType>(type);
        token.scalar.fill(static_cast<char>(0xff));
        if (type == FT_BOOL) token.scalar[0] = 1;
        PPValue value;
        assert(promote_pp_literal(token, value));
        bool uns = (type >= FT_UNSIGNED_CHAR && type <= FT_UNSIGNED_LONG_LONG_INT) ||
                   type == FT_CHAR16_T || type == FT_CHAR32_T;
        assert(value.is_unsigned == uns);
        if (type == FT_BOOL) assert(value.bits == 1);
        else if (!uns || fundamental_width(token.type) == 8) assert(value.bits == ~std::uint64_t(0));
        else assert(value.bits == (std::uint64_t(1) << (8 * fundamental_width(token.type))) - 1);
    }
}

static void steady_storage()
{
    std::string pattern = "defined alpha ? ((0 && (7/0)) + u'a') : 0u\n";
    std::string text;
    for (int i = 0; i < 5000; ++i) text += pattern;
    SourceBuffer source(std::move(text));
    IdentifierTable names;
    Macros macros = { names.intern(TextView("alpha", 5)), true };
    PPTokenCursor cursor(source, names);
    PPExpressionStats stats;
    PPExpressionEvaluator evaluator(names, lookup, &macros, &stats);
    assert(line(cursor, evaluator).value.bits == 97);
    std::size_t before = allocations, storage = evaluator.storage_bytes();
    for (int i = 1; i < 5000; ++i) {
        PPExpressionResult result = line(cursor, evaluator);
        assert(result.valid && result.value.is_unsigned && result.value.bits == 97);
    }
    assert(allocations == before && storage == evaluator.storage_bytes());
    assert(stats.lines == 5000 && stats.errors == 0 && stats.reductions == 20000);
}

int main()
{
    static_assert(std::is_trivially_copyable<PPValue>::value, "no per-value ownership");
    macro_identity_and_lifetimes();
    literal_promotions();
    steady_storage();
    std::cout << "PA3 API: stable names, live macro query, promotions, scratch reuse and zero warmed allocations passed\n";
}
