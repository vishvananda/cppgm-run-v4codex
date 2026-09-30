#include "syntax/parser.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace syntax {
void Parser::native_attributes(NodeId owner, NativeAttributes value)
{
    if (!value.section && !value.weak && !value.tags && value.effects == FunctionEffects::Unknown) return;
    owner = ast.nodes.occurrences[owner].source;
    auto prior = ast.native_attribute_owners.get(owner);
    if (prior) {
        auto& old = ast.native_attributes[prior];
        if (old.section && value.section && old.section != value.section) throw std::runtime_error("conflicting section attributes");
        if (value.section) old.section = value.section;
        old.weak |= value.weak;
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
unsigned Parser::balanced(const char* open, const char* close, NativeAttributes* native)
{
    unsigned result = 0;
    in.require(open);
    while (!in.is(close)) {
        if (in.peek().kind == PostTokenKind::eof) throw std::runtime_error("unterminated attribute");
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
        else if (in.is("(")) result |= balanced("(", ")",native);
        else if (in.is("[")) result |= balanced("[", "]");
        else if (in.is("{")) result |= balanced("{", "}");
        else {
            if (native && (in.is("pure") || in.is("__pure__"))) native->effects = std::max(native->effects,FunctionEffects::ReadOnly);
            if (native && (in.is("const") || in.is("__const__"))) native->effects = FunctionEffects::ReadNone;
            if (native && (in.is("weak") || in.is("__weak__"))) native->weak = true;
            if (in.is("packed") || in.is("__packed__")) result |= 32;
            if (in.is("noinline") || in.is("__noinline__")) result |= 64;
            if (in.is("always_inline") || in.is("__always_inline__")) result |= 128;
            if (in.is("cppgm_stable_prefix") || in.is("__cppgm_stable_prefix__")) {
                if (in.is("(",1)) throw std::runtime_error("stable-prefix attribute takes no arguments");
                result |= 16;
            }
            in.take();
        }
    }
    in.take(); return result;
}

unsigned Parser::attributes(std::uint32_t* alignment, NativeAttributes* native)
{
    unsigned result = 0;
    for (;;) {
        if (in.is("[") && in.is("[", 1)) result |= balanced("[", "]");
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
            result |= balanced("(", ")",native);
        } else break;
    }
    return result;
}

} }
