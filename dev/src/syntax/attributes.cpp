#include "syntax/parser.h"
#include "support/attributes.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace syntax {
void Parser::native_attributes(NodeId owner, NativeAttributes value)
{
    // Type attributes own ordinary parsed expression children, so template
    // occurrence projection retains their operands without reparsing metadata.
    auto type_owner = ast[owner].kind == Kind::SimpleDeclaration || ast[owner].kind == Kind::Function ? ast[owner].first : owner;
    for (auto a = value.vector_attributes; a;) {
        auto next = ast[a].next; ast[a].next = 0; ast.append(type_owner,a); a = next;
    }
    value.vector_attributes = 0;
    if (!value.section && !value.weak && !value.tags && !value.no_unique_address && value.effects == FunctionEffects::Unknown) return;
    owner = ast.nodes.occurrences[owner].source;
    auto prior = ast.native_attribute_owners.get(owner);
    if (prior) {
        auto& old = ast.native_attributes[prior];
        if (old.section && value.section && old.section != value.section) throw std::runtime_error("conflicting section attributes");
        if (value.section) old.section = value.section;
        old.weak |= value.weak;
        old.no_unique_address |= value.no_unique_address;
        old.effects = std::max(old.effects,value.effects);
        // Attributes are published only after this declaration's parse. Link
        // its fresh list to the prior immutable prefix without copying nodes.
        if (value.tags) {
            auto tail = value.tags;
            while (ast.abi_tags[tail].next) tail = ast.abi_tags[tail].next;
            ast.abi_tags[tail].next = old.tags; old.tags = value.tags;
        }
    } else {
        ast.native_attribute_owners.put(owner,ast.native_attributes.size());
        ast.native_attributes.push_back(value);
    }
}
unsigned Parser::balanced(const char* open, const char* close, NativeAttributes* native, std::uint32_t* alignment)
{
    unsigned result = 0;
    in.require(open);
    while (!in.is(close)) {
        if (in.peek().kind == PostTokenKind::eof) throw std::runtime_error("unterminated attribute");
        // Only attribute names have meaning. Identifiers in an unknown
        // attribute's arguments must never become declaration properties.
        bool unscoped = true;
        if (identifier() && in.is("::",1)) {
            bool known = in.is("gnu") || in.is("clang");
            in.take(); in.take(); unscoped = false;
            if (!known) {
                in.take();
                if (in.is("(")) { auto end = in.matching(0); for (std::size_t i=0;i<=end;++i) in.take(); }
                continue;
            }
        }
        if (in.is("abi_tag") || in.is("__abi_tag__")) {
            in.take(); in.require("(");
            do {
                if (in.peek().kind != PostTokenKind::literal) throw std::runtime_error("abi_tag requires string arguments");
                auto token = in.take(); const auto& value = ast.literals[token.literal];
                if (value.kind != LiteralKind::string || value.type != FT_CHAR || value.bytes < 2)
                    throw std::runtime_error("abi_tag requires a nonempty narrow string");
                auto data = ast.literal_bytes.data()+value.offset;
                for (unsigned i = 0; i+1 < value.bytes; ++i) {
                    auto c = data[i];
                    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        c == '_' || (i && c >= '0' && c <= '9')))
                        throw std::runtime_error("abi_tag requires an identifier");
                }
                if (native) {
                    ast.abi_tags.push_back({ids.intern(TextView(data,value.bytes-1)),native->tags});
                    native->tags = ast.abi_tags.size()-1;
                }
            } while (in.eat(","));
            in.require(")");
        }
        else if (in.is("section") || in.is("__section__")) {
            in.take(); in.require("(");
            if (in.peek().kind != PostTokenKind::literal) throw std::runtime_error("section requires a string literal");
            auto token = in.take(); const auto& value = ast.literals[token.literal];
            if (value.kind != LiteralKind::string || value.type != FT_CHAR || value.bytes < 2)
                throw std::runtime_error("section requires a nonempty narrow string");
            auto data = ast.literal_bytes.data()+value.offset;
            for (unsigned i = 0; i+1 < value.bytes; ++i) {
                auto c = data[i];
                if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '.')) throw std::runtime_error("unsafe section name");
            }
            in.require(")");
            if (!native) throw std::runtime_error("section attribute requires an object declaration");
            auto name = ids.intern(TextView(data,value.bytes-1));
            if (native->section && native->section != name) throw std::runtime_error("conflicting section attributes");
            native->section = name;
        }
        else if (in.is("vector_size") || in.is("__vector_size__")) {
            in.take(); in.require("(");
            auto operand = expression(2); in.require(")");
            if (!native) throw std::runtime_error("vector_size requires a type owner");
            auto attribute = wrap(Kind::VectorAttribute,operand);
            ast[attribute].next = native->vector_attributes;
            native->vector_attributes = attribute;
        }
        else if (in.is("aligned") || in.is("__aligned__")) {
            in.take();
            NodeId operand = 0;
            if (in.eat("(")) { operand = expression(2); in.require(")"); }
            if (alignment) {
                ast.alignments.push_back({operand,*alignment,false,true});
                *alignment = ast.alignments.size()-1;
            }
        }
        else if (in.is("(")) result |= balanced("(", ")",native,alignment);
        else if (in.is("[")) result |= balanced("[", "]",native,alignment);
        else if (in.is("{")) result |= balanced("{", "}");
        else {
            if (unscoped && (in.is("no_unique_address") || in.is("__no_unique_address__"))) {
                if (in.is("(",1)) throw std::runtime_error("no_unique_address takes no arguments");
                if (native) native->no_unique_address = true;
            }
            if (native && (in.is("pure") || in.is("__pure__"))) native->effects = std::max(native->effects,FunctionEffects::ReadOnly);
            if (native && (in.is("const") || in.is("__const__"))) native->effects = FunctionEffects::ReadNone;
            if (native && (in.is("weak") || in.is("__weak__"))) native->weak = true;
            if (using_if_exists_attribute(ids.spelling(in.peek().text))) result |= UsingIfExists;
            if (in.is("packed") || in.is("__packed__")) result |= 32;
            if (in.is("noinline") || in.is("__noinline__")) result |= 64;
            if (in.is("always_inline") || in.is("__always_inline__")) result |= 128;
            if (in.is("cppgm_stable_prefix") || in.is("__cppgm_stable_prefix__")) {
                if (in.is("(",1)) throw std::runtime_error("stable-prefix attribute takes no arguments");
                result |= 16;
            }
            in.take();
            if (in.is("(")) { auto end = in.matching(0); for (std::size_t i=0;i<=end;++i) in.take(); }
        }
    }
    in.take(); return result;
}

unsigned Parser::attributes(std::uint32_t* alignment, NativeAttributes* native)
{
    unsigned result = 0;
    for (;;) {
        if (in.is("[") && in.is("[", 1)) result |= balanced("[", "]",native,alignment);
        else if (in.eat("alignas")) {
            in.require("(");
            bool is_type = type_operand();
            NodeId operand = is_type ? type_id() : expression(2);
            in.require(")");
            if (alignment) {
                ast.alignments.push_back(AlignmentAttribute{operand, *alignment, is_type});
                *alignment = ast.alignments.size()-1;
            }
        } else if (in.is("__attribute__") || in.is("__attribute")) {
            in.take();
            result |= balanced("(", ")",native,alignment);
        } else break;
    }
    return result;
}

} }
