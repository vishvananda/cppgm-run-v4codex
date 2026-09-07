#include "abi/itanium/fact_writer.h"
#include "abi/itanium/operations.h"
#include <stdexcept>
#include <sstream>

namespace abi_mangle {
namespace {
bool linear_type(Kind kind) {
    switch (kind) {
    case Kind::Pointer: case Kind::Reference: case Kind::RvalueReference:
    case Kind::Cv: case Kind::Pack: case Kind::Array: case Kind::Vector:
    case Kind::Tagged: case Kind::Vendor: case Kind::Name: return true;
    default: return false;
    }
}
std::string join_form(std::initializer_list<std::string> pieces) {
    std::ostringstream out;
    for (const std::string& piece : pieces) out << piece;
    return out.str();
}
}

FactWriter::FactWriter(const Graph& graph) : g(graph), seen(graph.size(), 0) {}
void FactWriter::definition(Id id, unsigned bit, const std::string& command,
                            const std::string& name, const std::string& form) {
    if (!seen[id]) touched.push_back(id);
    seen[id] |= bit;
    definitions += command; definitions += ' '; definitions += name;
    definitions += ' '; definitions += form; definitions += '\n';
}
std::string FactWriter::ref(char family, Id id) {
    unsigned bit = family == 't' ? 1 : family == 'a' ? 2 : family == 'x' ? 4 : family == 'c' ? 8 : 16;
    if (!id || id >= seen.size()) throw std::runtime_error("invalid fact serialization ID");
    std::string name = std::string(1, family) + std::to_string(id);
    if (seen[id] & bit) return name;
    if (family == 't' && g[id].a && linear_type(g[id].kind)) {
        std::vector<Id> chain;
        Id base = id;
        while (!(seen[base] & 1) && g[base].a && linear_type(g[base].kind)) {
            chain.push_back(base); base = g[base].a;
        }
        ref('t', base);
        for (auto i = chain.rbegin(); i != chain.rend(); ++i)
            definition(*i, 1, "let-type", "t" + std::to_string(*i), type(*i));
        return name;
    }
    if (++depth > 1024) throw std::runtime_error("ABI fact serialization nesting limit exceeded");
    std::string form, command;
    if (family == 't') { form = type(id); command = "let-type"; }
    else if (family == 'a') { form = argument(id); command = "let-arg"; }
    else if (family == 'x') { form = expression(id); command = "let-expr"; }
    else if (family == 'c') { form = context(id); command = "let-context"; }
    else { form = entity(id); command = "let-entity"; }
    --depth; definition(id, bit, command, name, form);
    return name;
}
std::string FactWriter::list(const Node& n, char family) {
    std::string result;
    for (Id i = 0; i < n.count; ++i) result += ' ' + ref(family, g.child(n, i));
    return result;
}
std::string FactWriter::type(Id id) {
    const Node& n = g[id];
    switch (n.kind) {
    case Kind::Name:
        if (!n.a) return join_form({"named:", g.spelling(n.b)});
        return join_form({"member ", ref('t', n.a), " ", g.spelling(n.b)});
    case Kind::Standard:
        return join_form({std::string("std-template "), abi_standard_substitution_code(
            static_cast<AbiStandardSubstitutionKind>(n.a)), " true standard"});
    case Kind::Template: {
        const Node& prefix = g[n.a]; std::string result;
        if (prefix.kind == Kind::Name) {
            if (prefix.a) result = "member-template " + ref('t', prefix.a) + ' ' + g.spelling(prefix.b);
            else result = "template " + g.spelling(prefix.b);
        } else if (prefix.kind == Kind::Parameter && !prefix.b)
            result = "template-param-template " + std::to_string(prefix.value);
        else if (prefix.kind == Kind::Standard)
            result = std::string("std-template ") + abi_standard_substitution_code(
                static_cast<AbiStandardSubstitutionKind>(prefix.a)) + " false standard";
        else result = "template-name " + ref('t', n.a);
        return join_form({result, list(n, 'a')});
    }
    case Kind::Tagged: {
        std::string result = "tagged " + ref('t', n.a);
        for (Id i = 0; i < n.count; ++i) result += ' ' + g.spelling(g.child(n, i));
        return result;
    }
    case Kind::Builtin: return abi_builtin_type_word(static_cast<AbiBuiltinTypeKind>(n.a));
    case Kind::Parameter: return join_form({std::string(n.b ? "template-param-subst " : "template-param "), std::to_string(n.value)});
    case Kind::Pointer: return join_form({"ptr ", ref('t', n.a)});
    case Kind::Reference: return join_form({"ref ", ref('t', n.a)});
    case Kind::RvalueReference: return join_form({"rref ", ref('t', n.a)});
    case Kind::Cv: return join_form({std::string(n.b & 1 ? "const " : ""), (n.b & 2 ? "volatile " : ""), ref('t', n.a)});
    case Kind::Pack: return join_form({"pack ", ref('t', n.a)});
    case Kind::Vendor: return join_form({"vendor ", g.spelling(n.b), " ", ref('t', n.a)});
    case Kind::Array:
        if (n.b) return join_form({"array-expression ", ref('x', n.b), " ", ref('t', n.a)});
        return join_form({"array ", std::to_string(n.value), " ", ref('t', n.a)});
    case Kind::Vector: return join_form({"vector ", std::to_string(n.value), " ", ref('t', n.a)});
    case Kind::Transform: return join_form({"builtin-transform ", g.spelling(n.b), list(n, 't')});
    case Kind::FunctionType:
        if (n.b) return join_form({"function-type-qualified ", std::to_string(n.b),
            (n.c ? " true " : " false "), ref('t', n.a), list(n, 't')});
        return join_form({std::string(n.c ? "function-type-variadic " : "function-type "), ref('t', n.a), list(n, 't')});
    case Kind::MemberPointer: return join_form({"member-pointer ", ref('t', n.a), " ", ref('t', n.b)});
    case Kind::Decltype: return join_form({"decltype ", ref('x', n.a)});
    case Kind::Local: return join_form({"local-type ", ref('c', n.a), " ", g.spelling(n.b), " ", std::to_string(n.value)});
    case Kind::Lambda: return join_form({"lambda-closure ", ref('c', n.a), " ", std::to_string(n.value), list(n, 't')});
    default: throw std::runtime_error("invalid type fact serialization");
    }
}
std::string FactWriter::argument(Id id) {
    const Node& n = g[id];
    switch (n.kind) {
    case Kind::TypeArgument: return join_form({"type ", ref('t', n.a)});
    case Kind::Value: return join_form({"value ", ref('t', n.a), " ", std::to_string(static_cast<std::int64_t>(n.value))});
    case Kind::DependentValue:
        return join_form({"dependent-value ", ref('t', n.a), " ", ref('t', g[n.b].a), " ", std::to_string(static_cast<std::int64_t>(g[n.b].value))});
    case Kind::ExpressionArgument: return join_form({"expression ", ref('x', n.a)});
    case Kind::ArgumentPack: return join_form({"pack", list(n, 'a')});
    case Kind::TemplateEntity:
        if (g[n.a].kind == Kind::Parameter) return join_form({"template-param-template ", std::to_string(g[n.a].value)});
        return join_form({"template-entity-type ", ref('t', n.a)});
    case Kind::MemberTemplateEntity: return join_form({"member-template-entity ", ref('t', n.a), " ", g.spelling(n.b), " -"});
    case Kind::EntityArgument: return join_form({std::string(n.b ? "entity-address " : "entity-reference "), ref('e', n.a)});
    default: throw std::runtime_error("invalid argument fact serialization");
    }
}
std::string FactWriter::expression(Id id) {
    const Node& n = g[id];
    switch (n.kind) {
    case Kind::ExprParameter: return join_form({"template-param ", std::to_string(n.value)});
    case Kind::ExprFunctionParameter: return join_form({"function-param ", std::to_string(n.value)});
    case Kind::Value: return join_form({"value ", ref('t', n.a), " ", std::to_string(static_cast<std::int64_t>(n.value))});
    case Kind::Unary: return join_form({std::string("unary "), operation_code(n.b), " ", ref('x', n.a)});
    case Kind::Binary: return join_form({std::string("binary "), operation_code(n.c), " ", ref('x', n.a), " ", ref('x', n.b)});
    case Kind::Conditional: return join_form({"conditional ", ref('x', n.a), " ", ref('x', n.b), " ", ref('x', n.c)});
    case Kind::ExprPack: return join_form({"pack ", ref('x', n.a)});
    case Kind::Call: return join_form({"call ", ref('x', n.a), list(n, 'x')});
    case Kind::Conversion: return join_form({"conversion ", ref('t', n.a), list(n, 'x')});
    case Kind::Cast: return join_form({std::string("cast "), operation_code(n.c), " ", ref('t', n.a), " ", ref('x', n.b)});
    case Kind::TemplateId: return join_form({"template-id ", g.spelling(n.a), list(n, 'a')});
    case Kind::TypeTrait: return join_form({"type-trait ", g.spelling(n.a), list(n, 't')});
    case Kind::SizeofType: return join_form({"sizeof-type ", ref('t', n.a)});
    case Kind::Member: return join_form({"member ", ref('t', n.a), (n.c ? " yes " : " no "), g.spelling(n.b), list(n, 'a')});
    case Kind::ObjectMember: return join_form({std::string("object-member "), operation_code(n.c), " ", ref('x', n.a), " ", g.spelling(n.b), list(n, 'a')});
    case Kind::EntityExpression: return join_form({std::string(n.b ? "entity-address " : "entity-reference "), ref('e', n.a)});
    default: throw std::runtime_error("invalid expression fact serialization");
    }
}
std::string FactWriter::qualifier_words(unsigned bits) {
    std::string result;
    if (bits & 1) result += " qualifier const";
    if (bits & 2) result += " qualifier volatile";
    if (bits & 4) result += " qualifier lvalue-ref";
    if (bits & 8) result += " qualifier rvalue-ref";
    return result;
}
std::string FactWriter::function(const Function& original) {
    Function f = original;
    f.name = function_shape(g, original, f.arguments, f.tags, f.template_prefix);
    std::string result;
    if (f.local_owner) {
        result = "local-owner " + ref('t', f.local_owner);
        if (f.name) result += " source " + g.spelling(g[f.name].b);
    } else {
        const Node& name = g[f.name]; std::string source = g.spelling(name.b);
        if (source.empty()) source = "operator";
        if (name.a) result = "member " + ref('t', name.a) + ' ' + source;
        else result = "path " + source;
    }
    // The full normalized inline shape is also usable in let-context/entity
    // records. Shape markers disambiguate parameters from template arguments.
    if (f.category != FunctionCategory::Inferred)
        result += f.category == FunctionCategory::Member ? " member-shape yes" : " member-shape no";
    if (f.template_prefix) result += " template-prefix";
    if (f.context && !f.local_owner) result += " context " + ref('c', f.context);
    for (Id a : f.arguments) result += " argument " + ref('a', a);
    for (Id p : f.parameters) result += " param " + ref('t', p);
    if (f.result) result += " result " + ref('t', f.result);
    if (f.variadic) result += " ...";
    if (f.c_linkage) result += " c-linkage";
    if (f.terminal != ABI_TERMINAL_NONE) result += std::string(" terminal ") + abi_terminal_word(f.terminal);
    if (f.literal_suffix) result += " literal-suffix " + g.spelling(f.literal_suffix);
    if (f.conversion) result += " conversion " + ref('t', f.conversion);
    for (Id tag : f.tags) result += " tag " + g.spelling(tag);
    return result + qualifier_words(f.qualifiers);
}
std::string FactWriter::entity(Id id) {
    const Node& n = g[id];
    if (n.kind == Kind::SymbolEntity) return "symbol " + g.spelling(n.a);
    if (n.kind == Kind::FunctionEntity) return "function " + function(entity_function(g, id));
    if (n.kind == Kind::VariableEntity) return std::string(n.b ? "internal-variable-type " : "variable-type ") + ref('t', n.a);
    throw std::runtime_error("invalid entity fact serialization");
}
std::string FactWriter::context(Id id) {
    const Node& n = g[id];
    if (n.kind == Kind::RawContext) return "raw " + g.spelling(n.a);
    return "function " + function(entity_function(g, id));
}
std::string FactWriter::write(const Target& t) {
    std::string result;
    switch (t.kind) {
    case TargetKind::Type: result = "type " + ref('t', t.type); break;
    case TargetKind::Function: result = "function " + function(t.function); break;
    case TargetKind::Variable: result = std::string(t.internal ? "internal-variable-type " : "variable-type ") + ref('t', t.type); break;
    case TargetKind::Typeinfo: result = "typeinfo " + ref('t', t.type); break;
    case TargetKind::TypeinfoName: result = "typeinfo-name " + ref('t', t.type); break;
    case TargetKind::Vtable: result = "vtable " + ref('t', t.type); break;
    case TargetKind::Vtt: result = "vtt " + ref('t', t.type); break;
    case TargetKind::ConstructionVtable: result = "construction-vtable " + ref('t', t.type) + ' ' + std::to_string(t.this_adjust) + ' ' + ref('t', t.base); break;
    case TargetKind::TlsWrapper: result = "tls-wrapper-type " + ref('t', t.type); break;
    case TargetKind::VirtualThunk:
        result = "virtual-base-thunk " + std::to_string(t.vcall_offset) +
            " this-adjust " + std::to_string(t.this_adjust) + " function " + function(t.function); break;
    case TargetKind::Thunk:
        result = "thunk " + std::to_string(t.this_adjust);
        if (t.has_result_adjust) {
            if (t.virtual_result) result += " virtual-result";
            result += ' ' + std::to_string(t.result_adjust);
            if (t.virtual_result) result += ' ' + std::to_string(t.result_vcall_offset);
        }
        result += " function " + function(t.function); break;
    }
    std::string output = std::move(definitions); output += result; output += '\n';
    definitions.clear();
    for (Id id : touched) seen[id] = 0;
    touched.clear();
    return output;
}
std::string serialize_fact_file(const AbiFactFile& file) {
    std::string result;
    FactWriter writer(file.graph);
    for (std::size_t i = 0; i < file.cases.size(); ++i) {
        result += "case case" + std::to_string(i) + '\n' + writer.write(file.cases[i]);
    }
    return result;
}
} // namespace abi_mangle
