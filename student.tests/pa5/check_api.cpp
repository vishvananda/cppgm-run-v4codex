#include "syntax/parser.h"
#include "preprocess/preprocessor.h"
#include <fstream>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

using namespace cppgm;
using namespace cppgm::syntax;

static void check_graph(const Ast& ast, NodeId root)
{
    std::vector<unsigned char> seen(ast.nodes.size());
    std::vector<NodeId> pending(1, root);
    while (!pending.empty()) {
        NodeId id = pending.back();
        pending.pop_back();
        assert(id < ast.nodes.size());
        if (seen[id]) continue; // Names can be shared as non-owning detail links.
        seen[id] = 1;
        const Node& node = ast[id];
        assert(node.location < ast.locations.size());
        const Location& loc = ast.locations[node.location];
        assert(loc.begin <= loc.end);
        if (node.detail) pending.push_back(node.detail);
        std::size_t siblings = 0;
        NodeId last = 0;
        for (NodeId child = node.first; child; child = ast[child].next) {
            assert(child < ast.nodes.size() && ++siblings < ast.nodes.size());
            pending.push_back(child);
            last = child;
        }
        assert(node.last == last);
    }
}

static void ownership()
{
    SourceBuffer source("int answer=42; const char* text=\"a\" \"b\"; double x=1.25;", 17);
    IdentifierTable ids;
    Ast ast(true);
    NodeId root;
    {
        PPTokenCursor pp(source, ids);
        PostTokenCursor post(pp, ids, true);
        Cursor cursor(post, ids, ast);
        Parser parser(cursor, ast, ids);
        root = parser.translation_unit();
        assert(cursor.max_pending < 32);
    } // Borrowed post-token decoding storage is now destroyed.
    check_graph(ast, root);
    bool number = false, string = false, floating = false;
    for (const Node& node : ast.nodes) {
        if (node.kind != Kind::Literal) continue;
        assert(ast.locations[node.location].file == 17);
        assert(ast.locations[node.location].line == 1);
        assert(node.literal < ast.literals.size());
        const LiteralValue& value = ast.literals[node.literal];
        if (value.kind == LiteralKind::integer) {
            int n;
            std::memcpy(&n, value.scalar.data(), sizeof n);
            assert(n == 42);
            number = true;
        } else if (value.kind == LiteralKind::string) {
            assert(value.elements == 3 && value.bytes == 3);
            assert(std::memcmp(ast.literal_bytes.data()+value.offset, "ab", 3) == 0);
            string = true;
        } else if (value.kind == LiteralKind::floating) {
            double n;
            std::memcpy(&n, value.scalar.data(), sizeof n);
            assert(n == 1.25);
            floating = true;
        }
    }
    assert(number && string && floating);
}

static void categories()
{
    Names names(true);
    ScopeId a = names.enter(0), b = names.enter(0), c = names.enter(0);
    names.import(a, b);
    names.import(b, a);
    names.bind(0, 1, Category::Type);
    assert(names.qualified(a, 1).category == Category::Unknown);
    assert(names.lookup(a, 1).category == Category::Type);
    names.bind(b, 1, Category::Value);
    assert(names.qualified(a, 1).category == Category::Value);
    names.bind(0, 2, Category::Namespace, c);
    names.bind(a, 2, Category::Value);
    assert(names.lookup(a, 2).category == Category::Value);
    assert(names.qualifier(a, 2).target == c);
    assert(names.qualifier(a, 2, true).category == Category::Unknown);
    assert(names.qualified(unknown_scope, 1).category == Category::Unknown);
    ScopeId anonymous = names.unnamed_namespace(a);
    assert(names.unnamed_namespace(a) == anonymous);
    assert(names.unnamed_namespace(b) != anonymous);
    assert(names.lookup_scopes && names.probes);
}

