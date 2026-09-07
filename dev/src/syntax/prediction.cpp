#include "syntax/parser.h"

namespace cppgm { namespace syntax {

std::size_t Parser::probe_angles(std::size_t ahead)
{
    std::size_t cached = in.angle_end(ahead);
    if (cached != ahead) {
        if (ast.telemetry) ++angle_hits;
        return cached;
    }
    angle_stack.clear();
    angle_stack.push_back(ahead);
    for (std::size_t i = ahead + 1;; ++i) {
        if (ast.telemetry) ++angle_work;
        if (in.peek(i).kind == PostTokenKind::eof || in.is(";", i) || in.is("}", i)) return ahead;
        if (in.is("(", i) || in.is("[", i) || in.is("{", i)) i = in.matching(i);
        else if (in.is("<", i)) angle_stack.push_back(i);
        else if (in.is(">", i) || in.is(">>", i)) {
            unsigned pieces = in.is(">>", i) ? 2 : 1;
            while (pieces-- && !angle_stack.empty()) {
                in.remember_angle(angle_stack.back(), i + 1);
                angle_stack.pop_back();
            }
            if (angle_stack.empty()) return i + 1;
        }
    }
}

Parser::NameProbe Parser::probe_name(std::size_t ahead)
{
    NameProbe result;
    ScopeId owner = scope;
    bool qualified = in.is("::", ahead);
    if (qualified) {
        owner = 0;
        ++ahead;
        result.qualified = true;
    }
    for (;;) {
        bool explicit_template = in.is("template", ahead);
        if (explicit_template) ++ahead;
        if (in.is("operator", ahead) || in.is("~", ahead)) {
            result.special = true;
            result.valid = true;
            result.end = ahead;
            return result;
        }
        if (!identifier(ahead)) break;
        Token token = in.peek(ahead++);
        Binding binding = qualified ? names.qualified(owner, token.text) : names.lookup(scope, token.text);
        result.previous = result.terminal;
        result.terminal = token.text;
        result.valid = true;
        result.binding = binding;
        bool potential = explicit_template || template_category(binding.category);
        if (binding.category == Category::Unknown) {
            potential |= lexical_hint(token.text) & 2;
            if (!potential && in.is("<", ahead) && identifier(ahead + 1)) potential = type_start(ahead + 1);
            potential |= builtin(ahead + 1) || in.is("typename", ahead + 1) ||
                         in.is("const", ahead + 1) || in.is("volatile", ahead + 1);
        }
        if (in.is("<", ahead) && potential) {
            std::size_t end = probe_angles(ahead);
            if (end != ahead) {
                result.templated = true;
                ahead = end;
            }
        }
        result.end = ahead;
        if (!in.is("::", ahead) || in.is("*", ahead + 1)) return result;
        Binding qualifier = names.qualifier(owner, token.text, qualified);
        owner = qualifier.target ? qualifier.target : unknown_scope;
        qualified = true;
        result.qualified = true;
        ++ahead;
    }
    return result;
}

std::size_t Parser::probe_type(std::size_t ahead)
{
    bool base = false;
    for (;;) {
        if (builtin(ahead)) {
            base = true;
            ++ahead;
        } else if (in.is("const", ahead) || in.is("volatile", ahead)) ++ahead;
        else if (!base && in.is("decltype", ahead)) {
            ahead = in.matching(ahead + 1) + 1;
            base = true;
        } else if (!base) {
            bool dependent = in.is("typename", ahead);
            if (dependent) ++ahead;
            NameProbe probe = probe_name(ahead);
            if (!probe.valid || (!dependent && !type_start(ahead))) return ahead;
            ahead = probe.end;
            base = true;
        } else return ahead;
    }
}

bool Parser::type_operand()
{
    if (!type_start()) return false;
    std::size_t end = probe_type(0);
    // Function-style construction is an expression in unary/trait contexts.
    if (in.is("(", end) && !in.is("*", end + 1) && !in.is("&", end + 1)) return false;
    return true;
}

bool Parser::special_ahead()
{
    std::size_t i = 0;
    while (in.is("inline", i) || in.is("virtual", i) || in.is("explicit", i) ||
           in.is("constexpr", i) || in.is("friend", i) || in.is("static", i)) ++i;
    while (in.is("__attribute__", i) || in.is("__attribute", i)) i = in.matching(i + 1) + 1;
    NameProbe probe = probe_name(i);
    if (!probe.valid) return false;
    if (probe.special) return true;
    if (!in.is("(", probe.end)) return false;
    if (current_class && probe.terminal == current_class) return true;
    return probe.qualified && probe.previous == probe.terminal;
}

void Parser::predeclare_class()
{
    // Complete-class contexts can use nested types declared after a body.
    // This category-only lookahead does not construct/reparse any grammar node.
    std::size_t i = 0;
    bool templated = false;
    while (!in.is("}", i) && in.peek(i).kind != PostTokenKind::eof) {
        if (in.is("template", i) && in.is("<", i + 1)) {
            std::size_t end = probe_angles(i + 1);
            if (end > i + 1) {
                i = end;
                templated = true;
            }
        }
        if ((in.is("class", i) || in.is("struct", i) || in.is("union", i) || in.is("enum", i)) && identifier(i + 1))
            names.bind(scope, in.peek(i + 1).text, templated ? Category::TemplateType : Category::Type);
        if (in.is("using", i) && identifier(i + 1) && in.is("=", i + 2))
            names.bind(scope, in.peek(i + 1).text, Category::Type);
        if (in.is("typedef", i)) {
            // Find declaration names without constructing grammar. Delimiter
            // indexing skips initializers/array bounds/parameter lists; the
            // first identifier after the type prefix starts each declarator.
            std::size_t p = probe_type(i + 1);
            bool need_name = true;
            for (; !in.is(";", p) && in.peek(p).kind != PostTokenKind::eof; ++p) {
                if (in.is(",", p)) { need_name = true; continue; }
                if (need_name && identifier(p)) {
                    names.bind(scope, in.peek(p).text, Category::Type);
                    need_name = false;
                }
                if (in.is("[", p) || in.is("{", p) || (in.is("(", p) && !need_name)) p = in.matching(p);
            }
        }
        if (templated && identifier(i) && in.is("(", i + 1) && in.peek(i).text != current_class)
            names.bind(scope, in.peek(i).text, Category::TemplateValue);
        if (in.is(";", i) || in.is("{", i)) templated = false;
        if (in.is("(", i) || in.is("[", i) || in.is("{", i)) i = in.matching(i);
        ++i;
    }
}

} }