static void cross_file_string()
{
    const char* source = "/tmp/pa5-api-cross-file.cpp";
    const char* header = "/tmp/pa5-api-cross-file.hpp";
    {
        std::ofstream src(source), hdr(header);
        src << "const char* text=\"left\"\n#include \"" << header << "\"\n;\n";
        hdr << "\"right\"\n";
    }
    Preprocessor pp(source, "Sep  7 2026", "12:00:00");
    PostTokenCursor post(pp, pp.identifiers(), true);
    Ast ast;
    Cursor cursor(post, pp.identifiers(), ast);
    Parser parser(cursor, ast, pp.identifiers());
    check_graph(ast, parser.translation_unit());
    bool found = false;
    for (const Node& node : ast.nodes) {
        if (node.kind != Kind::Literal) continue;
        const LiteralValue& value = ast.literals[node.literal];
        assert(value.bytes == 10 && value.elements == 10);
        assert(std::memcmp(ast.literal_bytes.data()+value.offset, "leftright", 10) == 0);
        const Location& location = ast.locations[node.location];
        assert(location.file == 1 && location.begin == 17 && location.end == 23);
        found = true;
    }
    assert(found);
    std::remove(source);
    std::remove(header);
}

static std::size_t nested(unsigned depth)
{
    std::string text = "template<class T> struct box {}; ";
    for (unsigned i = 0; i < depth; ++i) text += "box<";
    text += "int";
    for (unsigned i = 0; i < depth; ++i) text += ">";
    text += " value;";
    SourceBuffer source(text);
    IdentifierTable ids;
    PPTokenCursor pp(source, ids);
    PostTokenCursor post(pp, ids, true);
    Ast ast(true);
    Cursor cursor(post, ids, ast);
    Parser parser(cursor, ast, ids);
    check_graph(ast, parser.translation_unit());
    assert(cursor.delimiter_work == cursor.produced);
    assert(parser.angle_work < cursor.produced * 2);
    assert(parser.angle_hits >= depth);
    assert(ast.nodes.size() < cursor.produced * 4);
    return parser.angle_work;
}

static void presumed_and_user_literal()
{
    const char* path = "/tmp/pa5-api-location.cpp";
    {
        std::ofstream source(path);
        source << "#line 70 \"logical.cpp\"\n#define V 19_tag\nint x=V;\n";
    }
    Preprocessor pp(path, "Sep  7 2026", "12:00:00");
    PostTokenCursor post(pp, pp.identifiers(), true);
    Ast ast(true);
    Cursor cursor(post, pp.identifiers(), ast);
    Parser parser(cursor, ast, pp.identifiers());
    check_graph(ast, parser.translation_unit());
    bool found = false;
    for (const Node& node : ast.nodes) {
        if (node.kind != Kind::Literal) continue;
        const LiteralValue& value = ast.literals[node.literal];
        assert(pp.identifiers().spelling(value.suffix).equals("_tag"));
        assert(pp.identifiers().spelling(value.prefix).equals("19"));
        const Location& loc = ast.locations[node.location];
        assert(loc.line == 71);
        assert(pp.identifiers().spelling(loc.presumed_file).equals("logical.cpp"));
        found = true;
    }
    std::remove(path);
    assert(found);
}

static void lexical_hint_work()
{
    const std::string name(1024, 'q');
    std::string text = "void f(){";
    for (unsigned i = 0; i < 1024; ++i) text += name + ";";
    text += "}";
    SourceBuffer source(text);
    IdentifierTable ids;
    PPTokenCursor pp(source, ids);
    PostTokenCursor post(pp, ids, true);
    Ast ast(true);
    Cursor cursor(post, ids, ast);
    Parser parser(cursor, ast, ids);
    check_graph(ast, parser.translation_unit());
    assert(parser.hint_bytes <= name.size() + 16);
}

int main(int argc, char** argv)
{
    ownership();
    categories();
    cross_file_string();
    presumed_and_user_literal();
    lexical_hint_work();
    std::size_t small = nested(64), large = nested(256);
    assert(large < small * 5);
    for (int i = 1; i < argc; ++i) {
        Preprocessor pp(argv[i], "Sep  7 2026", "12:00:00");
        PostTokenCursor post(pp, pp.identifiers(), true);
        Ast ast;
        Cursor cursor(post, pp.identifiers(), ast);
        Parser parser(cursor, ast, pp.identifiers());
        check_graph(ast, parser.translation_unit());
    }
    std::cout << "PA5 API: decoded literal lifetime, graph identities/locations and linear nested-angle work pass\n";
}
